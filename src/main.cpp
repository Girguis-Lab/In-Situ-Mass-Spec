
/* ISMS code for the ATMEL2560 based ISMS controller
  This is for the V3 redesign of the ISMSs with dedicated PCB (Year 2025/26)
  To compile this project, use Visual Studio Code IDE and install the PlatformIO Extension
  Use PlatformIO buttons in the bottom toolbar to compile, and upload. Uploads must be done with
  the USB b port on the circuit board, however serial data comes from the RS232 output
  (The serial of the USB B port is joined to the TURBO Pump RS485 Serial)
*/

#include "includes.h"
#include "modernCommands.h"

bool autostarted = false; // Becomes true once autostart has happened, so we don't do it again
unsigned long minFreeRam = getTotalRam();

void taskTick()
{
  resetWDT();
  turboSerialRS485.task();
  unsigned long freeRam = getAvailableRAM();
  if (freeRam < minFreeRam)
    minFreeRam = freeRam;
}

// A non-blocking delay function that allows lazy serial and watchdog resets to keep working while paused in various places.
// returns true if a lazy serial command was executed.
bool nonBlockDelay(unsigned long ms)
{
  unsigned long start = micros();
  bool commandRecieved = false;
  bool extendTimeout = false;
  bool skipTimeoutExtensionForUi = false;
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
  }

  // pause longer to give the user more time to finish typing their command
  if (extendTimeout && !skipTimeoutExtensionForUi && !commandRecieved)
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

void log_value_postfix(float value, float minOk, float maxOk, bool gotValidResponse)
{
  if (!gotValidResponse || value <= minOk || value >= maxOk)
  {
    Log.info("(!)");
  }
  Log.info(",");
}

void setup()
{
  // LED1 on to indicate setup done
  LED1_PWR.turnOn();
  // wdtStart();
  wdtStop();
  resetWDT();
  COMMS.begin(COMMS_BAUDRATE);
  COMMS.println("| Wait for startup...");

  delay(5000); // 5s delay to allow serial connection to establish before we start sending data, which can cause the first messages to be lost

  // Print startup messages
  COMMS.println("| ISMS Initializing... [ Firmware Version " FIRMWARE_VERSION " ]");
  wdtPrintStatus(COMMS); // Print the startup status from the watchdog to the COMMS serial for reference
  COMMS.print("| Free RAM baseline: ");
  COMMS.println(getAvailableRAM());

  // Setup commands, logging and load saved settings
  lazy.set_commands(all_comms_commands);
  lazy.set_help_callback(lazy_help_callback);
  Log.begin(DEBUG_LOG_LEVEL_OFF, &COMMS, false);
  load_settings();
  Log.setLevel(constrain(saved_settings.log_level, DEBUG_LOG_LEVEL_OFF, DEBUG_LOG_LEVEL_HIGH)); // Log levels below info are NOT used, because they may hide expected log messages.

  // Setup Pins and ensure everything is off to start
  LED1_PWR.begin(LOW);     // LED 1: Status Indicator light
  LED2_PWR.begin(LOW);     // LED 2: Warning/error Indicator light
  PH_PWR.begin(LOW);       // Ph probe power pin
  ROUGHING_PWR.begin(LOW); // Roughing vacuum pump power pin

  // Begin turbo pump serial interface and turn the pump off to start (in case of reboot)
  turboTC80.begin();
  turboTC80.sendCommand(PfeifferVacProtocol::ControlCommand::MotorPump, PfeifferVacProtocol::BooleanOld(false));

  // Initialize fluid pump control and ensure it's off to start
  fluidPump.init();
  fluidPump.setSpeed(0); // Ensure fluid pump is off

  // Turn off LED1 on to indicate setup done
  LED1_PWR.turnOff();

  // initilize system
  nonBlockDelay(saved_settings.autostart_delay);
  if (saved_settings.autostart_on && !autostarted && !beatActive)
  {
    autostarted = true;
    COMMS.println("Autostarting system now.");
    // Run the startup command
    LazySerial::Context ctx(LazySerial::CallingMode::INVOKE, COMMS);
    cmd_full_startup(ctx);
    COMMS.println("System autostart complete.");
  }

  // Warn operator if stats logging is disabled.
  if (!saved_settings.stats_loging_enabled)
  {
    Log.warningln("!WARN: Stats Logging disabled, send STATS_LOGGING <ms> to set stats logging interval.");
  }
}

