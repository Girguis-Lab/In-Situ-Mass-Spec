#pragma once
#include "includes.h"

// forward function delcaration - implemented later in the file.
void cmd_help(LazySerial::Context &context);

// Command to set the log level of the system. Usage: LOGLEVEL <0-6> where 0 is silent and 6 is verbose.
void cmd_set_debug_loglevel(LazySerial::Context &context)
{
    LAZY_COMMAND("DEBUG_LOGGING", "<OFF|LOW|HIGH>", "Enable additional logging to serial for troubleshooting purposes.");
    char *levelStr;
    bool ok = context.parse_word(&levelStr);
    if (ok)
    {
        int level = saved_settings.log_level;
        if (strcasecmp(levelStr, "OFF") == 0)
        {
            level = LOG_LEVEL_INFO;
        }
        else if (strcasecmp(levelStr, "LOW") == 0)
        {
            level = LOG_LEVEL_TRACE;
        }
        else if (strcasecmp(levelStr, "HIGH") == 0)
        {
            level = LOG_LEVEL_VERBOSE;
        }
        else
        {
            context.stream.println(F(ERROR " Invalid log level, use OFF, LOW, or HIGH"));
            return;
        }
        Log.setLevel(level);
        saved_settings.log_level = level;
        save_settings();
        Log.infoln(OK "DEBUG_LOGGING Set to %s Level (%d)", levelStr, level);
    }
}

void cmd_set_stats_broadcast_interval(LazySerial::Context &context)
{
    LAZY_COMMAND("STATS_INTERVAL", "<MS|OFF>", "Sets how often the system broadcasts stats to serial in milliseconds. Send OFF to disable stats logging.");
    char *intervalStr;
    bool ok = context.parse_word(&intervalStr);
    if (ok)
    {
        if (strcasecmp(intervalStr, "OFF") == 0)
        {
            Log.info("STATS_LOGGING OFF - periodic stats logging disabled");
            saved_settings.stats_loging_enabled = false;
            save_settings();
        }
        else
        {
            char *end;
            unsigned long interval = strtoul(intervalStr, &end, 10);
            LAZY_RETURN_USAGE_UNLESS(end > intervalStr); // Check that we parsed something
            saved_settings.stats_log_interval = interval;
            saved_settings.stats_loging_enabled = true;
            save_settings();
            Log.info("STATS_LOGGING ON - Logging interval set to %lu ms", interval);
        }
    }
}

// Helper function to handle ON/OFF commands
// If the *onoff string passed is "ON" or "OFF" (case insensitive), the passed pin number is set HIGH or LOW respectively.
void handle_onoff_command(uint8_t pin, char *onoff, Stream &stream)
{

    if (strcasecmp(onoff, "ON") == 0 || strcasecmp(onoff, "1") == 0)
    {
        digitalWrite(pin, HIGH);
        stream.println(F("ON"));
    }
    else if (strcasecmp(onoff, "OFF") == 0 || strcasecmp(onoff, "0") == 0)
    {
        digitalWrite(pin, LOW);
        stream.println(F("OFF"));
    }
    else
    {
        stream.println(F(ERROR " Invalid argument, use ON or OFF"));
    }
}

// A command to print the firmware version, compile date, and time for reference.
void cmd_version(LazySerial::Context &context)
{
    LAZY_COMMAND("VERSION", "", "Prints firmware version");
    context.stream.println(F(OK "VERSION " FIRMWARE_VERSION " COMPILED AT " __TIMESTAMP__));
}

// A command to show the configured pinout for reference.
void cmd_pinout(LazySerial::Context &context)
{
    LAZY_COMMAND("PINOUT", "", "Prints configured arduino pinout and I2C/RS485 addresses");
    context.stream.println(F(OK "PINOUT " LAZY_KEYVAL(PIN_LED1) LAZY_KEYVAL(PIN_LED2) LAZY_KEYVAL(PIN_PWR_ROUGHING) LAZY_KEYVAL(PIN_PWR_PH) LAZY_KEYVAL(PIN_PWR_FLUIDPUMP) LAZY_KEYVAL(PIN_ANALOG_FLUIDPUMP_SPEED)));
    context.stream.print(F("ADDRESSES: Turbo Pump RS485 Address="));
    context.stream.println(LAZY_KEYVAL(turboTC80.getAddress()));
}

