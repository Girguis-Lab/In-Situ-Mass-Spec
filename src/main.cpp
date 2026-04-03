
/* ISMS code for the ATMEL2560 based ISMS controller
  This is for the 2025/26 "V3" redesign of the ISMSs with dedicated PCB
  Use F() for long strings so they get stored in the flash and not RAM

  TODO Edit this comment:
  Includes external pump control using the comand U SPRATEXXX where XXX is the pumping rate in percent
  of full capacity (100 is 100% capacity or 312 mL/min, 000 is 0% or pump off, 050 corresponds to 50%
  capacity or 156 mL/min; 100% is 3.2VDC or 312 mL/min).

  Includes 'autostart' which will automatically start the system (power up SBE, pH probe, KNF1, then
  spins up turbo).
*/

#include "includes.h"

bool autostarted = false;           // Becomes true once autostart has happened, so we don't do it again
bool commsBufferOverflowed = false; // flag to mark if the comms serial buffer ever has overflowed (meaning we sent too much data too fast and some got lost)

// A non-blocking delay function that allows lazy serial and watchdog resets to keep working while paused in various places.
void nonBlockDelay(unsigned long ms)
{
  unsigned long start = micros();
  while (micros() - start < ms * 1000) // Non-blocking delay, handles rollover
  {
    if (COMMS.availableForWrite() == 0)
    {
      commsBufferOverflowed = true;
    }
    if (COMMS.available() > 0)
    {
      ms += 2000; // If we're receiving a command pause other scripts to give the user more time to finish their command instead of risking cutting them off in the middle of typing
    }
    lazy.loop();
    resetWDT();
  }
}

void log_value(const char *name, float value)
{
  if (COMMS.availableForWrite() == 0)
    return;
  COMMS.print(name);
  COMMS.print("(");
  COMMS.print(value, 5);
  COMMS.print("):");
  COMMS.print(value);
  COMMS.print(",");
}

void log_value_w_status(const char *name, float value, float minOk, float maxOk)
{
  if (COMMS.availableForWrite() == 0)
    return;
  COMMS.print(name);
  COMMS.print(":");
  COMMS.print(value);
  if (value >= minOk && value <= maxOk)
  {
    COMMS.print("✅");
  }
  else
  {
    COMMS.print("❌");
  }

  COMMS.print(",");
}

void log_value_postfix(float value, float minOk, float maxOk, bool gotValidResponse)
{
  if (!gotValidResponse || value <= minOk || value >= maxOk)
  {
    Log.info("(!)");
  }
  Log.info(",");
}

// Command to set the log level of the system. Usage: LOGLEVEL <0-6> where 0 is silent and 6 is verbose.
void cmd_set_loglevel(LazySerial::Context &context)
{
  LAZY_COMMAND("DEBUG_LOGGING", "<LEVEL>", "Sets the log level of the system. LEVEL is one of SILENT, FATAL, ERROR, WARNING, INFO, TRACE, VERBOSE")
  char *levelStr;
  bool ok = context.parse_word(&levelStr);
  if (ok)
  {
    int level = logLevelToInt(levelStr);
    LAZY_RETURN_USAGE_UNLESS(level >= 0); // logLevelToInt returns -1 if the string is not a valid log level
    Log.setLevel(level);
    saved_settings.log_level = level;
    save_settings();
  }
  Log.info("LOGLEVEL is %s (level %d)", logLevelToString(saved_settings.log_level).c_str(), saved_settings.log_level);
}

void cmd_set_stats_log_interval(LazySerial::Context &context)
{
  LAZY_COMMAND("STATS_INTERVAL", "<MS|OFF>", "Sets how often the system logs stats to serial in milliseconds. Send OFF to disable stats logging.");
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

// A command to show a help message listing all commands.
void cmd_help(LazySerial::Context &context)
{
  LAZY_COMMAND("HELP", "", "Shows this help message");
  lazy.cmd_help();
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

// --------------------------------------------------
// ------------- FLUID PUMP CONTROL commands --------------------
// --------------------------------------------------

void cmd_fluidpump_rate(LazySerial::Context &context)
{
  LAZY_COMMAND("FLUIDPUMP_RATE", "<0-100>", "Sets the fluid pumping rate as a percentage of full speed. Send 0 to turn off pump power");
  uint8_t rate = 0;
  bool ok = context.parse_int_minmax(&rate, (uint8_t)0, (uint8_t)100);
  LAZY_RETURN_USAGE_UNLESS(ok);
  int resultRate = fluidPump.setSpeed(rate);
  if (resultRate < 0)
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

void cmd_turbo_init(LazySerial::Context &context)
{
  LAZY_COMMAND("TURBO_INIT", "", "Sets up the turbo pump for operation");
  context.stream.println(F(OK "TURBO_INIT"));
  turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::GasMode, (uint8_t)PfeifferVacProtocol::FuncGasMode::LightGases, true);
  turboTC80.receiveTelegram(true);
  turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::PumpgStatn, (uint8_t)PfeifferVacProtocol::FuncPumpingStation::OnAndAckn, true); // Turn on pumping station (acknowledging errors)
  turboTC80.receiveTelegram(true);
  turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::MotorPump, (uint8_t)PfeifferVacProtocol::FuncOnOff::On);
  turboTC80.receiveTelegram(true);
}

