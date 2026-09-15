/*
 * This file is part of the LazySerial library.
 * Copyright (C) 2025 Lazy Cat Software <arduino@neko.stream>
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <Arduino.h>

#include "LazySerial/helpers.h"
#include "LazySerial/Context.h"

#define LAZYSERIAL_VERSION 2.1

#define LAZY_COMMAND(NAME, USAGESTR, DESCRIPTIONSTR)                                            \
  if (context.mode == LazySerial::CallingMode::GET_METADATA)                                    \
  {                                                                                             \
    context.command_name = F(NAME);                                                             \
    context.command_usage = F(USAGESTR);                                                        \
    context.command_description = F(DESCRIPTIONSTR);                                            \
    return;                                                                                     \
  }                                                                                             \
  else if (context.mode == LazySerial::CallingMode::IDENTIFY)                                   \
  {                                                                                             \
    context.stream.print(NAME);                                                                 \
    return;                                                                                     \
  }                                                                                             \
  else if (context.mode == LazySerial::CallingMode::USAGE)                                      \
  {                                                                                             \
    context.stream.print(reinterpret_cast<const __FlashStringHelper *>(LazySerial::ERR_USAGE)); \
    context.stream.print(NAME);                                                                 \
    context.stream.print(' ');                                                                  \
    context.stream.println(USAGESTR);                                                           \
    return;                                                                                     \
  }                                                                                             \
  else if (context.mode == LazySerial::CallingMode::INVOKE)                                     \
  {                                                                                             \
    if (!context.matches_command(NAME))                                                         \
    {                                                                                           \
      return; /* not us. */                                                                     \
    }                                                                                           \
    context.mode = LazySerial::CallingMode::MATCHED;                                            \
  }

#define LAZY_RETURN_USAGE_IF(X)                    \
  if (X)                                           \
  {                                                \
    context.mode = LazySerial::CallingMode::USAGE; \
    return;                                        \
  }
#define LAZY_RETURN_USAGE_UNLESS(X)                \
  if (!(X))                                        \
  {                                                \
    context.mode = LazySerial::CallingMode::USAGE; \
    return;                                        \
  }

namespace LazySerial
{
  /**
   * The function pointer signature used for callbacks.
   * It is necessary to pass the 'args' string as a non-const char *, since you will most likely want to
   * use strtok() on it to further parse things.
   */
  typedef void (*CallbackFunction)(Context &);

  /**
   * Function pointer signature for a generic character-reading source, for use with scripts saved to EEPROM.
   */
  typedef char (*ReaderFunction)(size_t);

  // Save a tiny bit of precious memory by defining this globally.
  static const char ERR_USAGE[] PROGMEM = "ERR Usage: ";

  template <size_t BUF_SIZE>
  class LazySerial
  {
  public:
    /**
     * Constructor. Pass in the Stream to read and write from/to.
     * For example, the magic 'Serial' global variable should be usable as a Stream-class object.
     */
    explicit LazySerial(
        Stream &stream) : d_stream(stream),
                          d_help(NULL),
                          d_commands(nullptr),
                          d_commands_size(0),
                          d_usage_column_width(0)
    {
      clear_buffer();
    }

    /**
     * Call this during setup() to set your array of LazySerial::CallbackFunction s
     * Weird template magic allows us to deduce the size of array passed.
     * Ref: https://cplusplus.com/articles/D4SGz8AR/
     * "The way to prevent this conversion (known as 'decay') is to declare the function parameter as a reference to an array by changing fun(string s[N]) to fun(string (&s)[N])"
     */
    template <size_t S>
    void
    set_commands(
        CallbackFunction (&commands)[S])
    {
      d_commands = commands;
      d_commands_size = S;
      // Pre-scan commands to calculate the max usage string length for help printing.
      d_usage_column_width = 0;
      for (uint8_t i = 0; i < d_commands_size; ++i)
      {
        Context context(CallingMode::GET_METADATA, d_stream);
        d_commands[i](context);
        if (context.command_usage)
        {
          size_t name_length = strlen_P(reinterpret_cast<PGM_P>(context.command_name));
          size_t usage_length = strlen_P(reinterpret_cast<PGM_P>(context.command_usage));
          d_usage_column_width = max(d_usage_column_width, name_length + usage_length + 1); // +1 for the space between command name and usage string
        }
      }
    }

    /**
     * Call this from your own loop() for LazySerial to poll the Serial device for more data.
     * Shouldn't delay for too long unless one of your callbacks ends up triggering and taking time to process.
     */
    bool
    loop()
    {
      // Slowly assemble the command buffer byte by byte.
      bool ready = assemble_command();
      if (!ready)
        return false;
      // copy the full command to a temporary buffer to avoid recursive commands overwriting stuff.
      char tmp_buf[BUF_SIZE];
      memcpy(tmp_buf, d_buf, BUF_SIZE);
      // clear the global input buffer
      clear_buffer();
      // run the command!
      run_command(tmp_buf);
      return true;
    }

    /**
     * Instead of LazySerial polling the supplied Stream for commands, you can also supply a large string of
     * \n-terminated commands to run in a batch.
     */
    void
    run_script(
        const char *script)
    {
      const char *pos = script;
      const char *end = script;
      while (*pos)
      {
        // starting from pos, search for a \n or \0.
        end = pos;
        while (*end && *end != '\n')
        {
          end++;
        }
        // Copy the resulting line into the modifiable command buffer.
        size_t length = end - pos;
        if (length)
        {
          length = MIN(length, BUF_SIZE - 1);
          strncpy(d_buf, pos, length);
          d_buf[length] = '\0';

          // Parse out the command and its arguments and run it!
          run_command(d_buf);
          clear_buffer();
        }
        // Next line
        if (*end)
        {
          end++;
        }
        pos = end;
      }
    }