// Command to set any pin high or low for testing purposes.
void cmd_gpio(LazySerial::Context &context)
{
    LAZY_COMMAND("GPIO", "<pin number> <ON|OFF>", "Set any Arduino pin high or low - TESTING ONLY, DO NOT USE WITHOUT KNOWING WHAT YOU'RE DOING.");
    uint8_t pin = 0;
    char *onoff;
    bool ok = context.parse_int(&pin);
    LAZY_RETURN_USAGE_UNLESS(ok);
    ok = context.parse_word(&onoff);
    LAZY_RETURN_USAGE_UNLESS(ok);

    pinMode(pin, OUTPUT);
    context.stream.print("OK GPIO ");
    context.stream.print(pin);
    handle_onoff_command(pin, onoff, context.stream);
}

// --------------------------------------------------
// ------------- POWER ON/OFF commands --------------------
// --------------------------------------------------

// A command to turn the roughing pump power on or off.
void cmd_roughing_on_off(LazySerial::Context &context)
{
    LAZY_COMMAND("ROUGHING", "<ON|OFF>", "Turns the roughing pump power on or off");
    char *onoff;
    bool ok = context.parse_word(&onoff);
    LAZY_RETURN_USAGE_UNLESS(ok);
    context.stream.print(F(OK "ROUGHING "));
    handle_onoff_command(PIN_PWR_ROUGHING, onoff, context.stream);
}

// A command to turn the pH probe power on or off.
void cmd_ph_pwr(LazySerial::Context &context)
{
    LAZY_COMMAND("PH", "<ON|OFF>", "Turns the pH probe power on or off");
    char *onoff;
    bool ok = context.parse_word(&onoff);
    LAZY_RETURN_USAGE_UNLESS(ok);
    context.stream.print(F(OK "PH "));
    handle_onoff_command(PIN_PWR_PH, onoff, context.stream);
}

// A command to turn the fluid pump power on or off.
void cmd_fluidpump_on_off(LazySerial::Context &context)
{
    LAZY_COMMAND("FLUIDPUMP", "<ON|OFF>", "Turns the fluid pump power on or off");
    char *onoff;
    bool ok = context.parse_word(&onoff);
    LAZY_RETURN_USAGE_UNLESS(ok);
    context.stream.print(F(OK "FLUIDPUMP "));
    if (strcasecmp(onoff, "ON") == 0)
    {
        if (saved_settings.fluidpump_rate == 0)
        {
            // TODO check this behavior
            // If the saved pump rate is 0, set it to a default value of 50% to ensure the pump actually turns on when the user sends the ON command, and save that to settings so it's used for next time as well.
            saved_settings.fluidpump_rate = 50;
            save_settings();
        }
        int resultRate = fluidPump.setSpeed(saved_settings.fluidpump_rate);
        if (resultRate == FLUID_PUMP_UNINITIALIZED)
        {
            context.stream.println(F(ERROR " Fluid pump not initialized"));
        }
    }
    else if (strcasecmp(onoff, "OFF") == 0)
    {
        int resultRate = fluidPump.setSpeed(0);
        if (resultRate == FLUID_PUMP_UNINITIALIZED)
        {
            context.stream.println(F(ERROR " Fluid pump not initialized"));
        }
    }
    else
    {
        LAZY_RETURN_USAGE_IF(true); // Invalid argument, use ON or OFF
    }
}

// --------------------------------------------------
// ------------- FLUID PUMP CONTROL commands --------------------
// --------------------------------------------------

void cmd_fluidpump_rate(LazySerial::Context &context)
{
    LAZY_COMMAND("FLUIDPUMP_RATE", "<-100-100>", "Sets the fluid pumping rate as a percentage of full speed. Negative values run pump in reverse. 0 will turn off pump power");
    int rate = 0;
    bool ok = context.parse_int_minmax(&rate, -100, 100);
    LAZY_RETURN_USAGE_UNLESS(ok);
    saved_settings.fluidpump_rate = rate;
    save_settings();
    int resultRate = fluidPump.setSpeed(rate);
    if (resultRate == FLUID_PUMP_UNINITIALIZED)
    {
        context.stream.println(F(ERROR " Fluid pump not initialized"));
    }
    else
    {
        context.stream.print(F(OK "FLUIDPUMP_RATE "));
        context.stream.print(resultRate);
        context.stream.println(F(" %"));
    }
}

// --------------------------------------------------
// ---------- TURBO PUMP CONTROL commands -----------------
// --------------------------------------------------

