#pragma once
#include "includes.h"

// --------- Helper Functions -----------

// handle ON/OFF commands that directly control an arduino pin
// If the *onoff string passed is "ON" or "OFF" (case insensitive), the passed pin number is set HIGH or LOW respectively.
void handle_onoff_pin_command(powerPin *pwrPin, char *onoff, Stream &stream)
{

    if (strcasecmp(onoff, "ON") == 0 || strcasecmp(onoff, "1") == 0)
    {
        pwrPin->turnOn();
        stream.println(F("ON"));
    }
    else if (strcasecmp(onoff, "OFF") == 0 || strcasecmp(onoff, "0") == 0)
    {
        pwrPin->turnOff();
        stream.println(F("OFF"));
    }
    else
    {
        stream.println(F(ERROR " Invalid argument, use ON or OFF"));
    }
}

// --------- User Commands -----------

// forward function delcaration - implemented later in the file.
void cmd_help(LazySerial::Context &context);

// Command to set the log level of the system.
void cmd_set_debug_loglevel(LazySerial::Context &context)
{
    LAZY_COMMAND("DEBUG_LOGGING", "<OFF|LOW|HIGH>", "Enable additional logging to serial for troubleshooting purposes.");
    char *levelStr;
    bool ok = context.parse_word(&levelStr);
    LAZY_RETURN_USAGE_IF(!ok)
    if (ok)
    {
        DebugLogLevel level = saved_settings.log_level;
        if (strcasecmp(levelStr, "OFF") == 0)
        {
            level = DEBUG_LOG_LEVEL_OFF;
        }
        else if (strcasecmp(levelStr, "LOW") == 0)
        {
            level = DEBUG_LOG_LEVEL_LOW;
        }
        else if (strcasecmp(levelStr, "HIGH") == 0)
        {
            level = DEBUG_LOG_LEVEL_HIGH;
        }
        else
        {
            COMMS.println(F(ERROR " Invalid log level, use OFF, LOW, or HIGH"));
            return;
        }

        LOG_SET_LEVEL(level);
        saved_settings.log_level = level;
        save_settings();
        LOG_INFO(F(OK "DEBUG_LOGGING Set to "));
        LOG_INFO(logLevelToString(saved_settings.log_level).c_str(), "(", (int)LOG_GET_LEVEL(), ")\n");
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
            saved_settings.stats_loging_enabled = false;
        }
        else
        {
            char *end;
            unsigned long interval = strtoul(intervalStr, &end, 10);
            LAZY_RETURN_USAGE_UNLESS(end > intervalStr); // Check that we parsed something
            saved_settings.stats_log_interval = interval;
            saved_settings.stats_loging_enabled = true;
        }
        save_settings();
    }

    // Print the current state:
    if (!saved_settings.stats_loging_enabled)
    {
        LOG_INFO(OK "STATS_LOGGING OFF - periodic stats logging disabled\n");
    }
    else
    {
        LOG_INFO(F(OK "STATS_LOGGING ON - Logging interval set to "));
        LOG_INFO(saved_settings.stats_log_interval);
        LOG_INFO(F("ms\n"));
    }
}

// A command to print the firmware version, compile date, and time for reference.
void cmd_version(LazySerial::Context &context)
{
    LAZY_COMMAND("VERSION", "", "Prints firmware version");
    COMMS.println(F(OK "VERSION " FIRMWARE_VERSION " COMPILED AT " __TIMESTAMP__));
}

// A command to show the configured pinout for reference.
void cmd_pinout(LazySerial::Context &context)
{
    LAZY_COMMAND("PINOUT", "", "Prints configured arduino pinout and I2C/RS485 addresses");
    COMMS.println(F(OK "PINOUT " LAZY_KEYVAL(PIN_LED1) LAZY_KEYVAL(PIN_LED2) LAZY_KEYVAL(PIN_PWR_ROUGHING) LAZY_KEYVAL(PIN_PWR_PH) LAZY_KEYVAL(PIN_PWR_FLUIDPUMP) LAZY_KEYVAL(PIN_ANALOG_FLUIDPUMP_SPEED)));
    COMMS.print(F("ADDRESSES: Turbo Pump RS485 Address="));
    COMMS.println(LAZY_KEYVAL(turboTC80.getAddress()));
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

    powerPin pwrPin(pin);
    COMMS.print(OK "GPIO ");
    COMMS.print(pin);
    COMMS.print(" ");
    handle_onoff_pin_command(&pwrPin, onoff, context.stream);
}