    /**
     * Or a generic function to be called with an incrementing index until a '\0' is returned.
     * This lets me read from EEPROM without depending on EEPROM.h here.
     */
    void
    run_script(
        ReaderFunction read_char_fn)
    {
      size_t pos = 0;
      size_t this_cmd_pos = 0;
      char ch = read_char_fn(pos);
      while (ch)
      {
        if (ch == '\n')
        {
          // Reached newline, run this command rather than append '\n'.
          d_buf[this_cmd_pos] = '\0';
          run_command(d_buf);
          clear_buffer();
          // Reset.
          this_cmd_pos = 0;
        }
        else
        {
          // Copy into command buffer as we go.
          d_buf[this_cmd_pos++] = ch;
        }
        // Read next ch
        ch = read_char_fn(++pos);
      }
      // Reached \0, is there any leftover?
      if (this_cmd_pos)
      {
        d_buf[this_cmd_pos] = '\0';
        run_command(d_buf);
        clear_buffer();
      }
    }

    /**
     * Dispatch the command named by 'cmd_name', to whatever callback has been registered by the user.
     * If none match, cmd_help() will be invoked instead.
     */
    void
    dispatch_command(
        const char *cmd_name,
        char *cmd_args)
    {
      // No-op command, helps in the case we are getting CRLF.
      LAZY_RETURN_IF(cmd_name[0] == '\0');

      // Scan through all registered callbacks.
      for (uint8_t i = 0; i < d_commands_size; ++i)
      {
        Context context{CallingMode::INVOKE, d_stream, cmd_name, cmd_args};
        d_commands[i](context);
        LAZY_RETURN_IF(context.mode == CallingMode::MATCHED);
        if (context.mode == CallingMode::USAGE)
        {
          // We matched the command but ran into problems parsing args.
          // Call it again asking it to print its usage message.
          d_commands[i](context);
          return;
        }
      }
      // Nothing matched. Print some help?
      if (d_help)
      {
        Context context{CallingMode::INVOKE, d_stream, "HELP", cmd_args};
        d_help(context);
      }
      else
      {
        cmd_help();
      }
    }

    /**
     * The default help function.
     * The magic HELP command is hard-coded to actually hit this method rather than anything in
     * the function table, because we want access to our list of commands.
     */
    void
    cmd_help()
    {
      d_stream.println(F("Generic Serial Command Help: - why did we get here? You should have set a help callback if you want this to do anything."));
    }

    /**
     * Set an alternative callback when no command matches.
     */
    void
    set_help_callback(
        CallbackFunction cmd_help)
    {
      d_help = cmd_help;
    }

  private:
    void
    clear_buffer()
    {
      d_pos = 0;
      d_buf[d_pos] = '\0';
    }

    /**
     * Read bytes from the serial until we get a \n.
     * Returns true if we get one and have a completed command (with \0) in the buffer,
     * false if we have yet to get a full command (or hit LAZYSERIAL_BUF_SIZE - no incomplete
     * command will be processed)
     */
    bool
    assemble_command()
    {
      while (d_stream.available())
      {
        // Read new character
        char ch = d_stream.read();
        d_stream.print(ch); // Echo back what we got.

        // If it's the \n terminator, don't advance pos but instead write \0 and return success.
        // Arduino seems to (correctly) interpret \n as 10, LF. Which is 'Newline' in the Serial Monitor.
        // Minicom is being weird. Let's just support both CR and LF (and in the event we get both, consume the extra newline)
        if (ch == '\n' || ch == '\r')
        {
          line_end = ch;
          if (ch == '\r' && d_stream.peek() == '\n')
          {
            d_stream.read(); // throw away the trailing newline from the buffer
          }
          d_buf[d_pos] = '\0';
          return true;
        }

        // For the mundane case, add the character to the buf and advance pos,
        // keeping a \0 terminator just in case.
        d_buf[d_pos] = ch;
        d_pos++;
        if (d_pos >= BUF_SIZE)
        {
          // But if we're going to overflow, forget the whole damn thing.
          clear_buffer();
          return false;
        }
        d_buf[d_pos] = '\0'; // Just me being paranoid.
      }
      return false;
    }

    /**
     * Once the buffer is full, identify what command it is, parse and run it.
     *
     */
    void
    run_command(char tmp_buf[BUF_SIZE])
    {
      // Identify the command word. strchr is in <string.h>
      char *end_of_cmd = strchr(tmp_buf, ' ');
      char *cmd_name = tmp_buf;
      char *cmd_args = tmp_buf;
      if (end_of_cmd)
      {
        // Set the delimiting space to a \0, advance args ptr to one past it.
        end_of_cmd[0] = '\0'; // cmd_name will now be valid
        end_of_cmd++;
        cmd_args = end_of_cmd;
      }
      else
      {
        // No args. Put the 'args' pointer at the trailing \0 of the command itself, making args the empty string.
        cmd_args = tmp_buf + strlen(tmp_buf);
      }

      // Dispatch command!
      dispatch_command(cmd_name, cmd_args);
    }

    /**
     * What stream we are reading from / writing to.
     */
    Stream &d_stream;

    /**
     * Permit cmd_help to be overridden with something custom (and outside of this class).
     */
    CallbackFunction d_help;

    /**
     * A statically declared list of callback functions.
     */
    CallbackFunction *d_commands;
    uint8_t d_commands_size;

  public:
    uint8_t d_usage_column_width; // for help printing, the width of the longest usage string among registered commands
    char line_end = '\0';

  private:
    /**
     * Command Buffer, and our current position within it.
     */
    char d_buf[BUF_SIZE];
    size_t d_pos;

  }; // class
} // namespace