void cmd_turbo_on_off(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO", "<ON|OFF>", "Power ON/OFF turbo pump at configured speed.");
    char *onoff;
    bool ok = context.parse_word(&onoff);
    LAZY_RETURN_USAGE_UNLESS(ok);
    if (strcasecmp(onoff, "ON") == 0)
    {
        turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::MotorPump, PfeifferVacProtocol::BooleanOld(true));
        turboTC80.receiveTelegram(true);
    }
    else if (strcasecmp(onoff, "OFF") == 0)
    {
        turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::MotorPump, PfeifferVacProtocol::BooleanOld(false));
        turboTC80.receiveTelegram(true);
    }
    else
    {
        LAZY_RETURN_USAGE_IF(true); // Invalid argument, show usage message
    }
}

void cmd_turbo_speed(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO_SPEED", "<0.0-100.0>", "Sets the turbo pump to run at a target speed as percent of max speed (100% is 90,000 RPM for the Pfeiffer TC80) - Send 0 to reset to pfeiffer default speed control mode");
    // todo what is actual max speed ?
    uint16_t param = 0;
    bool ok = context.parse_float_minmax(&param, (uint16_t)0.0, (uint16_t)100.0);
    LAZY_RETURN_USAGE_UNLESS(ok);
    if (param == 0)
    {
        // Send 0 to reset to pfeiffer default speed control mode
        context.stream.println(F(OK "GOT '0' SO RESETING TO DEFAULT PFEIFFER SPEED CONTROL"));
        turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::SpdSetMode, PfeifferVacProtocol::UShortInt((uint16_t)0)); // turn off custom speed setting mode to go back to pfeiffer default speed control mode
        turboTC80.receiveTelegram(true);
        // todo is this needed?
        // turboTC80.sendCommand(PfeifferVacProtocol::ReferenceValueInput::SpdSVal, PfeifferVacProtocol::UReal(65.0)); // pfeiffer default is 65% of max power which corresponds to XXX todo???
        // turboTC80.receiveTelegram(true);
    }
    else
    {
        uint16_t speedValue = param;
        turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::SpdSetMode, PfeifferVacProtocol::UShortInt((uint16_t)1)); // turn on custom speed setting mode & diable pfeiffer default speed control mode
        turboTC80.receiveTelegram(true);
        turboTC80.sendCommand(PfeifferVacProtocol::ReferenceValueInput::SpdSVal, PfeifferVacProtocol::UReal(speedValue));
        turboTC80.receiveTelegram(true);
    }
}

void cmd_turbo_limit_pwr(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO_LIMIT_PWR", "<0-100>", "Sets the turbo pump power limit as a percentage of full power (Pfeiffer max draw is 5A)");
    context.stream.println(F(OK "TURBO_LIMIT_PWR"));
    turboTC80.sendCommand(PfeifferVacProtocol::ReferenceValueInput::PwrSVal, PfeifferVacProtocol::UShortInt(100));
    bool isValid; // TODO check this funciton
    turboTC80.receiveUInteger(true, isValid);
    delay(10000);
}

void cmd_turbo_cmd(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO_CMD", "<PARAM> <DATA>", "Send a command to the turbo pump with the 3-digit parameter number and optional data string");

    uint16_t param = 0;
    bool ok = context.parse_int_minmax(&param, (uint16_t)0, (uint16_t)999);
    LAZY_RETURN_USAGE_UNLESS(ok);

    char *dataStr = nullptr;
    ok = context.parse_word(&dataStr);
    LAZY_RETURN_USAGE_UNLESS(ok);

    turboTC80.sendCommand(param, dataStr, true);
    turboTC80.receiveTelegram(true);
}

void cmd_turbo_query(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO_QUERY", "<PARAM>", "Query a parameter from the turbo pump with the 3-digit parameter number");
    COMMS.println(F(OK "TURBO_QUERY"));
    uint16_t param = 0;
    bool ok = context.parse_int_minmax(&param, (uint16_t)0, (uint16_t)999);
    LAZY_RETURN_USAGE_UNLESS(ok);

    // queries always have 'action' set to '0' and a query data of "?"
    turboTC80.sendQuery(param, true);
    turboTC80.receiveTelegram(true, 1000);
}

void cmd_turbo_raw(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO_RAW", "<COMMAND>", "Send raw ASCII to the turbo pump");
    char *command;
    bool ok = context.parse_word(&command);
    LAZY_RETURN_USAGE_UNLESS(ok);
    turboTC80._sendTelegramRaw(command);
    turboTC80.receiveTelegram(true, 1000);
}

