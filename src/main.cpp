// Entry point and main loop for the ATMEL2560 based ISMS controller.
//
// This is the V3 redesign of the ISMSs with dedicated PCB (Year 2025/26).
//
// Brings the instrument up in setup() -- watchdog, operator serial port, saved
// settings, switched power rails, turbo pump, fluid pump, and the optional
// automatic startup sequence -- then spends loop() broadcasting the periodic
// stats telegram that operators and the topside software read. Also provides
// nonBlockDelay(), the cooperative delay that every waiting routine in this
// firmware uses so that serial commands keep being answered and the watchdog
// keeps being petted no matter how long an operation takes.
//
// To compile this project, use Visual Studio Code IDE and install the
// PlatformIO Extension. Use PlatformIO buttons in the bottom toolbar to
// compile, and upload. Uploads must be done with the USB b port on the circuit
// board, however serial data comes from the RS232 output (the serial of the
// USB B port is joined to the TURBO Pump RS485 Serial).

#include "includes.h"
#include "modernCommands.h"

bool autostarted = false; // Becomes true once autostart has happened, so we don't do it again
// Lowest free RAM seen since boot, so the stats telegram can warn about a slow
// leak or a deep call path even after the memory has been reclaimed.
unsigned long minFreeRam = getTotalRam();

// Services the background work that must keep happening no matter what else is
// running.
//
// Pets the watchdog, lets the turbo pump's RS485 driver turn the transceiver
// around and drain its buffers, and tracks the low-water mark of free RAM.
// Called from the innermost loop of nonBlockDelay(), so it runs every few
// microseconds while any part of the firmware is waiting.
void taskTick()
{
  resetWDT();
  turboSerialRS485.task();
  unsigned long freeRam = getAvailableRAM();
  if (freeRam < minFreeRam)
    minFreeRam = freeRam;
}

// Waits roughly `ms` milliseconds without stalling the firmware's background
// work, and reports whether an operator command arrived while waiting.
//
// Spins on micros() -- which handles counter rollover -- calling taskTick()
// and polling the command line for the whole interval, so the watchdog is
// still petted and commands are still answered inside even a 60 second wait.
// If characters are still sitting in the receive buffer when the interval
// ends, the wait is extended by 3 more seconds to give a human time to finish
// typing; that courtesy is skipped for CR-terminated input, which identifies
// the topside GUI rather than a person at a keyboard.
//
// Returns true if a command was executed during the wait. Callers use this to
// abandon a long sequence the operator has just overridden.
bool nonBlockDelay(unsigned long ms)
{
  unsigned long start = micros();
  bool commandRecieved = false;
  bool extendTimeout = false;
  bool skipTimeoutExtensionForGui = false;
  // Non-blocking delay, handles rollover
  while (micros() - start < ms * 1000)
  {
    taskTick();
    // Check if any serial command is incoming and pause longer to give the user more time to finish typing their command
    if (COMMS.available() != 0)
    {
      extendTimeout = true;
    }
    commandRecieved = commandRecieved || lazy.loop();
    if (lazy.line_end == '\r')
      skipTimeoutExtensionForGui = true;
  }

  // pause longer to give the user more time to finish typing their command
  if (extendTimeout && !skipTimeoutExtensionForGui)
  {
    start = micros();
    while (micros() - start < 3000000)
    {
      taskTick();
      commandRecieved = commandRecieved || lazy.loop();
    }
  }

  // return if a bool indicating if a user command was processed.
  return commandRecieved;
}

// Prints the separator that follows one value in the stats telegram, flagging
// the value first if it looks wrong.
//
// When INCLUDE_OUT_OF_NORMAL_RANGE_MARKS is defined, a "(!)" marker is emitted
// before the comma if `gotValidResponse` is false or if `value` falls outside
// the exclusive range (`minOk`, `maxOk`), which makes a bad reading easy to
// pick out of a long log. The trailing comma is always printed, so the
// telegram's field layout does not change when the marks are compiled out.
void log_value_postfix(float value, float minOk, float maxOk, bool gotValidResponse)
{
#ifdef INCLUDE_OUT_OF_NORMAL_RANGE_MARKS
  if (!gotValidResponse || value <= minOk || value >= maxOk)
  {
    COMMS.print("(!)");
  }
#endif
  COMMS.print(",");
}