// Command to set any pin high or low for testing purposes.
void cmd_beat(LazySerial::Context &context)
{
    LAZY_COMMAND("BEAT", "", "Send to stop autostart for this boot, used to temporarily enforce fully manual control");
    beatActive = true;
    COMMS.println(OK "BEAT");
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
    COMMS.print(F(OK "ROUGHING "));
    handle_onoff_pin_command(&ROUGHING_PWR, onoff, context.stream);
}

// A command to turn the accessory power on or off (usually a PH Probe).
void cmd_accessory_pwr(LazySerial::Context &context)
{
    LAZY_COMMAND(ACCESSORY_NAME, "<ON|OFF>", "Turns the " ACCESSORY_NAME " power on or off");
    char *onoff;
    bool ok = context.parse_word(&onoff);
    LAZY_RETURN_USAGE_UNLESS(ok);
    COMMS.print(F(OK "" ACCESSORY_NAME " "));
    handle_onoff_pin_command(&ACCESSORY_PWR, onoff, context.stream);
}

// A command to turn the fluid pump power on or off.
void cmd_fluidpump_on_off(LazySerial::Context &context)
{
    LAZY_COMMAND("FLUIDPUMP", "<ON|OFF>", "Turns the fluid pump power on or off");
    char *onoff;
    bool ok = context.parse_word(&onoff);
    LAZY_RETURN_USAGE_UNLESS(ok);
    COMMS.print(F(OK "FLUIDPUMP "));
    if (strcasecmp(onoff, "ON") == 0)
    {
        if (saved_settings.fluidpump_rate == 0)
        {
            // If the saved pump rate is 0, set it to a default value of 50% to ensure the pump actually turns on when the user sends the ON command, and save that to settings so it's used for next time as well.
            saved_settings.fluidpump_rate = 50;
            save_settings();
        }
        int resultRate = fluidPump.setSpeed(saved_settings.fluidpump_rate);
        if (resultRate == FLUID_PUMP_UNINITIALIZED)
        {
            COMMS.println(F(ERROR " Fluid pump not initialized"));
        }
    }
    else if (strcasecmp(onoff, "OFF") == 0)
    {
        int resultRate = fluidPump.setSpeed(0);
        if (resultRate == FLUID_PUMP_UNINITIALIZED)
        {
            COMMS.println(F(ERROR " Fluid pump not initialized"));
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
        COMMS.println(F(ERROR " Fluid pump not initialized"));
    }
    else
    {
        COMMS.print(F(OK "FLUIDPUMP_RATE "));
        COMMS.print(resultRate);
        COMMS.println(F(" %"));
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
    uint16_t param = 0;
    bool ok = context.parse_float_minmax(&param, (uint16_t)0.0, (uint16_t)100.0);
    LAZY_RETURN_USAGE_UNLESS(ok);
    if (param == 0)
    {
        // Send 0 to reset to pfeiffer default speed control mode
        COMMS.println(F(OK "GOT '0' SO RESETING TO DEFAULT PFEIFFER SPEED CONTROL"));
        turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::SpdSetMode, PfeifferVacProtocol::UShortInt((uint16_t)0)); // turn off custom speed setting mode to go back to pfeiffer default speed control mode
        turboTC80.receiveTelegram(true);
    }
    else
    {
        uint16_t speedValue = param;
        turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::SpdSetMode, PfeifferVacProtocol::UShortInt((uint16_t)1)); // turn on custom speed setting mode & diable pfeiffer default speed control mode
        turboTC80.receiveTelegram(true);
        nonBlockDelay(20); // TC80 Likes a delay between queries.
        turboTC80.sendCommand(PfeifferVacProtocol::ReferenceValueInput::SpdSVal, PfeifferVacProtocol::UReal(speedValue));
        turboTC80.receiveTelegram(true);
    }
}

