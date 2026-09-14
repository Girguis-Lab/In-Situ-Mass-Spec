// Serial command handlers for the ISMS V3 operator interface.
//
// Every command an operator can type on the COMMS port is implemented here as
// a LazySerial callback: power switching, fluid pump and turbo pump control,
// the automated startup sequence, and the settings that persist in EEPROM.
//
// Each handler opens with LAZY_COMMAND(), which declares the command's name,
// usage string, and description. That macro is what makes one function serve
// several purposes: LazySerial calls the same handler to match incoming text,
// to print a usage error, and to harvest metadata for the help tables, and the
// macro returns early in every mode but the last.
//
// The arrays near the bottom split the handlers into the everyday command set
// and the debugging set that HELP DEBUG reveals, plus the combined list that
// LazySerial actually matches against - make sure to add future commands
// to BOTH a everyday/debug list AND the combined list.

#pragma once
#include "includes.h"

// --------- Helper Functions -----------

// Applies an ON/OFF argument to a pin, for commands that do nothing else.
//
// Drives `pwrPin` high for "ON" or "1" and low for "OFF" or "0", matched case
// insensitively, and echoes the resulting state to `stream`. Any other value
// of `onoff` prints a usage error and leaves the pin untouched.
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

// Runs the startup sequence to bring the ISMS from a cold stop to ready.
// Note that you will want to wait much longer after this startup (hours) before
// taking readings if  the vacuum chamber has not been pumped down recently or
// has switched from  a different gas environment (such as going from land
// testing to ocean use).

// startedByUser is a flag that will disable BEAT detection so a manual triggered
// startup sequence will always run to completion.