void log_stats()
{

  String warnings = "";
  bool responseIsValid = true;

  Log.info("Fluidpump_Rate:%d%%", fluidPump.getSpeed());
  log_value_postfix((float)fluidPump.getSpeed(), 20, 100, true);
  if (nonBlockDelay(20))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  Log.trace("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::SetRotSpdRpm, false);
  long turboRpmTarget = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::SetRotSpdRpm, responseIsValid, false);
  if (!responseIsValid)
    warnings += "Turbo_RPM_Setpoint wrong or missing response, ";
  Log.info("Turbo_RPM_Setpoint:%l", turboRpmTarget);
  log_value_postfix((float)turboRpmTarget, 50000, 95000, responseIsValid);
  if (nonBlockDelay(20))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  responseIsValid = true;
  Log.trace("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::ActualSpdRpm, false);
  long turboRpm = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::ActualSpdRpm, responseIsValid, false);
  if (!responseIsValid)
    warnings += "Turbo_RPM wrong or missing response, ";
  else if (turboRpm > 1 && turboRpm < TC80_TURBO_LOW_SPEED_WARNING_RPM)
    warnings += "Low Turbopump RPM, ensure RGA filament is OFF to avoid burnout!";
  Log.info("Turbo_RPM:%l", turboRpm);
  log_value_postfix((float)turboRpm, 50000, 95000, responseIsValid);
  if (nonBlockDelay(20))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  responseIsValid = true;
  Log.trace("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::DrvCurrent, false);
  float turboCurrent = turboTC80.receiveUReal(PfeifferVacProtocol::StatusRequest::DrvCurrent, responseIsValid, false);
  if (!responseIsValid)
    warnings += "Turbo_Current wrong or missing response, ";
  Log.info("Turbo_Current:%FA", turboCurrent);
  log_value_postfix(turboCurrent, 0.2, 5.2, responseIsValid);
  if (nonBlockDelay(20))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  responseIsValid = true;
  Log.trace("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::TempRotor, false);
  long turboRotorTemp = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::TempRotor, responseIsValid, false);
  if (!responseIsValid)
    warnings += "Turbo_Rotor_Temp wrong or missing response, ";
  Log.info("Turbo_Rotor_Temp:%lC", turboRotorTemp);
  log_value_postfix((float)turboRotorTemp, 0, 60, responseIsValid);
  if (nonBlockDelay(20))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  responseIsValid = true;
  Log.trace("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::TempElec, false);
  long turboElecTemp = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::TempElec, responseIsValid, false);
  if (!responseIsValid)
    warnings += "Turbo_Elec_Temp wrong or missing response, ";
  Log.info("Turbo_Elec_Temp:%lC", turboElecTemp);
  log_value_postfix((float)turboElecTemp, 0, 70, responseIsValid);
  if (nonBlockDelay(20))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  responseIsValid = true;
  Log.trace("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::TempPmpBot, false);
  delay(1); // delay to avoid backtalk
  long turboBottomTemp = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::TempPmpBot, responseIsValid, false);
  if (!responseIsValid)
    warnings += "Turbo_Bottom_Temp wrong or missing response, ";
  Log.info("Turbo_Bottom_Temp:%lC", turboBottomTemp);
  log_value_postfix((float)turboBottomTemp, 0, 60, responseIsValid);
  if (nonBlockDelay(20))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  Log.info("Roughing_on:%d,", ROUGHING_PWR.getState());
  Log.info("Fluidpump_on:%d,", FLUIDPUMP_PWR.getState());
  Log.info("pH_on:%d,", PH_PWR.getState());
  if (nonBlockDelay(20))
    return; // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

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

  // Log warnings if there were any after a newline
  Log.warningln("");
  if (warnings.length() > 0)
  {
    Log.warningln("!WARN: %s", warnings.c_str());
    LED2_PWR.turnOn(); // turn on status LED 2 when warnings occur
  }
  else
  {
    LED2_PWR.turnOff(); // turn off status LED 2 when no warnings are active
  }
}

unsigned long lastStatsLogTime = 0;
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