void cmd_turbo_limit_pwr(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO_LIMIT_PWR", "<0-100>", "Sets the turbo pump power limit as a percentage of full power (Pfeiffer max draw is 5A)");
    COMMS.print(F(OK "TURBO_LIMIT_PWR "));
    int percent = 100;
    context.parse_int_minmax(&percent, 10, 100);
    COMMS.println(percent);
    bool isValid;
    turboTC80.sendCommand(PfeifferVacProtocol::ReferenceValueInput::PwrSVal, PfeifferVacProtocol::UShortInt((uint16_t)percent));
    turboTC80.receiveUShortInt(PfeifferVacProtocol::ReferenceValueInput::PwrSVal, isValid, true);
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

    uint16_t param = 0;
    bool ok = context.parse_int_minmax(&param, (uint16_t)0, (uint16_t)999);
    LAZY_RETURN_USAGE_UNLESS(ok);
    COMMS.print(F(OK "TURBO_QUERY"));
    COMMS.println(param);
    // queries always have 'action' set to '0' and a query data of "?"
    turboTC80.sendQuery(param, true);
    turboTC80.receiveTelegram(true);
}

void cmd_turbo_raw(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO_RAW", "<COMMAND>", "Send raw ASCII to the turbo pump");
    char *command;
    bool ok = context.parse_word(&command);
    LAZY_RETURN_USAGE_UNLESS(ok);
    COMMS.print(F(OK "TURBO_RAW"));
    COMMS.println(command);
    turboTC80._sendTelegramRaw(command);
    turboTC80.receiveTelegram(true);
}

void cmd_turbo_reset(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO_RESET", "", "Resets turbo pump parameters for default operation in normal conditions (light gases, default speed control mode, power limit 100%)");
    COMMS.println(F(OK "TURBO_RESET"));

    // Set gas mode to light gases.
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::GasMode, PfeifferVacProtocol::UShortInt((uint8_t)PfeifferVacProtocol::FuncGasMode::LightGases), true);
    turboTC80.receiveTelegram(true);
    nonBlockDelay(20); // TC80 Likes a delay between queries.

    // Reset power limit to 100% power use.
    turboTC80.sendCommand(PfeifferVacProtocol::ReferenceValueInput::PwrSVal, PfeifferVacProtocol::UShortInt(100));
    turboTC80.receiveTelegram(true);
    nonBlockDelay(20); // TC80 Likes a delay between queries.

    // Turn off custom speed setting mode to go back to pfeiffer default speed control mode
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::SpdSetMode, PfeifferVacProtocol::UShortInt((uint16_t)PfeifferVacProtocol::FuncYesNo::No));
    turboTC80.receiveTelegram(true);
    nonBlockDelay(20); // TC80 Likes a delay between queries.

    // Set rotation speed back to 100% speed.                                                                                        // TC80 Likes a delay between queries.
    turboTC80.sendCommand(PfeifferVacProtocol::ReferenceValueInput::SpdSVal, PfeifferVacProtocol::UReal(100.0));
    turboTC80.receiveTelegram(true);
    nonBlockDelay(20); // TC80 Likes a delay between queries.

    // Make sure the pumping station is on and prior errors have been acknowledged. (note that the "Pumping station" on is the electronics & other hardware, but does not cause the  motor to spin up)
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::PumpgStatn, PfeifferVacProtocol::BooleanOld((bool)PfeifferVacProtocol::FuncPumpingStation::OnAndAckn), true); // Turn on pumping station (acknowledging errors)
    turboTC80.receiveTelegram(true);

    COMMS.println(F(OK "TURBO_RESET COMPLETE"));
}

void cmd_turbo_clear_errors(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO_CLEAR_ERRORS", "", "Clears turbo pump errors and warning messages");
    COMMS.println(F(OK "TURBO_CLEAR_ERRORS"));
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::ErrorAckn, PfeifferVacProtocol::BooleanOld(true), true);
    turboTC80.receiveTelegram(true);
    COMMS.println(F("| TURBO_CLEAR_ERRORS COMPLETE"));
}