// Powers the roughing pump, then the fluid pump at 100%, then the accessory
// port, spacing them 100 ms apart so their inrush currents do not overlap.
// Then waits a full 60 seconds for the roughing pump to pull the foreline down
// before configuring and starting the turbo pump -- starting the turbo pump
// into a poor foreline vacuum is what this delay exists to prevent, so do not
// shorten it.
//
// Runs for over a minute, but waits through nonBlockDelay(), so serial
// commands are still answered and the watchdog is still petted throughout.
// Returns without touching the turbo pump if BEAT arrives during the wait.
// Also called directly from setup() when autostart is enabled.
void run_startup_sequence(bool startedByUser)
{
    // do not finish automatic startup sequence if beat was callled.
    if (beatActive && !startedByUser)
    {
        COMMS.println(F("| BEAT Recived. Exiting Startup..."));
        return;
    }

    if (startedByUser)
        COMMS.println(F(OK "Starting system now."));
    else
        COMMS.println(F(OK "Autostarting system now."));

    // Power on sequence

    // 1. Power on roughing pump
    ROUGHING_PWR.turnOn();
    COMMS.println(F("| Roughing Pump ON"));
    nonBlockDelay(100); // Wait 100ms to avoid current spikes

    // do not finish automatic startup sequence if beat was callled.
    if (beatActive && !startedByUser)
    {
        COMMS.println(F("| BEAT Recived. Exiting Startup..."));
        return;
    }

    // 2. Power on fluid pump
    fluidPump.setSpeed(100);
    COMMS.println(F("| Fluid Pump ON 100%"));
    nonBlockDelay(100); // Wait 100ms to avoid current spikes

    // do not finish automatic startup sequence if beat was callled.
    if (beatActive && !startedByUser)
    {
        COMMS.println(F("| BEAT Recived. Exiting Startup..."));
        return;
    }

    // 3. Power on PH Probe or Accessory
    ACCESSORY_PWR.turnOn();
    COMMS.println(F("| " ACCESSORY_NAME " ON"));

    // do not finish automatic startup sequence if beat was callled.
    if (beatActive && !startedByUser)
    {
        COMMS.println(F("| BEAT Recived. Exiting Startup..."));
        return;
    }

    // !!!! IMPORTANT WAIT 60 Seconds so that roughing pump has sufficiently pumped down vacuum !!!!!
    COMMS.println(F("| Waiting 60 seconds for rough out..."));

    // Wait 60 seconds while checking for beat command (rollover safe time check) !!!!!
    unsigned long startTime = millis();
    while (millis() - startTime < 60000)
    {
        nonBlockDelay(1);
        // do not finish automatic startup sequence if beat was callled.
        if (beatActive && !startedByUser)
        {
            COMMS.println(F("| BEAT Recived. Exiting Startup..."));
            return;
        }
    }

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

// --------- User Commands -----------

// forward function delcaration - implemented later in the file. HELP is listed
// in the command tables below, but it can only be implemented after the tables
// it prints have been defined.
void cmd_help(LazySerial::Context &context);

// DEBUG_LOGGING <OFF|LOW|HIGH>: sets how much troubleshooting detail is
// written to the operator serial port, and saves the choice to EEPROM.
//
// The three operator-facing levels map onto the DebugLog library's INFO, DEBUG
// and TRACE levels. Nothing below INFO is ever selected, because those levels
// would also suppress the periodic stats telegram that the topside software
// depends on.
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

// STATS_INTERVAL <MS|OFF>: sets how often the periodic stats telegram is
// broadcast, in milliseconds, or turns it off entirely, saving the choice to
// EEPROM.
//
// Prints the resulting state whether or not it changed, including when no
// argument was supplied, so the command doubles as a way to read the current
// interval back.
//
// The interval is bounded rather than taken as given. Too small and telegrams
// run back to back and saturate the 9600 baud link, leaving no room to type
// the command that would undo it -- and since the value is saved to EEPROM,
// that state would survive a power cycle. Use OFF to stop the telegram; that
// is what it is for.
void cmd_set_stats_broadcast_interval(LazySerial::Context &context)
{
    LAZY_COMMAND("STATS_INTERVAL", "<100-3600000|OFF>", "Sets how often the system broadcasts stats to serial in milliseconds. Send OFF to disable stats logging.");
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
            LAZY_RETURN_USAGE_UNLESS(interval >= STATS_INTERVAL_MIN_MS && interval <= NONBLOCK_DELAY_MAX_MS);
            saved_settings.stats_log_interval = interval;
            saved_settings.stats_loging_enabled = true;
        }
        save_settings();
    }

    // Print the current state:
    if (!saved_settings.stats_loging_enabled)
    {
        LOG_INFO(OK "STATS_INTERVAL OFF - periodic stats logging disabled\n");
    }
    else
    {
        LOG_INFO(F(OK "STATS_INTERVAL ON - Logging interval set to "));
        LOG_INFO(saved_settings.stats_log_interval);
        LOG_INFO(F("ms\n"));
    }
}

// VERSION: prints the firmware version together with the date and time this
// binary was compiled, so an instrument in the field can be matched back to
// the source it was built from.
void cmd_version(LazySerial::Context &context)
{
    LAZY_COMMAND("VERSION", "", "Prints firmware version");
    COMMS.println(F(OK "VERSION " FIRMWARE_VERSION " COMPILED AT " __TIMESTAMP__));
}

// PINOUT: prints the pin assignments this firmware was compiled with, plus the
// turbo pump's RS485 address.
//
// Useful for checking a binary against the board in front of you before
// chasing a wiring fault -- the pins are compile-time constants from
// includes.h, not something the firmware discovers.
void cmd_pinout(LazySerial::Context &context)
{
    LAZY_COMMAND("PINOUT", "", "Prints configured arduino pinout and I2C/RS485 addresses");
    COMMS.println(F(OK "PINOUT " LAZY_KEYVAL(PIN_LED1) LAZY_KEYVAL(PIN_LED2) LAZY_KEYVAL(PIN_PWR_ROUGHING) LAZY_KEYVAL(PIN_PWR_PH) LAZY_KEYVAL(PIN_PWR_FLUIDPUMP) LAZY_KEYVAL(PIN_ANALOG_FLUIDPUMP_SPEED)));
    COMMS.print(F("ADDRESSES: Turbo Pump RS485 Address="));
    COMMS.println(LAZY_KEYVAL(turboTC80.getAddress()));
}

