
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
#include "modernCommands.h"

bool autostarted = false;           // Becomes true once autostart has happened, so we don't do it again
bool commsBufferOverflowed = false; // flag to mark if the comms serial buffer ever has overflowed (meaning we sent too much data too fast and some got lost)

// A non-blocking delay function that allows lazy serial and watchdog resets to keep working while paused in various places.
void nonBlockDelay(unsigned long ms)
{
  unsigned long start = micros();
  do
  {
    // Check if we have sent too much serial data too fast recently.
    if (COMMS.availableForWrite() == 0)
    {
      commsBufferOverflowed = true;
    }
    // Check if any serial command is incoming and pause longer to give the user more time to finish their command
    if (COMMS.available() != 0)
    {
      // If the initial request was for longer than 4 seconds, keep that remaining time; otherwise allow up to 4 seconds.
      unsigned long elapsedMs = (micros() - start) / 1000;
      unsigned long remainingMs = (ms > elapsedMs) ? (ms - elapsedMs) : 0;
      unsigned long extendedMs = max(remainingMs, (unsigned long)4000);
      start = micros(); // Reset the start time to now to give the user more time to finish their command and read the response, while still allowing lazy serial commands to be processed and WDT resets to happen
      ms = extendedMs;
    }
    lazy.loop();
    turboSerialRS485.task();
    resetWDT();
  } while (micros() - start < ms * 1000); // Non-blocking delay, handles rollover
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
  // wdtStart();
  wdtStop();
  resetWDT();
  COMMS.begin(COMMS_BAUDRATE);
  COMMS.println("| Wait for startup...");

  delay(5000); // 5s delay to allow serial connection to establish before we start sending data, which can cause the first messages to be lost

  // Print startup messages
  COMMS.println("| ISMS Initializing... [ Firmware Version " FIRMWARE_VERSION " ]");
  wdtPrintStatus(COMMS); // Print the startup status from the watchdog to the COMMS serial for reference

  // Setup commands, logging and load saved settings
  lazy.set_commands(all_comms_commands);
  lazy.set_help_callback(lazy_help_callback);
  Log.begin(LOG_LEVEL_INFO, &COMMS, false);
  load_settings();
  Log.setLevel(constrain(saved_settings.log_level, LOG_LEVEL_INFO, LOG_LEVEL_VERBOSE)); // Log levels below info are NOT used, because they may hide expected log messages.

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
  //   COMMS.println("Autostarting system now.");
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
  //   COMMS.println("System autostart complete.");
  // }

  // turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::DrvCurrent, true);
}

void log_stats()
{

  String warnings = "";
  bool responseIsValid = true;

  Log.trace("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::SetRotSpdRpm, false);
  long turboRpmTarget = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::SetRotSpdRpm, responseIsValid, false);
  Log.info("Turbo_RPM_Target:%l", turboRpmTarget);
  if (!responseIsValid)
    warnings += "Turbo_RPM_Target wrong or missing response, ";
  log_value_postfix((float)turboRpmTarget, 50000, 95000, responseIsValid);
  nonBlockDelay(0); // Yield to allow command checks to run and reset WDT, while ensuring stats logging doesn't cause long pauses

  responseIsValid = true;
  Log.trace("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::ActualSpdRpm, false);
  long turboRpm = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::ActualSpdRpm, responseIsValid, false);
  Log.info("Turbo_RPM:%l", turboRpm);
  if (!responseIsValid)
    warnings += "Turbo_RPM wrong or missing response, ";
  log_value_postfix((float)turboRpm, 50000, 95000, responseIsValid);

  responseIsValid = true;
  Log.trace("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::DrvCurrent, false);
  float turboCurrent = turboTC80.receiveUReal(PfeifferVacProtocol::StatusRequest::DrvCurrent, responseIsValid, false);
  Log.info("Turbo_Current:%FA", turboCurrent);
  if (!responseIsValid)
    warnings += "Turbo_Current wrong or missing response, ";
  log_value_postfix(turboCurrent, 0.2, 5.2, responseIsValid);

  responseIsValid = true;
  Log.trace("\n");
  turboTC80.sendQuery(PfeifferVacProtocol::StatusRequest::TempRotor, false);
  long turboRotorTemp = turboTC80.receiveUInteger(PfeifferVacProtocol::StatusRequest::TempRotor, responseIsValid, false);
  Log.info("Turbo_Rotor_Temp:%lC", turboRotorTemp);
  if (!responseIsValid)
    warnings += "Turbo_Rotor_Temp wrong or missing response, ";
  log_value_postfix((float)turboRotorTemp, 0, 60, responseIsValid);

  Log.info("Fluidpump_Rate:%d%%", fluidPump.getSpeed());
  log_value_postfix((float)fluidPump.getSpeed(), 20, 100, true);

  responseIsValid = true;
  String lastestError = turboTC80.queryLatestError(responseIsValid, false, 5000);
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

  // if (commsBufferOverflowed)
  // {
  //   // TODO why is this showing up in the logs?
  //   warnings += "Comms serial overflowed. ";
  //   commsBufferOverflowed = false; // reset the flag after logging the warning
  // }

  while (COMMS.getWriteError())
  {
    // Wait for space in the serial buffer to log the warning, while still allowing lazy serial commands to be processed and WDT resets to happen
    nonBlockDelay(100);
  }
  commsBufferOverflowed = false;

  // check free memory and log a warning if it's low.
  unsigned long freeMemory = getAvailableRAM();
  if (freeMemory < 500)
  {
    warnings += "Low Memory: " + String(freeMemory) + "/" + String(getTotalRam()) + " bytes remaining, ";
  }

  // Log warnings if there were any
  Log.warningln("");
  if (warnings.length() > 0)
  {
    Log.warningln("!WARN: %s", warnings.c_str());
  }
}

unsigned long lastStatsLogTime = 0;
void loop()
{
  // always wait at least 1 ms between stats logs to allow lazy serial commands to be processed and WDT resets to happen, while ensuring stats logging doesn't cause long pauses
  nonBlockDelay(1); // Wait 1 ms between checks to see if it's time to log stats again, while still allowing lazy serial commands to be processed and WDT resets to happen

  // Check if it's time to log stats again, based on the user configured interval in saved settings. If not, return early.
  if (millis() - lastStatsLogTime < saved_settings.stats_log_interval)
    return;

  // Log stats and update the last log time
  lastStatsLogTime = millis();
  if (saved_settings.stats_loging_enabled)
  {
    log_stats();
  }
}
