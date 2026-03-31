
/* ISMS code for the ATMEL2560 based ISMS controller
  This is for the 2025 redesign of the ISMSs with the dedicated PCB
  Use F() for long strings so they get stored in the flash and not RAM

  Includes external pump control using the comand U SPRATEXXX where XXX is the pumping rate in percent
  of full capacity (100 is 100% capacity or 312 mL/min, 000 is 0% or pump off, 050 corresponds to 50%
  capacity or 156 mL/min; 100% is 3.2VDC or 312 mL/min).

  Includes 'autostart' which will automatically start the system (power up SBE, pH probe, KNF1, then
  spins up turbo).
*/

#include "includes.h"

bool autostart = false;             // If true, the system will autostart on bootup after a 5 second delay
bool autostarted = false;           // Becomes true once autostart has happened, so we don't do it again
bool commsBufferOverflowed = false; // flag to mark if the comms serial buffer ever has overflowed (meaning we sent too much data too fast and some got lost)

// A non-blocking delay function that allows lazy serial and watchdog resets to keep working while paused in various places.
void nonBlockDelay(unsigned long ms)
{
  unsigned long start = micros();
  while (micros() - start < ms * 1000) // Non-blocking delay, handles rollover
  {
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

// Helper function to handle ON/OFF commands
// If the *onoff string passed is "ON" or "OFF" (case insensitive), the passed pin number is set HIGH or LOW respectively.
void handle_onoff_command(uint8_t pin, char *onoff, LazySerial::Context &context)
{
  if (strcasecmp(onoff, "ON") == 0)
  {
    digitalWrite(pin, HIGH);
    context.stream.println(F("ON"));
  }
  else if (strcasecmp(onoff, "OFF") == 0)
  {
    digitalWrite(pin, LOW);
    context.stream.println(F("OFF"));
  }
  else
  {
    context.stream.println(F(ERROR " Invalid argument, use ON or OFF"));
  }
}

// A command to show a help message listing all commands.
void cmd_help(LazySerial::Context &context)
{
  LAZY_COMMAND(F("HELP"), F("Shows this help message"));
  lazy.cmd_help();
}

// A command to identify yourself, essential when you have lots of projects hooked up over USB!
void cmd_version(LazySerial::Context &context)
{
  LAZY_COMMAND(F("VERSION"), F("Prints firmware version"));
  context.stream.println(F(OK "VERSION " FIRMWARE_VERSION " COMPILED AT " __TIMESTAMP__));
}

// A command to show the configured pinout for reference.
void cmd_pinout(LazySerial::Context &context)
{
  LAZY_COMMAND(F("PINOUT"), F("Prints configured arduino pinout and I2C/RS485 addresses"));
  context.stream.println(F(OK "PINOUT " LAZY_KEYVAL(PIN_LED1) LAZY_KEYVAL(PIN_LED2) LAZY_KEYVAL(PIN_PWR_ROUGHING) LAZY_KEYVAL(PIN_PWR_PH) LAZY_KEYVAL(PIN_PWR_FLUIDPUMP) LAZY_KEYVAL(PIN_ANALOG_FLUIDPUMP_SPEED) "\n Addresses: " LAZY_KEYVAL(turboTC80.getAddress())));
}

// Command to set any pin high or low for testing purposes.
void cmd_gpio(LazySerial::Context &context)
{
  LAZY_COMMAND("GPIO", "<pin number> <ON|OFF> Set any Arduino pin high or low - TESTING ONLY, DO NOT USE WITHOUT KNOWING WHAT YOU'RE DOING.");
  uint8_t pin = 0;
  char *onoff;
  bool ok = context.parse_int(&pin);
  LAZY_RETURN_USAGE_UNLESS(ok);
  ok = context.parse_word(&onoff);
  LAZY_RETURN_USAGE_UNLESS(ok);

  pinMode(pin, OUTPUT);
  context.stream.print("OK GPIO ");
  context.stream.print(pin);
  if (strcasecmp(onoff, "ON") == 0)
  {
    digitalWrite(pin, HIGH);
    context.stream.print(" ON\n");
  }
  else
  {
    digitalWrite(pin, LOW);
    context.stream.print(" OFF\n");
  }
}

// --------------------------------------------------
// ------------- POWER ON/OFF commands --------------------
// --------------------------------------------------

// A command to turn the roughing pump power on or off.
void cmd_roughing_on_off(LazySerial::Context &context)
{
  LAZY_COMMAND(F("ROUGHING"), F("<ON|OFF> Turns the roughing pump power on or off"));
  char *onoff;
  bool ok = context.parse_word(&onoff);
  LAZY_RETURN_USAGE_UNLESS(ok);
  context.stream.print(F(OK "ROUGHING "));
  handle_onoff_command(PIN_PWR_ROUGHING, onoff, context);
}

// A command to turn the pH probe power on or off.
void cmd_ph_pwr(LazySerial::Context &context)
{
  LAZY_COMMAND(F("PH"), F("<ON|OFF> Turns the pH probe power on or off"));
  char *onoff;
  bool ok = context.parse_word(&onoff);
  LAZY_RETURN_USAGE_UNLESS(ok);
  context.stream.print(F(OK "PH "));
  handle_onoff_command(PIN_PWR_PH, onoff, context);
}

// --------------------------------------------------
// ------------- FLUID PUMP CONTROL commands --------------------
// --------------------------------------------------

void cmd_fluidpump_rate(LazySerial::Context &context)
{
  LAZY_COMMAND(F("FLUIDPUMP_RATE"), F("<0-100> [INTEGER] Sets the fluid pumping rate as a percentage of full speed. Send 0 to turn off pump power"));
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
  LAZY_COMMAND(F("TURBO_INIT"), F("Sets up the turbo pump for operation"));
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
  LAZY_COMMAND(F("TURBO"), F("<ON|OFF> Turn the turbo motor off or on to the previously set speed"));
  char *onoff;
  bool ok = context.parse_word(&onoff);
  LAZY_RETURN_USAGE_UNLESS(ok);
  if (strcasecmp(onoff, "ON") == 0)
  {
    turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::MotorPump, PfeifferVacProtocol::BooleanOld(true));
    turboTC80.receiveTelegram(true);
    // context.stream.println(F("ON"));
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
  LAZY_COMMAND(F("TURBO_SPEED"), F("Sets the turbo pump to run at a target speed as percent of max speed (100% is 90,000 RPM for the Pfeiffer TC80) - Send 0 to reset to pfeiffer default speed control mode"));
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
  LAZY_COMMAND(F("TURBO_LIMIT_PWR"), F("Sets the turbo pump power limit as a percentage of full power (Pfeiffer max draw is 5A)"));
  context.stream.println(F(OK "TURBO_LIMIT_PWR"));
  turboTC80.sendCommand(PfeifferVacProtocol::ReferenceValueInput::PwrSVal, PfeifferVacProtocol::UExpoNew(100));
  bool isValid; // TODO check this funciton
  turboTC80.receiveUInteger(true, isValid);
}

void cmd_turbo_cmd(LazySerial::Context &context)
{
  LAZY_COMMAND(F("TURBO_CMD"), F("<PARAMETER_NUM> <DATA> Send a command to the turbo pump"));

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
  LAZY_COMMAND(F("TURBO_QUERY"), F("<PARAMETER_NUM> Query a parameter from the turbo pump"));

  uint16_t param = 0;
  bool ok = context.parse_int_minmax(&param, (uint16_t)0, (uint16_t)999);
  LAZY_RETURN_USAGE_UNLESS(ok);

  // queries always have 'action' set to '0' and a query data of "?"
  turboTC80.sendQuery(param, true);
  turboTC80.receiveTelegram(true, 10000);
}

void cmd_turbo_raw(LazySerial::Context &context)
{
  LAZY_COMMAND(F("TURBO_RAW"), F("<COMMAND> Send raw askii to the turbo pump"));
  char *command;
  bool ok = context.parse_word(&command);
  LAZY_RETURN_USAGE_UNLESS(ok);
  turboTC80._sendTelegramRaw(command);
}

void cmd_full_startup(LazySerial::Context &context)
{
  LAZY_COMMAND(F("STARTUP"), F("Start up everything"));

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

LazySerial::CallbackFunction comms_commands[] = {
    cmd_help,
    cmd_version,
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
  COMMS.begin(COMMS_SPEED);
  DEBUG.begin(COMMS_SPEED);
  wdtPrintStatus(COMMS);
  DEBUG.println("MCU initializing");

  // Setup Pins
  pinMode(PIN_LED1, OUTPUT);           // LED 1: Indicator light
  pinMode(PIN_LED2, OUTPUT);           // LED 2: Indicator light
  pinMode(PIN_PWR_PH, OUTPUT);         // PICO_ON: Ph probe power
  pinMode(PIN_PWR_ROUGHING, OUTPUT);   // MVP_ON: Roughing vacuum pump power
  pinMode(51, OUTPUT);                 // MVP_ON: Roughing vacuum pump power
  digitalWrite(PIN_PWR_ROUGHING, LOW); // Turn off Roughing vacuum pump

  digitalWrite(PIN_PWR_FLUIDPUMP, LOW); //  Turn off fluid pump
  digitalWrite(PIN_PWR_PH, LOW);        //  Turn off Ph probe

  lazy.set_commands(comms_commands);
  // start turbo and turn it off to start (in case of reboot)
  turboTC80.begin();
  turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::MotorPump, PfeifferVacProtocol::BooleanOld(false));
  // turboTC80.receiveBooleanOld()

  fluidPump.init();
  fluidPump.setSpeed(0); // Ensure fluid pump is off

  // Turn on Roughing pump by default:
  digitalWrite(PIN_PWR_ROUGHING, HIGH);

  // LED1 on to indicate setup done
  digitalWrite(PIN_LED1, HIGH);

  // // initilize system
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

void loop()
{
  nonBlockDelay(4000); // Main loop

  bool responseIsValid = true;

  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::SetRotSpdRpm, false);
  uint32_t turboSetpoint = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::SetRotSpdRpm, responseIsValid, false);
  log_value_w_status("Turbo_SETPOINT_RPM", (float)turboSetpoint, 50000, 95000);

  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::ActualSpdRpm, false);
  uint32_t turboRpm = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::ActualSpdRpm, responseIsValid, false);
  log_value_w_status("Turbo_RPM", (float)turboRpm, 50000, 95000);

  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::DrvCurrent, false);
  float turboCurrent = turboTC80.receiveUReal(PfeifferVacProtocol::StatusRequest::DrvCurrent, responseIsValid, false);
  log_value_w_status("Turbo_Amps", turboCurrent, 0.2, 5.2);

  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::TempRotor, false);
  uint32_t turboRotorTemp = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::TempRotor, responseIsValid, false);
  log_value_w_status("Turbo_Rotor℃", turboRotorTemp, 0, 60);

  log_value_w_status("Fluidpump_Rate", fluidPump.getSpeed(), 20, 100);

  COMMS.println();
}