void cmd_full_startup(LazySerial::Context &context)
{
    LAZY_COMMAND("STARTUP", "", "Run full auto startup sequence.");
    COMMS.println(F(OK "Autostarting system now."));
    // Power on sequence

    // 1. Power on roughing pump
    ROUGHING_PWR.turnOn();
    COMMS.println(F("| Roughing Pump ON"));
    nonBlockDelay(100); // Wait 100ms to avoid current spikes

    // 2. Power on fluid pump
    fluidPump.setSpeed(100);
    COMMS.println(F("| Fluid Pump ON 100%"));
    nonBlockDelay(100); // Wait 100ms to avoid current spikes

    // 3. Power on PH Probe or Accessory
    ACCESSORY_PWR.turnOn();
    COMMS.println(F("| " ACCESSORY_NAME " ON"));

    // !!!! IMPORTANT WAIT 60 Seconds so that roughing pump has sufficiently pumped down vacuum !!!!!
    COMMS.println(F("| Waiting 60 seconds for rough out..."));
    nonBlockDelay(60000); // Wait 60 seconds !!!!!

    if (beatActive)
        return; // do not finish startup sequence if beat was callled.

    COMMS.println(F("| Turning ON Turbo Pump..."));
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::GasMode, PfeifferVacProtocol::UShortInt((uint8_t)PfeifferVacProtocol::FuncGasMode::LightGases), true);
    turboTC80.receiveTelegram(true);
    nonBlockDelay(20); // TC80 Likes a delay between queries.
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::MotorPump, PfeifferVacProtocol::BooleanOld((uint8_t)PfeifferVacProtocol::FuncOnOff::On), true);
    turboTC80.receiveTelegram(true);
    nonBlockDelay(20);                                                                                                                                                          // TC80 Likes a delay between queries.
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::PumpgStatn, PfeifferVacProtocol::BooleanOld((uint8_t)PfeifferVacProtocol::FuncPumpingStation::OnAndAckn), true); // Turn on pumping station (acknowledging errors)
    turboTC80.receiveTelegram(true);
    nonBlockDelay(20); // TC80 Likes a delay between queries.
    COMMS.println(F("| Startup Complete"));
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
            COMMS.println(F(OK "AUTOSTART ON"));
            save_settings();
        }
        else if (strcasecmp(param, "OFF") == 0)
        {
            saved_settings.autostart_on = false;
            COMMS.println(F(OK "AUTOSTART OFF"));
            save_settings();
        }
        else
        {
            COMMS.println(F(OK "AUTOSTART SET TO"));
            char *end;
            unsigned long delay = strtoul(param, &end, 10);
            LAZY_RETURN_USAGE_UNLESS(end > param) // check that we parsed some number and not just garbage
            saved_settings.autostart_delay = delay;
            saved_settings.autostart_on = true; // if we're setting a delay, we should also turn autostart on
            save_settings();
        }
    }
    else
    {
        COMMS.println(F(OK "AUTOSTART SET TO"));
    }
    LOG_INFO(F("| AUTOSTART"));
    LOG_INFO(saved_settings.autostart_on ? " ON " : " OFF ");
    LOG_INFO(F(" AUTOSTART_DELAY: "));
    LOG_INFO(saved_settings.autostart_delay, "\n");
    if (!hasParam)
        COMMS.println(F("| [To manually trigger startup routine, use command: STARTUP]\n"));
}

void cmd_reset_settings(LazySerial::Context &context)
{
    LAZY_COMMAND("RESET_SETTINGS", "", "Resets settings saved on the ISMS to their default values (does not change TURBO pump or RGA parameters)");
    COMMS.println(F(OK "RESET_SETTINGS"));
    reset_settings();
    COMMS.println(F("| RESET_SETTINGS COMPLETE"));
}

LazySerial::CallbackFunction basic_comms_commands[] = {
    cmd_help,
    cmd_beat,
    cmd_full_startup,
    cmd_roughing_on_off,
    cmd_accessory_pwr,
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
    cmd_reset_settings,
    cmd_turbo_reset,
    cmd_turbo_query,
    cmd_turbo_cmd,
    cmd_turbo_raw,
    cmd_gpio,
};

LazySerial::CallbackFunction all_comms_commands[] = {
    cmd_help,
    cmd_beat,
    cmd_version,
    cmd_set_debug_loglevel,
    cmd_set_stats_broadcast_interval,
    cmd_autostart_on_off,
    cmd_full_startup,
    cmd_roughing_on_off,
    cmd_accessory_pwr,
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
    nonBlockDelay(4000); // give the human more time to read commands
}

/** Prints out the debug/troubleshooting commands and their usage as a table */
void debug_help_table(LazySerial::Context &context)
{
    print_padded(COMMS, F("+====== DEBUGGING COMMANDS "), lazy.d_usage_column_width + 3, '=');
    COMMS.println(F("==================="));
    print_cmd_help_table(debug_comms_commands, sizeof(debug_comms_commands) / sizeof(debug_comms_commands[0]));
    nonBlockDelay(4000); // give the human more time to read commands
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