// Initializes the instrument once, at power-on or after a reset.
//
// Starts the watchdog, brings up the operator serial port and waits 5 seconds
// so the first messages are not lost to a link that is still settling, reports
// why the board last reset, registers the serial commands, then loads the
// saved settings and applies the saved log level. Every switched rail and both
// pumps are explicitly driven off, so a reboot mid-deployment does not leave
// the instrument in a half-running state. Finally, runs the full startup
// sequence if autostart is enabled and BEAT has not already cancelled it, and
// warns the operator if the periodic stats telegram is switched off.
void setup()
{
  // LED1 on to indicate setup done
  LED1_PWR.turnOn();
  wdtStart();
  // wdtStop(); // uncomment to disable wdt
  resetWDT();
  COMMS.begin(COMMS_BAUDRATE);
  COMMS.println("| Wait for startup...");

  delay(5000); // 5s delay to allow serial connection to establish before we start sending data, which can cause the first messages to be lost
  resetWDT();

  // Print startup messages
  COMMS.println("| ISMS Initializing... [ Firmware Version " FIRMWARE_VERSION " ]");
  wdtPrintStatus(COMMS); // Print the startup status from the watchdog to the COMMS serial for reference
  COMMS.print("| Free RAM baseline: ");
  COMMS.println(getAvailableRAM());

  // Setup commands, logging and load saved settings
  // LazySerial::CallbackFunction all_comms_commands[COUNT_OF(basic_comms_commands) + COUNT_OF(debug_comms_commands)];
  // concatArrays(basic_comms_commands, debug_comms_commands, all_comms_commands); // join the basic and debug commands into an array with all commands for lazy serial
  lazy.set_commands(all_comms_commands);
  lazy.set_help_callback(lazy_help_callback);
  LOG_ATTACH_SERIAL(COMMS);
  LOG_SET_DELIMITER("");
  LOG_SET_LEVEL(DEBUG_LOG_LEVEL_OFF);
  load_settings();
  LOG_SET_LEVEL((DebugLogLevel)constrain((uint8_t)saved_settings.log_level, (uint8_t)DEBUG_LOG_LEVEL_OFF, (uint8_t)DEBUG_LOG_LEVEL_HIGH)); // Log levels below INFO/DEBUG_OFF are NOT used, because they may hide expected log messages.

  // Setup Pins and ensure everything is off to start
  LED1_PWR.begin(LOW);      // LED 1: Status Indicator light
  LED2_PWR.begin(LOW);      // LED 2: Warning/error Indicator light
  ACCESSORY_PWR.begin(LOW); // Accessory/Ph probe power pin
  ROUGHING_PWR.begin(LOW);  // Roughing vacuum pump power pin

  // Begin turbo pump serial interface and turn the pump off to start (in case of reboot)
  turboTC80.begin();
  turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::MotorPump, PfeifferVacProtocol::BooleanOld(false));

  // Initialize fluid pump control and ensure it's off to start
  fluidPump.init();
  fluidPump.setSpeed(0); // Ensure fluid pump is off

  // Turn off LED1 on to indicate setup done
  LED1_PWR.turnOff();

  // auto initilize system
  if (saved_settings.autostart_on && !autostarted && !beatActive)
  {
    autostarted = true;
    nonBlockDelay(saved_settings.autostart_delay);
    // Run the startup command as if it was sent from the console
    LazySerial::Context ctx(LazySerial::CallingMode::MATCHED, COMMS);
    cmd_full_startup(ctx);
  }

  // Warn operator if stats logging is disabled.
  if (!saved_settings.stats_loging_enabled)
  {
    LOG_WARN(F("!WARN: Stats Logging disabled, send STATS_LOGGING <ms> to set stats logging interval.\n"));
  }
}