// GPIO <pin number> <ON|OFF>: drives any Arduino pin high or low.
//
// A bench-testing aid with no interlocks of any kind: it will reconfigure a
// pin that a pump or the RS485 transceiver is already driving. Registered only
// in the debug command set for that reason.
void cmd_gpio(LazySerial::Context &context)
{
    LAZY_COMMAND("GPIO", "<0-" GPIO_MAX_PIN_STR "> <ON|OFF>", "Set any Arduino pin high or low - TESTING ONLY, DO NOT USE WITHOUT KNOWING WHAT YOU'RE DOING.");
    int pin = 0;
    char *onoff;
    // The upper bound must be the real pin count: digitalWrite() indexes
    // digital_pin_to_port_PGM[] with no bounds check of its own, so a larger
    // number reads past the end of that flash table and can end up writing an
    // arbitrary I/O port.
    bool ok = context.parse_int_minmax(&pin, 0, (int)GPIO_MAX_PIN);
    LAZY_RETURN_USAGE_UNLESS(ok);
    ok = context.parse_word(&onoff);
    LAZY_RETURN_USAGE_UNLESS(ok);

    powerPin pwrPin((uint8_t)pin);
    COMMS.print(OK "GPIO ");
    COMMS.print(pin);
    COMMS.print(" ");
    handle_onoff_pin_command(&pwrPin, onoff, context.stream);
}

// BEAT: cancels the automatic startup sequence for the rest of this boot,
// enforcing fully manual control.
//
// Sets the flag that setup() and cmd_full_startup() both check, so an operator
// who gets this in before the autostart delay expires -- or during the 60
// second rough-out wait, which nonBlockDelay() keeps responsive -- stops the
// instrument from bringing itself up. The flag is not persisted: the next
// reset restores whatever AUTOSTART is set to.
void cmd_beat(LazySerial::Context &context)
{
    LAZY_COMMAND("BEAT", "", "Send to stop autostart for this boot, used to temporarily enforce fully manual control");
    beatActive = true;
    COMMS.println(OK "BEAT");
}

// --------------------------------------------------
// ------------- POWER ON/OFF commands --------------------
// --------------------------------------------------

// ROUGHING <ON|OFF>: switches power to the roughing vacuum pump.
//
// The roughing pump has to be running to establish the foreline vacuum the
// turbo pump needs, so switching it off while the turbo pump is spinning is an
// operator error this command does not guard against.
void cmd_roughing_on_off(LazySerial::Context &context)
{
    LAZY_COMMAND("ROUGHING", "<ON|OFF>", "Turns the roughing pump power on or off");
    char *onoff;
    bool ok = context.parse_word(&onoff);
    LAZY_RETURN_USAGE_UNLESS(ok);
    COMMS.print(F(OK "ROUGHING "));
    handle_onoff_pin_command(&ROUGHING_PWR, onoff, context.stream);
}

// pH <ON|OFF>: switches power to the accessory port.
//
// The command's name and its output both come from ACCESSORY_NAME, which
// tracks what is actually fitted to the port -- normally a pH probe, or the
// cryo pump on instruments equipped with one -- so the command an operator
// types differs between builds.
void cmd_accessory_pwr(LazySerial::Context &context)
{
    LAZY_COMMAND(ACCESSORY_NAME, "<ON|OFF>", "Turns the " ACCESSORY_NAME " power on or off");
    char *onoff;
    bool ok = context.parse_word(&onoff);
    LAZY_RETURN_USAGE_UNLESS(ok);
    COMMS.print(F(OK "" ACCESSORY_NAME " "));
    handle_onoff_pin_command(&ACCESSORY_PWR, onoff, context.stream);
}