void cmd_turbo_on_off(LazySerial::Context &context)
{
  LAZY_COMMAND("TURBO", "<ON|OFF>", "Turn the turbo motor OFF or ON (at previously set speed)");
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
    context.stream.println(F(ERROR " Invalid argument, use ON or OFF"));
  }
}

void cmd_turbo_speed(LazySerial::Context &context)
{
  LAZY_COMMAND("TURBO_SPEED", "<0-100>", "Sets the turbo pump to run at a target speed as percent of max speed (100% is 90,000 RPM for the Pfeiffer TC80) - Send 0 to reset to pfeiffer default speed control mode");
  // todo what is actual max speed ?
  uint16_t param = 0;
  bool ok = context.parse_int_minmax(&param, (uint16_t)0, (uint16_t)100);
  LAZY_RETURN_USAGE_UNLESS(ok);
  if (param == 0)
  {
    // Send 0 to reset to pfeiffer default speed control mode
    context.stream.println(F(OK "GOT '0' SO RESETING TO DEFAULT PFEIFFER SPEED CONTROL"));
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::SpdSetMode, PfeifferVacProtocol::BooleanNew(false)); // turn off custom speed setting mode to go back to pfeiffer default speed control mode
    turboTC80.receiveTelegram(true);
    turboTC80.sendCommand(PfeifferVacProtocol::ReferenceValueInput::SpdSVal, (uint16_t)65); // pfeiffer default is 65% of max power which corresponds to XXX todo
    turboTC80.receiveTelegram(true);
  }
  else
  {
    uint16_t speedValue = param;                                                                                   // Scale the percentage to a value between 0 and 65535
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::SpdSetMode, PfeifferVacProtocol::BooleanNew(true)); // turn on custom speed setting mode & diable pfeiffer default speed control mode
    turboTC80.receiveTelegram(true);
    turboTC80.sendCommand(PfeifferVacProtocol::ReferenceValueInput::SpdSVal, speedValue);
    turboTC80.receiveTelegram(true);
  }
}

void cmd_turbo_amps_limit(LazySerial::Context &context)
{
  LAZY_COMMAND("TURBO_LIMIT_PWR", "<0-100>", "Sets the turbo pump power limit as a percentage of full power (Pfeiffer max draw is 5A)");
  context.stream.println(F(OK "TURBO_LIMIT_PWR"));
  turboTC80.sendCommand(PfeifferVacProtocol::ReferenceValueInput::PwrSVal, PfeifferVacProtocol::UExpoNew(100));
  bool isValid; // TODO check this funciton
  turboTC80.receiveUInteger(true, isValid);
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

  // queries always have 'action' set to '0' and a query data of "?"
  turboTC80.sendQuery(param, true);
  turboTC80.receiveTelegram(true, 10000);
}

void cmd_turbo_raw(LazySerial::Context &context)
{
  LAZY_COMMAND("TURBO_RAW", "<COMMAND>", "Send raw ASCII to the turbo pump");
  char *command;
  bool ok = context.parse_word(&command);
  LAZY_RETURN_USAGE_UNLESS(ok);
  turboTC80._sendTelegramRaw(command);
}