// Broadcasts one stats telegram on the operator serial port and updates the
// warning LED.
//
// Prints the fluid pump rate, then queries the turbo pump for its speed
// setpoint, actual speed, drive current, and rotor, electronics and pump
// bottom temperatures, then the state of the switched power rails -- each
// value followed by log_value_postfix(), so readings outside their normal band
// stand out. Along the way it accumulates plain-language warnings for missing
// or malformed pump replies, readings that indicate trouble (a low turbo RPM
// risks burning out the RGA filament), any active turbo pump error and its
// history, and low free RAM. Those warnings are logged after the values, and
// LED2 is lit for as long as any of them is active.
//
// Yields through nonBlockDelay() between queries so commands are still
// answered and the watchdog still petted while the telegram is assembled, and
// returns early -- cutting the telegram short -- if one of those yields
// executed a command, on the assumption that the operator's request matters
// more than a complete line of stats.
void log_stats()
{

  String warnings = "";
  bool responseIsValid = true;

  COMMS.print("Fluidpump_Rate:");
  COMMS.print(fluidPump.getSpeed());
  COMMS.print("%");
  log_value_postfix((float)fluidPump.getSpeed(), 20, 100, true);
  if (FLUIDPUMP_PWR.getState() == 1)
  {
    if (fluidPump.getSpeed() < 0)
    {
      warnings += F("Fluid pump running backwards, use FLUIDPUMP_RATE 100 to go full speed forwards, ");
    }
    if (abs(fluidPump.getSpeed()) < 20)
    {
      warnings += String(F("Fluid pump set to a low speed (")) + String(fluidPump.getSpeed()) + F("%) set FLUIDPUMP_RATE xxx higher for consistant flow, ");
    }
  }
  if (nonBlockDelay(5))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  LOG_DEBUG("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::SetRotSpdRpm, false);
  unsigned long turboRpmTarget = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::SetRotSpdRpm, responseIsValid, false);
  if (!responseIsValid)
    warnings += F("Turbo_RPM_Setpoint wrong or missing response, ");
  COMMS.print("Turbo_RPM_Setpoint:");
  COMMS.print(turboRpmTarget);
  log_value_postfix((float)turboRpmTarget, 50000, 95000, responseIsValid);
  if (nonBlockDelay(5))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  responseIsValid = true;
  LOG_DEBUG("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::ActualSpdRpm, false);
  unsigned long turboRpm = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::ActualSpdRpm, responseIsValid, false);
  if (!responseIsValid)
    warnings += F("Turbo_RPM wrong or missing response, ");
  else if (turboRpm > 1 && turboRpm < TC80_TURBO_LOW_SPEED_WARNING_RPM)
    warnings += F("Low Turbopump RPM - ensure RGA filament is OFF to avoid burnout, ");
  COMMS.print("Turbo_RPM:");
  COMMS.print(turboRpm);
  log_value_postfix((float)turboRpm, 50000, 95000, responseIsValid);
  if (nonBlockDelay(5))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  responseIsValid = true;
  LOG_DEBUG("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::DrvCurrent, false);
  float turboCurrent = turboTC80.receiveUReal(PfeifferVacProtocol::StatusRequest::DrvCurrent, responseIsValid, false);
  if (!responseIsValid)
    warnings += F("Turbo_Current wrong or missing response, ");
  COMMS.print("Turbo_Current:");
  COMMS.print(turboCurrent);
  COMMS.print("A");
  log_value_postfix(turboCurrent, 0.2, 5.2, responseIsValid);
  if (nonBlockDelay(5))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  responseIsValid = true;
  LOG_DEBUG("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::TempRotor, false);
  unsigned long turboRotorTemp = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::TempRotor, responseIsValid, false);
  if (!responseIsValid)
    warnings += F("Turbo_Rotor_Temp wrong or missing response, ");
  COMMS.print("Turbo_Rotor_Temp:");
  COMMS.print(turboRotorTemp);
  COMMS.print("C");
  log_value_postfix((float)turboRotorTemp, 0, 60, responseIsValid);
  if (nonBlockDelay(5))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  responseIsValid = true;
  LOG_DEBUG("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::TempElec, false);
  unsigned long turboElecTemp = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::TempElec, responseIsValid, false);
  if (!responseIsValid)
    warnings += F("Turbo_Elec_Temp wrong or missing response, ");
  COMMS.print("Turbo_Elec_Temp:");
  COMMS.print(turboElecTemp);
  COMMS.print("C");
  log_value_postfix((float)turboElecTemp, 0, 70, responseIsValid);
  if (nonBlockDelay(5))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  responseIsValid = true;
  LOG_DEBUG("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::TempPmpBot, false);

  unsigned long turboBottomTemp = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::TempPmpBot, responseIsValid, false);
  if (!responseIsValid)
    warnings += F("Turbo_Bottom_Temp wrong or missing response, ");
  COMMS.print("Turbo_Bottom_Temp:");
  COMMS.print(turboBottomTemp);
  COMMS.print("C");
  log_value_postfix((float)turboBottomTemp, 0, 60, responseIsValid);
  if (nonBlockDelay(5))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  responseIsValid = true;
  LOG_DEBUG("\n");

  turboTC80.sendQuery(PfeifferVacProtocol::ReferenceValueInput::PwrSVal, false);
  uint16_t turboPowerLimit = turboTC80.receiveUShortInt(PfeifferVacProtocol::ReferenceValueInput::PwrSVal, responseIsValid, false);
  if (turboPowerLimit < 100)
  {
    warnings += String(F("Turbo Power Limit set to ")) + String(turboPowerLimit) + F("%, ");
  }
  if (nonBlockDelay(5))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  COMMS.print("Roughing_on:");
  COMMS.print(ROUGHING_PWR.getState());
  COMMS.print(",Fluidpump_on:");
  COMMS.print(FLUIDPUMP_PWR.getState());
  COMMS.print("," ACCESSORY_NAME "_on:");
  COMMS.print(ACCESSORY_PWR.getState());
  if (nonBlockDelay(5))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  // Log newline after all stats have been logged
  PRINT("\n");

  responseIsValid = true;
  String lastestError = turboTC80.queryLatestError(responseIsValid, false, 5000);
  // if there is any active turbo pump error, also query the turbo pump error history:
  if (lastestError.length() != 0)
  {
    warnings += "Turbo Error: ," + lastestError + ", ";
    responseIsValid = true;
    String errorHistory = turboTC80.queryErrorHistory(responseIsValid, false, 5000);
    if (errorHistory.length() != 0)
    {
      warnings += "Turbo Error History: ," + errorHistory + ", ";
    }
  }

  // check free memory and log a warning if it's low.
  unsigned long freeMemory = getAvailableRAM();
  if (freeMemory < 1000 || minFreeRam < 600)
  {
    warnings += "Low Free Memory: " + String(freeMemory) + "/" + String(getTotalRam()) + " bytes remaining [low since boot: " + String(minFreeRam) + "], ";
  }

  if (warnings.length() > 0)
  {
    LOG_WARN(F("!WARN: "));
    LOG_WARN(warnings);
    LOG_WARN("\n");
    LED2_PWR.turnOn(); // turn on status LED 2 when warnings occur
  }
  else
  {
    LOG_WARN("!WARN: All OK\n");
    LED2_PWR.turnOff(); // turn off status LED 2 when no warnings are active
  }
}

unsigned long lastStatsLogTime = 0; // millis() when the last stats telegram was sent.

// Broadcasts the stats telegram on the interval held in the saved settings.
//
// Waits a millisecond through nonBlockDelay() on every pass, so serial
// commands are serviced and the watchdog is petted even when no telegram is
// due, then logs stats once the configured interval has elapsed. LED1 is lit
// for the duration of each logging event, giving a visible heartbeat; when
// stats logging is disabled it is flashed briefly instead so the board still
// looks alive.
void loop()
{
  // always wait at least 1 ms between stats logs to allow lazy serial commands to be processed and WDT resets to happen, while ensuring stats logging doesn't cause long pauses
  nonBlockDelay(1); // Wait 1 ms between checks to see if it's time to log stats again, while still allowing lazy serial commands to be processed and WDT resets to happen

  // Check if it's time to log stats again, based on the user configured interval in saved settings. If not, return early.
  if (millis() - lastStatsLogTime < saved_settings.stats_log_interval)
  {
    // Turn off LED1 on to indicate the ISMS is between logging events
    LED1_PWR.turnOff();
    // Return to avoid logging
    return;
  }

  // Log stats and update the last log timestamp
  lastStatsLogTime = millis();
  // Turn on LED1 on to indicate the ISMS has started a stats log event.
  LED1_PWR.turnOn();
  if (saved_settings.stats_loging_enabled)
  {
    log_stats();
  }
  else
  {
    nonBlockDelay(100); // delay to give the status LED time to visibly flash.
  }

  // Turn off LED1 after the ISMS has done one stats log
  LED1_PWR.turnOff();
}