// FLUIDPUMP <ON|OFF>: starts the fluid pump at its saved rate, or stops it.
//
// ON applies the rate saved in EEPROM; if that rate is 0 the pump would not
// actually turn, so it is first raised to 50% and saved, on the assumption
// that an operator asking for ON wants flow. OFF sets the speed to 0, which
// also opens the pump's power relay. Reports an error if the pump object was
// never initialized.
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

// FLUIDPUMP_RATE <-100-100>: sets the fluid pump speed as a percentage of full
// speed and saves it to EEPROM as the rate FLUIDPUMP ON will use.
//
// Negative values run the pump in reverse and 0 cuts power to it. The rate is
// saved before it is applied, so it persists even if the pump itself reports
// that it has not been initialized, in which case an error is printed in place
// of the applied speed.
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

// TURBO <ON|OFF>: starts or stops the turbo pump's motor at whatever speed the
// pump is currently configured for.
//
// Sends the Pfeiffer MotorPump control command and prints the pump's reply.
// Spinning up takes several minutes, and this does not touch the roughing
// pump, so rough vacuum must already be established -- STARTUP sequences both
// pumps correctly.
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

// TURBO_SPEED <0.0-100.0>: runs the turbo pump at a fixed fraction of its
// maximum speed, which is 90,000 RPM on the Pfeiffer TC80.
//
// Enables the pump's set-speed mode and then writes the target. An argument of
// 0 instead disables set-speed mode, handing speed control back to the pump's
// own default logic. Running below full speed saves power but degrades the
// vacuum, and too low a speed risks the RGA filament -- see the warning
// emitted by log_stats().
void cmd_turbo_speed(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO_SPEED", "<0.0-100.0>", "Sets the turbo pump to run at a target speed as percent of max speed (100% is 90,000 RPM for the Pfeiffer TC80) - Send 0 to reset to pfeiffer default speed control mode");
    float param = 0;
    bool ok = context.parse_float_minmax(&param, (float)0.0, (float)100.0);
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
        float speedValue = param;
        turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::SpdSetMode, PfeifferVacProtocol::UShortInt((uint16_t)1)); // turn on custom speed setting mode & diable pfeiffer default speed control mode
        turboTC80.receiveTelegram(true);
        nonBlockDelay(20); // TC80 Likes a delay between queries.
        turboTC80.sendCommand(PfeifferVacProtocol::ReferenceValueInput::SpdSVal, PfeifferVacProtocol::UReal(speedValue));
        turboTC80.receiveTelegram(true);
    }
}

// TURBO_LIMIT_PWR <10-100>: caps the turbo pump's power draw as a percentage of
// full power, which is a 5 A draw on the TC80.
//
// Lowering the cap slows spin-up and is how the pump is kept inside a
// constrained power budget. The argument is accepted over 10-100 and anything
// outside that -- or unparseable -- prints the usage message and leaves the
// pump alone; log_stats() warns whenever the cap is below 100 so a forgotten
// limit does not get mistaken for a failing pump.
void cmd_turbo_limit_pwr(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO_LIMIT_PWR", "<10-100>", "Sets the turbo pump power limit as a percentage of full power (Pfeiffer max draw is 5A)");
    COMMS.print(F(OK "TURBO_LIMIT_PWR "));
    int percent = 100;
    bool ok = context.parse_int_minmax(&percent, 10, 100);
    LAZY_RETURN_USAGE_UNLESS(ok);
    COMMS.println(percent);
    bool isValid;
    turboTC80.sendCommand(PfeifferVacProtocol::ReferenceValueInput::PwrSVal, PfeifferVacProtocol::UShortInt((uint16_t)percent));
    turboTC80.receiveUShortInt(PfeifferVacProtocol::ReferenceValueInput::PwrSVal, isValid, true);
}