void cmd_turbo_reset(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO_RESET", "", "Resets turbo pump parameters for default operation in normal conditions (light gases, default speed control mode, power limit 100%)");
    context.stream.println(F("TURBO_RESET"));
    delay(500);
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::GasMode, PfeifferVacProtocol::UShortInt((uint8_t)PfeifferVacProtocol::FuncGasMode::LightGases), true);
    delay(500);
    turboTC80.receiveTelegram(true);
    delay(500);
    // turboTC80.sendCommand(PfeifferVacProtocol::ReferenceValueInput::PwrSVal, PfeifferVacProtocol::UShortInt(100), true);
    // // delay(500);
    // // turboTC80.receiveTelegram(true);
    // delay(500);
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::SpdSetMode, PfeifferVacProtocol::UShortInt(1)); // turn off custom speed setting mode to go back to pfeiffer default speed control mode
    delay(500);
    turboTC80.receiveTelegram(true);
    turboTC80.sendCommand(PfeifferVacProtocol::ReferenceValueInput::SpdSVal, PfeifferVacProtocol::UReal(100.0)); // pfeiffer default is 65% of max power which corresponds to XXX todo
    turboTC80.receiveTelegram(true);
    delay(500);
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::PumpgStatn, PfeifferVacProtocol::BooleanOld((bool)PfeifferVacProtocol::FuncPumpingStation::OnAndAckn), true); // Turn on pumping station (acknowledging errors)
    delay(500);
    turboTC80.receiveTelegram(true);
    COMMS.println(F(OK "TURBO_RESET COMPLETE"));
}

void cmd_turbo_clear_errors(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO_CLEAR_ERRORS", "", "Clears turbo pump errors and warning messages");
    context.stream.println(F("TURBO_CLEAR_ERRORS"));
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::ErrorAckn, PfeifferVacProtocol::BooleanOld(true), true);
    turboTC80.receiveTelegram(true);
    COMMS.println(F(OK "TURBO_CLEAR_ERRORS COMPLETE"));
}

void cmd_full_startup(LazySerial::Context &context)
{
    LAZY_COMMAND("STARTUP", "", "Run full startup sequence.");

    COMMS.println("Autostarting system now.");
    // Power on sequence
    // 1. Power on roughing pump
    digitalWrite(PIN_PWR_ROUGHING, HIGH);
    COMMS.println("Roughing Pump ON");
    nonBlockDelay(100); // Wait 100ms to avoid current spikes
    // 2. Power on fluid pump
    digitalWrite(PIN_PWR_FLUIDPUMP, HIGH);
    COMMS.println("Fluid Pump ON");
    nonBlockDelay(100); // Wait 100ms to avoid current spikes
    // 3. Power on PH Probe
    digitalWrite(PIN_PWR_PH, HIGH);
    COMMS.println("PH Probe ON");
    // !!!! IMPORTANT WAIT 60 Seconds so that roughing pump has sufficiently pumped down vacuum !!!!!
    COMMS.println("Waiting 60 seconds for rough out...");
    nonBlockDelay(60000); // Wait 60 seconds !!!!!
    COMMS.println("Turning ON Turbo Pump...");
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::GasMode, (uint8_t)PfeifferVacProtocol::FuncGasMode::LightGases, true);
    turboTC80.receiveTelegram(true);
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::MotorPump, (uint8_t)PfeifferVacProtocol::FuncOnOff::On);
    turboTC80.receiveTelegram(true);
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::PumpgStatn, (uint8_t)PfeifferVacProtocol::FuncPumpingStation::OnAndAckn, true); // Turn on pumping station (acknowledging errors)
    turboTC80.receiveTelegram(true);
    context.stream.println("Startup Complete");
}

void cmd_autostart_on_off(LazySerial::Context &context)
{
    LAZY_COMMAND("AUTOSTART", "<ON|OFF|DELAY>", "Set whether the system should startup automatically. If a integer is passed, the delay in milliseconds after boot.");
    char *param;
    bool hasParam = context.parse_word(&param);
    if (hasParam)
    {
        if (strcasecmp(param, "ON") == 0)
        {
            saved_settings.autostart_on = true;
            context.stream.println(F(OK "AUTOSTART ON"));
            save_settings();
        }
        else if (strcasecmp(param, "OFF") == 0)
        {
            saved_settings.autostart_on = false;
            context.stream.println(F(OK "AUTOSTART OFF"));
            save_settings();
        }
        else
        {
            char *end;
            unsigned long delay = strtoul(param, &end, 10);
            LAZY_RETURN_USAGE_UNLESS(end > param) // check that we parsed some number and not just garbage
            saved_settings.autostart_delay = delay;
            saved_settings.autostart_on = true; // if we're setting a delay, we should also turn autostart on
            save_settings();
        }
    }
    Log.infoln(F("| AUTOSTART %s, AUTOSTART_DELAY: %u ms"), saved_settings.autostart_on ? "ON" : "OFF", saved_settings.autostart_delay);
}