void cmd_full_startup(LazySerial::Context &context)
{
  LAZY_COMMAND("STARTUP", "", "Start up everything");

  DEBUG.println("Autostarting system now.");
  // Power on sequence
  // 1. Power on roughing pump
  digitalWrite(PIN_PWR_ROUGHING, HIGH);
  DEBUG.println("Roughing Pump ON");
  nonBlockDelay(100); // Wait 100ms to avoid current spikes
  // 2. Power on fluid pump
  digitalWrite(PIN_PWR_FLUIDPUMP, HIGH);
  DEBUG.println("Fluid Pump ON");
  nonBlockDelay(100); // Wait 100ms to avoid current spikes
  // 3. Power on PH Probe
  digitalWrite(PIN_PWR_PH, HIGH);
  DEBUG.println("PH Probe ON");
  // !!!! IMPORTANT WAIT 60 Seconds so that roughing pump has sufficiently pumped down vacuum !!!!!
  nonBlockDelay(60000); // Wait 60 seconds !!!!!
  DEBUG.println("Turning ON Turbo Pump...");
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

LazySerial::CallbackFunction comms_commands[] = {
    cmd_help,
    cmd_version,
    cmd_set_loglevel,
    cmd_set_stats_log_interval,
    cmd_autostart_on_off,
    cmd_full_startup,
    cmd_roughing_on_off,
    cmd_ph_pwr,
    cmd_fluidpump_rate,
    cmd_turbo_init,
    cmd_turbo_on_off,
    cmd_turbo_speed,
    cmd_turbo_amps_limit,
    cmd_turbo_cmd,
    cmd_turbo_query,
    cmd_turbo_raw,
    cmd_pinout,
    cmd_gpio,
};

void setup()
{
  wdtStart();
  resetWDT();
  COMMS.begin(COMMS_BAUDRATE);
  DEBUG.begin(COMMS_BAUDRATE);

  // Print startup messages
  COMMS.println("| ISMS Initializing... [ Firmware Version " FIRMWARE_VERSION " ]");
  wdtPrintStatus(COMMS); // Print the startup status from the watchdog to the COMMS serial for reference

  // Setup commands, logging and load saved settings
  lazy.set_commands(comms_commands);
  Log.begin(LOG_LEVEL_INFO, &COMMS, false);
  load_settings();
  Log.setLevel(saved_settings.log_level);

  // Setup Pins
  pinMode(PIN_LED1, OUTPUT);         // LED 1: Indicator light
  pinMode(PIN_LED2, OUTPUT);         // LED 2: Indicator light
  pinMode(PIN_PWR_PH, OUTPUT);       // Ph probe power pin
  pinMode(PIN_PWR_ROUGHING, OUTPUT); // Roughing vacuum pump power pin

  // Ensure everything is off to start
  digitalWrite(PIN_PWR_ROUGHING, LOW);  // Turn off Roughing vacuum pump
  digitalWrite(PIN_PWR_FLUIDPUMP, LOW); // Turn off fluid pump
  digitalWrite(PIN_PWR_PH, LOW);        // Turn off Ph probe

  // Begin turbo pump serial interface and turn the pump off to start (in case of reboot)
  turboTC80.begin();
  turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::MotorPump, PfeifferVacProtocol::BooleanOld(false));

  // Initialize fluid pump control and ensure it's off to start
  fluidPump.init();
  fluidPump.setSpeed(0); // Ensure fluid pump is off

  // LED1 on to indicate setup done
  digitalWrite(PIN_LED1, HIGH);

  // // initilize system
  // TODO Finish Autostart
  // nonBlockDelay(AUTOSTART_DELAY);
  // if (autostart && !autostarted)
  // {
  //   autostarted = true;
  //   DEBUG.println("Autostarting system now.");
  //   // Power on sequence
  //   // 1. Power on roughing pump
  //   digitalWrite(PIN_PWR_ROUGHING, HIGH);
  //   nonBlockDelay(1000); // Wait 1 second
  //   // 2. Power on fluid pump
  //   digitalWrite(PIN_PWR_FLUIDPUMP, HIGH);
  //   nonBlockDelay(1000); // Wait 1 second
  //   // 3. Power on PH Probe
  //   digitalWrite(PIN_PWR_PH, HIGH);
  //   nonBlockDelay(60000); // Wait 60 seconds
  //   // 4. Initialize turbo pump
  //   DEBUG.println("System autostart complete.");
  // }

  // turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::DrvCurrent, true);
}

void log_stats()
{

  String warnings = "";
  bool responseIsValid = true;

  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::SetRotSpdRpm, false);
  uint32_t turboRpmTarget = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::SetRotSpdRpm, responseIsValid, false);
  Log.info("Turbo_RPM_Target:%d", turboRpmTarget);
  if (!responseIsValid)
    warnings += "Turbo_RPM_Target got invalid serial data. ";
  log_value_postfix((float)turboRpmTarget, 50000, 95000, responseIsValid);

  responseIsValid = true;
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::ActualSpdRpm, false);
  uint32_t turboRpm = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::ActualSpdRpm, responseIsValid, false);
  Log.info("Turbo_RPM:%d", turboRpm);
  if (!responseIsValid)
    warnings += "Turbo_RPM got invalid serial data. ";
  log_value_postfix((float)turboRpm, 50000, 95000, responseIsValid);

  responseIsValid = true;
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::DrvCurrent, false);
  float turboCurrent = turboTC80.receiveUReal(PfeifferVacProtocol::StatusRequest::DrvCurrent, responseIsValid, false);
  Log.info("Turbo_Current:%fA", turboCurrent);
  if (!responseIsValid)
    warnings += "Turbo_Current got invalid serial data. ";
  log_value_postfix(turboCurrent, 0.2, 5.2, responseIsValid);

  responseIsValid = true;
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::TempRotor, false);
  uint32_t turboRotorTemp = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::TempRotor, responseIsValid, false);
  Log.info("Turbo_Rotor_Temp:%dC", turboRotorTemp);
  if (!responseIsValid)
    warnings += "Turbo_Rotor_Temp got invalid serial data. ";
  log_value_postfix((float)turboRotorTemp, 0, 60, responseIsValid);

  Log.info("Fluidpump_Rate:%d%%", fluidPump.getSpeed());
  log_value_postfix((float)fluidPump.getSpeed(), 20, 100, true);

  if (warnings.length() > 0)
  {
    Log.warning("WARN:[ %s ]", warnings.c_str());
  }

  Log.warningln("");
}

void loop()
{
  nonBlockDelay(saved_settings.stats_log_interval); // Main loop
  if (saved_settings.stats_loging_enabled)
  {
    log_stats();
  }
}