// TURBO_CMD <PARAM> <DATA>: writes an arbitrary Pfeiffer parameter, named by
// its three-digit number, with `DATA` passed through unchanged, then prints
// the pump's reply.
//
// The escape hatch for parameters this firmware has no dedicated command for.
// See the pfeiffer TC80 Manual for a full command list.
// Both arguments are required -- a Pfeiffer write telegram carries no
// meaningful empty payload, so there is nothing to send without DATA. Use
// TURBO_QUERY to read a parameter instead. The data string must already be in
// the askii format that parameter expects, so consult the TC80 manual first.
void cmd_turbo_cmd(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO_CMD", "<PARAM> <DATA>", "Send a command to the turbo pump with the 3-digit parameter number and a data string (use TURBO_QUERY to read a parameter)");

    uint16_t param = 0;
    bool ok = context.parse_int_minmax(&param, (uint16_t)0, (uint16_t)999);
    LAZY_RETURN_USAGE_UNLESS(ok);

    char *dataStr = nullptr;
    ok = context.parse_word(&dataStr);
    LAZY_RETURN_USAGE_UNLESS(ok);

    turboTC80.sendCommand(param, dataStr, true);
    turboTC80.receiveTelegram(true);
}

// TURBO_QUERY <PARAM>: reads an arbitrary Pfeiffer parameter by its
// three-digit number and prints the raw reply telegram, decoding nothing.
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

// TURBO_RAW <COMMAND>: writes a raw ASCII telegram onto the turbo pump's RS485
// line and prints whatever comes back.
//
// Bypasses telegram construction entirely -- only the terminating carriage
// return is added, so the address, action, parameter, data length and checksum
// all have to be correct in what is typed. For protocol debugging only.
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

// TURBO_RESET: returns the turbo pump to the configuration the ISMS expects
// for normal operation.
//
// Selects the light-gases mode, restores the 100% power limit, hands speed
// control back to the pump's own logic with the target at full speed, and
// turns the pumping station on while acknowledging any latched errors. Note
// that turning the pumping station on powers the pump electronics but does not
// spin the motor up -- TURBO ON does that. Short delays separate the commands
// because the TC80 drops telegrams that arrive back to back.
//
// Only the pump is touched; the RGA and the settings this firmware saves are
// left alone.
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

// TURBO_CLEAR_ERRORS: acknowledges the turbo pump's latched errors and
// warnings.
//
// Clears the pump's report of a fault, not the fault itself -- read the error
// out of the stats telegram before dismissing it.
void cmd_turbo_clear_errors(LazySerial::Context &context)
{
    LAZY_COMMAND("TURBO_CLEAR_ERRORS", "", "Clears turbo pump errors and warning messages");
    COMMS.println(F(OK "TURBO_CLEAR_ERRORS"));
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::ErrorAckn, PfeifferVacProtocol::BooleanOld(true), true);
    turboTC80.receiveTelegram(true);
    COMMS.println(F("| TURBO_CLEAR_ERRORS COMPLETE"));
}

// STARTUP: brings everything (the vacuum system, fluid pump, and pH probe or accessory) up from a powered off state, in the order/timing the hardware
// requires.
void cmd_full_startup(LazySerial::Context &context)
{
    LAZY_COMMAND("STARTUP", "", "Run full startup sequence - same as run in auto mode.");
    run_startup_sequence(true);
}