LazySerial::CallbackFunction basic_comms_commands[] = {
    cmd_help,
    cmd_full_startup,
    cmd_roughing_on_off,
    cmd_ph_pwr,
    cmd_fluidpump_on_off,
    cmd_fluidpump_rate,
    cmd_turbo_on_off,
    cmd_turbo_speed,
    cmd_turbo_limit_pwr,
    cmd_turbo_clear_errors,
    cmd_set_stats_broadcast_interval,
    cmd_autostart_on_off,
};

LazySerial::CallbackFunction debug_comms_commands[] = {
    cmd_help,
    cmd_version,
    cmd_pinout,
    cmd_set_debug_loglevel,
    cmd_turbo_reset,
    cmd_turbo_query,
    cmd_turbo_cmd,
    cmd_turbo_raw,
    cmd_gpio,
};

LazySerial::CallbackFunction all_comms_commands[] = {
    cmd_help,
    cmd_version,
    cmd_set_debug_loglevel,
    cmd_set_stats_broadcast_interval,
    cmd_autostart_on_off,
    cmd_full_startup,
    cmd_roughing_on_off,
    cmd_ph_pwr,
    cmd_fluidpump_on_off,
    cmd_fluidpump_rate,
    cmd_turbo_on_off,
    cmd_turbo_speed,
    cmd_turbo_limit_pwr,
    cmd_turbo_reset,
    cmd_turbo_clear_errors,
    cmd_turbo_cmd,
    cmd_turbo_query,
    cmd_turbo_raw,
    cmd_pinout,
    cmd_gpio,
};

/** Prints out the passed list of lazyserial commands and their usage in askii table format */
void print_cmd_help_table(LazySerial::CallbackFunction *commands_list, size_t commands_list_size)
{
    for (uint8_t i = 0; i < commands_list_size; ++i)
    {
        LazySerial::Context context(LazySerial::CallingMode::GET_METADATA, COMMS);
        commands_list[i](context);
        if (context.command_usage && context.command_description)
        {
            size_t name_length = strlen_P(reinterpret_cast<PGM_P>(context.command_name));
            COMMS.print(F("| "));
            COMMS.print(context.command_name);
            COMMS.print(' ');
            print_padded(COMMS, context.command_usage, lazy.d_usage_column_width - name_length - 1, ' ');
            COMMS.print(F(" | "));
            COMMS.println(context.command_description);
        }
        COMMS.print('+');
        print_padded(COMMS, F(""), lazy.d_usage_column_width + 2, '-');
        COMMS.println(F("+------------------"));
    }
};

/** Prints out the basic available commands and their usage as a table - used as the default help message callback by lazyserial */
void basic_help_table(LazySerial::Context &context)
{
    print_padded(COMMS, F("+====== AVAILABLE COMMANDS "), lazy.d_usage_column_width + 3, '=');
    COMMS.println(F("==================="));
    print_cmd_help_table(basic_comms_commands, sizeof(basic_comms_commands) / sizeof(basic_comms_commands[0]));
}

/** Prints out the debug/troubleshooting commands and their usage as a table */
void debug_help_table(LazySerial::Context &context)
{
    print_padded(COMMS, F("+====== DEBUGGING COMMANDS "), lazy.d_usage_column_width + 3, '=');
    COMMS.println(F("==================="));
    print_cmd_help_table(debug_comms_commands, sizeof(debug_comms_commands) / sizeof(debug_comms_commands[0]));
}

// A command to show a help message listing all available commands.
void cmd_help(LazySerial::Context &context)
{
    LAZY_COMMAND("HELP", "<DEBUG>", "Shows this help message, add DEBUG to show additional troubleshooting commands");
    char *param;
    bool hasParam = context.parse_word(&param);
    if (hasParam && strcasecmp(param, "DEBUG") == 0)
    {
        debug_help_table(context);
    }
    else
    {
        basic_help_table(context);
    }
}

// Set the default lazyserial help callback to the basic help table print function, which just shows the most commonly used commands.
const auto &lazy_help_callback = basic_help_table;