// SET_AUTOSTART <ON|OFF|DELAY>: controls whether STARTUP runs by itself after boot,
// and how long after boot it runs.
//
// ON and OFF toggle the behavior; a number is taken as the delay in
// milliseconds and enables autostart as well, since setting a delay only makes
// sense if the sequence is going to run. With no argument the current setting
// is reported and nothing changes. Changes are saved to EEPROM, so this is
// what makes an instrument bring itself up after an unattended power cycle.
void cmd_set_autostart_on_off(LazySerial::Context &context)
{
    LAZY_COMMAND("SET_AUTOSTART", "<ON|OFF|DELAY>", "Set whether the system should startup automatically. If an integer is passed, the delay in milliseconds after boot when autostart should happen.");
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
            char *end;
            unsigned long delay = strtoul(param, &end, 10);
            LAZY_RETURN_USAGE_UNLESS(end > param) // check that we parsed some number and not just garbage
            // Reject rather than clamp. A delay past this cannot be waited on
            // -- micros() rolls over every 71.58 minutes -- and silently
            // substituting a shorter one would boot the instrument at a time
            // the operator did not choose, every boot, since this is saved.
            if (delay > NONBLOCK_DELAY_MAX_MS)
            {
                COMMS.print(F(ERROR " AUTOSTART delay too long, maximum is "));
                COMMS.print(NONBLOCK_DELAY_MAX_MS);
                COMMS.println(F("ms (1 hour)"));
                return;
            }
            COMMS.println(F(OK "AUTOSTART SET TO"));
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

// RESET_SETTINGS: restores the settings this firmware keeps in EEPROM to their
// compiled-in defaults, taking effect immediately.
//
// Autostart, the stats interval, the log level and the saved fluid pump rate
// all revert. Turbo pump and RGA parameters live in those instruments rather
// than in EEPROM and are untouched -- use TURBO_RESET for the pump.
void cmd_reset_settings(LazySerial::Context &context)
{
    LAZY_COMMAND("RESET_SETTINGS", "", "Resets settings saved on the ISMS to their default values (does not change TURBO pump or RGA parameters)");
    COMMS.println(F(OK "RESET_SETTINGS"));
    reset_settings();
    COMMS.println(F("| RESET_SETTINGS COMPLETE"));
}

// The commands HELP lists: everything needed to operate the instrument
// normally. Used only for building that table, not for matching input.
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
    cmd_set_autostart_on_off,
};

// The extra commands HELP DEBUG reveals: diagnostics, raw pump access, and
// settings that should not be changed casually. Also table-building only.
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

// Every command LazySerial matches operator input against -- the union of the
// two lists above, and the only one of the three registered in setup(). Kept
// as its own array because joining the other two at runtime would cost RAM
// the ATmega2560 cannot spare; see the commented-out concatArrays() attempt in
// setup(). A command added above must be added here too, or it will appear in
// the help table without being accepted.
LazySerial::CallbackFunction all_comms_commands[] = {
    cmd_help,
    cmd_beat,
    cmd_version,
    cmd_set_debug_loglevel,
    cmd_set_stats_broadcast_interval,
    cmd_set_autostart_on_off,
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

// Prints the `commands_list_size` handlers in `commands_list` as an askii
// table of command names, usage strings, and descriptions.
//
// Calls each handler in GET_METADATA mode, which makes LAZY_COMMAND() fill the
// name, usage and description into the context and return instead of executing
// the command. Handlers that supply no usage or description are skipped.
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

// Prints the table of everyday commands, then pauses 4 seconds so the operator
// can read it before the periodic stats telegram scrolls it away. Registered
// as LazySerial's default help callback, so it is also what an unrecognized
// command prints.
void basic_help_table(LazySerial::Context &context)
{
    print_padded(COMMS, F("+====== AVAILABLE COMMANDS "), lazy.d_usage_column_width + 3, '=');
    COMMS.println(F("==================="));
    print_cmd_help_table(basic_comms_commands, sizeof(basic_comms_commands) / sizeof(basic_comms_commands[0]));
    nonBlockDelay(4000); // give the human more time to read commands
}

// Prints the table of debugging and troubleshooting commands, with the same 4
// second reading pause as the basic table.
void debug_help_table(LazySerial::Context &context)
{
    print_padded(COMMS, F("+====== DEBUGGING COMMANDS "), lazy.d_usage_column_width + 3, '=');
    COMMS.println(F("==================="));
    print_cmd_help_table(debug_comms_commands, sizeof(debug_comms_commands) / sizeof(debug_comms_commands[0]));
    nonBlockDelay(4000); // give the human more time to read commands
}

// HELP <DEBUG>: lists the everyday commands with their usage and description,
// or the troubleshooting commands instead when DEBUG is given.
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

// The help output LazySerial prints when it does not recognize a command:
// the basic table, so an operator who mistypes is shown the commands they are
// likely to want rather than the full debug list. Registered in setup().
const auto &lazy_help_callback = basic_help_table;
