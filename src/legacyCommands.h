/*


All responses from the MCU start with ‘M’+’␠’ (ASCII 77 and 32) (this will be omitted from the response description in the following). The main module should always respond.
If no specific response is required, it will reply “OK”. If the command has not been understood, the MCU will respond with “EC”.
All communications to and from the MCU should end with only CR (ASCII 13).

Command | Description	| Response, if different from standard “OK”
“BEAT”	Checking that the module is alive	“BEAT”
Queries NOTE: the command will be accepted as long as the first letter is correct
“?FIRM”	Request the firmware version	E.g. “2.2”
“?DATA”	Request the data telegram, same as the one broadcasted automatically at regular interval	See the full description below
“?CALTIME”	Request the information on how long each calibration bag has been used	“int1,int2,int3” int being the time in minutes for the bags 1 to 3. Values are integers.
“?VOLT”	Request Voltage informations for V1-V5 (the five valves for Calibration)	Returns:
“Reading Voltages
 V1: XX.XX V
 V2: XX.XX V
V3: XX.XX V
V4: XX.XX V
V5: XX.XX V”
Where xx.xx is the voltage measured
“?EXTT”	Enumerate External temperature sensor addresses. Use this only if the external temperature sensor has been replaced
“?ID”	Request the LTDISMS serial number	Returns “LTDISMSxxx” where xxx is the number
“?L”	Request limits and other current status of variables	See limits table
“?S”	Request the status byte
“?A”	Print firmware diagnostics
Power controls
“PTMPON”	Turn the turbo pump power on
“PTMPOFF”	Turn the turbo pump power off
“PRGAON”	Turn the RGA mass spectrometer power on
“PRGAOFF”	Turn the RGA mass spectrometer power off
“PKNF1ON”	Turn the main roughing pump power on. Not recommended. Use PKFN0ON which takes the KNFmode into account.(see 3)

“PKNF1OFF”	Turn the main roughing pump power off
“PKNF2ON”	Turn the auxiliary roughing pump power on. Not recommended. Use PKFN0ON which takes the KNFmode into account.
“PKNF2OFF”	Turn the auxiliary roughing pump power off
“PFAN#ON”	Turn the fan number # (1 to 4) power on
# = 1, Fan on the power conditioning units
# = 2, Fan on the turbo pump
# = 3, Fan on the nafion drier
# = 4, Fan on the knf pumps
“PFAN#OFF”	Turn the fan number # (1 to 4) power off
“PAUX1ON”	Turn the roughing vacuum valve 1 on
“PAUX1OFF”	Turn the roughing vacuum valve 1 off
“PAUX2ON”	Turn the roughing vacuum valve 2 on
“PAUX2OFF”	Turn the roughing vacuum valve 2 off
“PPHON”	Turn the pH meter power on
“PPHOFF”	Turn the pH meter power off
“PSBEON”	Turn the external SBE CTD pump power on
“PSBEOFF”	Turn the external SBE CTD pump power off
“PHEATON”	Turn the heaters power on (doesn’t turn the heater on, but provides power to their control relays. This upstream control is a security in case one of the solid state relays controlling the individual heaters burns to a closed state)
“PHEATOFF”	Turn the heaters power off
“PKNF0ON”	Calls the startKNF function, which takes in account the KNFMode.	“OK” or “FAILED” if there was an error
“PKNF0OFF”	Calls the stopKNF function, which takes in account the KNFMode.
External sensors control
“EISO”	Isolate the external sensors lines. This is to prevent a ground fault in the case of an external sensor grounded to the seawater.
“EPH”	This command is not valid anymore, because the pH value is now always read and the isolation is electronic, not via a relay anymore
“ET”	Close the circuit to the external temperature micro-LAN to get digital read of the external temperature sensor
Settings commands
“SETRATE*** #######”	Set the rates for:
*** = ‘EXT’ reading the external sensors
*** = ‘INT’ reading the internal sensors
*** = ‘POW’ reading the status from the power control module
*** = ‘SOL’ reading the status from the sample routing control module
*** = ‘AUT’ performing the autocheck (temperature, etc)
*** = ‘DLR’ current rate for the automatic broadcast of the full data set.
####### the rate, in milliseconds. E.g. 0001000 for 1 second.
Note: there is a space between *** and #######
Note: rates are preset to the recommend values
“SETDBON”	Turn on the automatic data broadcasting function. By default it is on.
“SETDBOFF”	Turn off the automatic data broadcasting.
“SETPRWON”	Turn on the pressure watch (see below for details)
“SETPRWOFF”	Turn off the pressure watch
“SETPRLI####”	Set the pressure limit for the pressure watch to another value than the default.
#### the limit value in mV for the sensor, between 0 and 5000.
“SETRESETCOMS2”	Reset the communication line on the main module COM2, communication with the power module	“C2Reseted”
“SETRESETCOMS3”	Reset the communication line on the main module COM3, communication with the sampling routing module	“C3Reseted”
“SETRESETP”	Resets the power module microcontroller	"PowerBoard reseted"
“SETRESETS”	Resets the sampling routing microcontroller	"SolenoidBoard reseted"
“SETLOGXON”	Turns the “logging” on. This mean the control module will send over the serial link more information, like “reading external sensors”, etc. For debugging only.
X is either 1 or 2, to having logging level 1 or 2
“SETLOGOFF”	Turns the “logging 1 or 2” option off.
“SETFAN#AON”	Sets the control to fan # (1 to 4) to automatic. This is the default for all fans
“SETFAN#AOFF”	Sets the control to fan # (1 to 4) to manual.
“SETKNFMODE#”	Sets the roughing pumps in the # mode (1 to 3). See the chapter on sample routings and vacuum control for details.
Note: this setting is stored in the EEPROM, so it will be kept even if the power is cycled.	“KNFMode set to # “
“SETTWATCHON”	Sets the temperature watch on. This is the default setting. If Temperature in the housing exceed some default limits, all components are turned off except the fans.
“SETTWATCHOFF”	Sets the temperature watch off. This is not recommended.
“SETMINUTEXXXXX”	Sets the value in millis for one minute in the automatic sequences. This is used to reduce intervals when testing sequences	“Minute set to”
“xxxxx”
“SETREADEEPROM”	Reads the values in the EEPROM
“SETRESETEEPROM”	Sets the value in EEPROM to 1 for knfmode and to 0 for the calibration times. Use this command only after a firmware update
“SETTADD”	Allow to write a new address for the external temperature sensor. Use this if the external temp. sensor has been changed. Follow displayed instructions.
“SETPKxxx”	Set correction factor for secondary pressure sensor. Xxx is 000 to 256 value.
“SETRESETCE”	Reset a critical error status
“SETLXXTXXX”	Set limit #XX to the value of XXX (0-256)	Will ask for a confirmation
“SETLD”	Reset all limits to default
“SETWDTON”	Set watchdog timer on
“SETWDTOFF”	Set watchdog timer off
Valve and sample routing
“V#ON”	Turns solenoid # (1 to 4) in ‘1’ state
“V#OFF”	Turns solenoid # (1 to 4) in ‘0’ state
“V#ONL”	Turns solenoid # (1 to 4) in ‘1’ state using a long pulse
“V#OFL”	Turns solenoid # (1 to 4) in ‘0’ state using a long pulse
“VISOMS”	Isolates the MS from the sampling routing, see chapter on sample routing for details
“VISOIN”	Isolates the membrane inlet, see chapter on sample routing for details
“VDIR”	Sets the inlet directly inline with the MS, but the MS is still isolated, see chapter on sample routing for details
“VNAFI”	Sets the inlet with the MS through the nafion drier, but the MS is still isolated, see chapter on sample routing for details
“VNAFV”	Sets nafion drier line to the roughing vacuum
“VMSON”	Calls for opening the line to the MS. The system will wait for the pressure to be in acceptable range before doing so.	Return “OP” if the pressure is too high. Eventually when the MS line is open will send “MS on”
“VMSONF”	Forces the opening of the line to the MS even if the pressure is too high.	This may result in receiving an EP1 error for the pressure being too high.
Heater commands
“HV#ON”	Initiates regeneration heating for the nafion drier
#=’1’ use heater1
#=’2’ use heater2
#=’3’ use both heater 1 and 2
Note: the heaters will stop after a set time
Note: this command shouldn’t be used, but rather use the higher level command for nafion regeneration.
“HV#OFF”	Stops regeneration heating for the nafion drier
# same as above
“HI#ON”	Initiates regeneration heating for the ionization chamber
#=’1’ use heater1
#=’2’ use heater2
#=’3’ use both heater 1 and 2
Note: the heaters will not stop until a corresponding command has been sent.
Note: this command shouldn’t be used, but rather use the higher level command for the chamber cleaning.
“HI#OFF”	Stops regeneration heating for the ionizations chamber
# same as above
Calibrations
“CV0ON”	Closes the calibration solenoid ground relay.
“CV0OFF”	Opens the calibration solenoid ground relay
“CV#ON”	Closes the calibration solenoid # relay.
“CV#OFF”	Opens the calibration solenoid # relay.
“CFLUID”	Sets the sampling path to fluid sampling
“CBGND”	Sets the sampling path to background sampling
“CAL#”	Sets the sampling path to bag #
Automatic Sequences. See chapter 6 for details.

“ASTART1”	Initiates the start1 sequence	“OK” and then “START1 finished” when completed
“ASTART2”	Initiates the start2 sequence	“OK” and then “START2 finished” when completed
“ASTANDBY”	Puts the instrument in the predefined standby mode. This also set the data broadcasting interval to 1 minute	“OK”, then “STANDBY mode activated”
“APOWEROFF”	Turns the instrument in the minimum power state	“OK”, then “POWEROFF mode activated”
“ACAL#”	Initiates calibration with bag #.
#= ‘1’ to ‘2’, calibrate for 10 minutes with bag 1 to 2.
#= ‘3’ use bag 3 for 1 min (antifouling)
#= ‘4’, calibrate for 10 minutes with background
#=’0’, stop the current calibration	“OK”, then “CAL# end” when finished, or “FAILED” +error, if failed, or “CAL aborted” if ACAL0 is called
“ACAL9”	Initiates a full calibration series	“OK”, then “CAL#direct” or “CAL#nafion” each time another calibration fluid /sample routing is used, and “CAL end” when finished.
“ANAFREG#”	Initiates a nafion drier regeneration cycle using the heater #
#=’1’ or ‘2’, uses the heater 1 or 2
#= ‘3’uses both heater
    “OK”, and “Nafion Reg finished” when finished, or “Nafion Reg Failed” if the correct temperature has not been reached
“AIONREG#”	Initiates an ion chamber regeneration cycle using the heater #
#=’1’ or ‘2’, uses the heater 1 or 2
#= ‘3’uses both heater
    “OK”, and “Ion Reg finished” when finished, or “Ion Reg Failed” if the correct temperature has not been reached
“ASAMPLE”	Initiates a predefined sampling cycle, with the default length value (10 min)	“OK”, then
“Sampling time set to XX”,
Then
“NAF” for the nafion dried sampling, then “DIR” for direct sampling, then “Sampling Finished” when the cycle ends.
“ASAMPLEXX”	Initiates the same cycle as ASAMPLE but with sampling times defined as XX minutes.
If xx=00, then uses the default time. Responses are the same as above.
“ASPLFNXX”	Initiates a sampling of the fluids in nafion mode, for XX minutes. If XX=00, then the sampling time is set to 32767 minutes (about 22days. This is a variable storage limit).	“OK”, then
“Sampling time set to XX”, then “NAF”, then
“fluid”,
Then “Sampling Finished” when done.
“ASPLFDXX”	Same as above, but sampling fluids in direct mode	“OK”, then
“Sampling time set to XX”, then “Dir”, then “fluid”
Then “Sampling Finished” when done.
“ASPLBNXX”	Same as above, but sampling background water in nafion mode	“OK”, then
“Sampling time set to XX”, then “NAF”, then
“background”,
Then “Sampling Finished” when done.
“ASPLBDXX”	Same as above, but sampling background water in direct mode	“OK”, then
“Sampling time set to XX”, then “Dir”, then “background”
Then “Sampling Finished” when done.
“ASPLSTP”	Specific code to stop a ASPL sampling.
“AABORT”	Aborts current sequence. Not recommended, leave the instrument in unknown state	“ABORTED”

*/
#include "includes.h"

static void print_line_end(Stream &stream)
{
    stream.print('13'); // CR
}

static void print_ok(Stream &stream)
{
    stream.print("M OK"); // CR
    print_line_end(stream);
}

static void print_ec(Stream &stream)
{
    stream.print(F("M EC"));
    print_line_end(stream);
}

// Mock command implementations. Each command sends a reasonable mock response.
// If a parameterized command is missing its parameter this will reply "M EC".

void cmd_beat(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U BEAT"), F("Checking that the module is alive - responds with BEAT"));
    context.stream.print(F("M BEAT"));
    print_line_end(context.stream);
}

void cmd_firm(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ?FIRM"), F("Request the firmware version - responds with version string"));
    context.stream.print("M " FIRMWARE_VERSION);
    print_line_end(context.stream);
}

void cmd_data(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ?DATA"), F("Request the data telegram"));
    // Example data telegram - replace with actual data retrieval
    context.stream.print("M DATA ");
    context.stream.print("Pressure:1013.25,");
    context.stream.print("Temperature:25.0,");
    context.stream.print("Humidity:40.0");
    print_line_end(context.stream);
}

static String request_string(const LazySerial::Context &context)
{
// Try common field names used by lazy serial contexts; fall back to empty string.
#if defined(__AVR__)
// on embedded builds context.request is commonly a String
#endif
    if constexpr (requires { context.request; })
    {
        return context.request;
    }
    if constexpr (requires { context.raw; })
    {
        return context.raw;
    }
    return String();
}

// Power controls
void cmd_ptmpon(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PTMPON"), F("Turn turbo pump on"));
    print_ok(context.stream);
}
void cmd_ptmpoff(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PTMPOFF"), F("Turn turbo pump off"));
    print_ok(context.stream);
}
void cmd_prgaon(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PRGAON"), F("Turn RGA on"));
    print_ok(context.stream);
}
void cmd_prgaoff(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PRGAOFF"), F("Turn RGA off"));
    print_ok(context.stream);
}
void cmd_pknf1on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PKNF1ON"), F("Main roughing pump on"));
    print_ok(context.stream);
}
void cmd_pknf1off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PKNF1OFF"), F("Main roughing pump off"));
    print_ok(context.stream);
}
void cmd_pknf2on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PKNF2ON"), F("Aux roughing pump on"));
    print_ok(context.stream);
}
void cmd_pknf2off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PKNF2OFF"), F("Aux roughing pump off"));
    print_ok(context.stream);
}

// Fans (1..4) ON/OFF
void cmd_pfan1on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PFAN1ON"), F("Fan1 on"));
    print_ok(context.stream);
}
void cmd_pfan1off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PFAN1OFF"), F("Fan1 off"));
    print_ok(context.stream);
}
void cmd_pfan2on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PFAN2ON"), F("Fan2 on"));
    print_ok(context.stream);
}
void cmd_pfan2off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PFAN2OFF"), F("Fan2 off"));
    print_ok(context.stream);
}
void cmd_pfan3on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PFAN3ON"), F("Fan3 on"));
    print_ok(context.stream);
}
void cmd_pfan3off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PFAN3OFF"), F("Fan3 off"));
    print_ok(context.stream);
}
void cmd_pfan4on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PFAN4ON"), F("Fan4 on"));
    print_ok(context.stream);
}
void cmd_pfan4off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PFAN4OFF"), F("Fan4 off"));
    print_ok(context.stream);
}

// Auxiliary valves and devices
void cmd_paux1on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PAUX1ON"), F("Aux vac valve1 on"));
    print_ok(context.stream);
}
void cmd_paux1off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PAUX1OFF"), F("Aux vac valve1 off"));
    print_ok(context.stream);
}
void cmd_paux2on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PAUX2ON"), F("Aux vac valve2 on"));
    print_ok(context.stream);
}
void cmd_paux2off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PAUX2OFF"), F("Aux vac valve2 off"));
    print_ok(context.stream);
}
void cmd_pphon(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PPHON"), F("pH meter on"));
    print_ok(context.stream);
}
void cmd_pphoff(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PPHOFF"), F("pH meter off"));
    print_ok(context.stream);
}
void cmd_psbeon(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PSBEON"), F("SBE CTD pump on"));
    print_ok(context.stream);
}
void cmd_psbeoff(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PSBEOFF"), F("SBE CTD pump off"));
    print_ok(context.stream);
}
void cmd_pheaton(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PHEATON"), F("Heaters power on"));
    print_ok(context.stream);
}
void cmd_pheatoff(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PHEATOFF"), F("Heaters power off"));
    print_ok(context.stream);
}

// KNF start/stop high-level
void cmd_pknf0on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PKNF0ON"), F("Start KNF considering mode"));
    print_ok(context.stream);
}
void cmd_pknf0off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U PKNF0OFF"), F("Stop KNF considering mode"));
    print_ok(context.stream);
}

// External sensors control
void cmd_eiso(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U EISO"), F("Isolate external sensors"));
    print_ok(context.stream);
}
void cmd_eph(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U EPH"), F("EPH obsolete"));
    context.stream.print(F("M EPH EC"));
    print_line_end(context.stream);
} // not valid anymore
void cmd_et(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ET"), F("Enable external temperature bus"));
    print_ok(context.stream);
}

// Queries
void cmd_caltime(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ?CALTIME"), F("Calibration bag usage times"));
    context.stream.print(F("M 120,30,0"));
    print_line_end(context.stream);
}
void cmd_volt(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ?VOLT"), F("Voltage V1-V5"));
    context.stream.print(F("M Reading Voltages\n V1: 12.34 V\n V2: 5.00 V\n V3: 3.30 V\n V4: 0.00 V\n V5: 4.98 V"));
    print_line_end(context.stream);
}
void cmd_extt(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ?EXTT"), F("Enumerate external temp sensor addresses"));
    context.stream.print(F("M 28-FF-4C-92-60-1A-03-5A"));
    print_line_end(context.stream);
}
void cmd_id(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ?ID"), F("Get serial number"));
    context.stream.print(F("M LTDISMS123"));
    print_line_end(context.stream);
}
void cmd_l(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ?L"), F("Request limits/status"));
    context.stream.print(F("M Limits: Pmin=0,Pmax=5000;Tmin=-10,Tmax=60"));
    print_line_end(context.stream);
}
void cmd_s(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ?S"), F("Status byte"));
    context.stream.print(F("M 0x5A"));
    print_line_end(context.stream);
}
void cmd_a(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ?A"), F("Firmware diagnostics"));
    context.stream.print(F("M Diagnostics: All systems nominal"));
    print_line_end(context.stream);
}

// Settings commands
void cmd_setrate_ext(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETRATEEXT"), F("Set external rate"));
    String req = request_string(context);
    if (req.indexOf(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
}
void cmd_setrate_int(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETRATEINT"), F("Set internal rate"));
    String req = request_string(context);
    if (req.indexOf(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
}
void cmd_setrate_pow(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETRATEPOW"), F("Set power rate"));
    String req = request_string(context);
    if (req.indexOf(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
}
void cmd_setrate_sol(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETRATESOL"), F("Set solenoid rate"));
    String req = request_string(context);
    if (req.indexOf(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
}
void cmd_setrate_aut(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETRATEAUT"), F("Set autocheck rate"));
    String req = request_string(context);
    if (req.indexOf(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
}
void cmd_setrate_dlr(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETRATEDLR"), F("Set data broadcast rate"));
    String req = request_string(context);
    if (req.indexOf(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
}

void cmd_setdbon(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETDBON"), F("Turn data broadcast on"));
    print_ok(context.stream);
}
void cmd_setdboff(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETDBOFF"), F("Turn data broadcast off"));
    print_ok(context.stream);
}
void cmd_setprwon(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETPRWON"), F("Pressure watch on"));
    print_ok(context.stream);
}
void cmd_setprwoff(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETPRWOFF"), F("Pressure watch off"));
    print_ok(context.stream);
}
void cmd_setprli(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETPRLI"), F("Set pressure limit"));
    String req = request_string(context);
    if (req.indexOf(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
}

void cmd_setresetcoms2(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETRESETCOMS2"), F("Reset COM2"));
    context.stream.print(F("M C2Reseted"));
    print_line_end(context.stream);
}
void cmd_setresetcoms3(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETRESETCOMS3"), F("Reset COM3"));
    context.stream.print(F("M C3Reseted"));
    print_line_end(context.stream);
}
void cmd_setresetp(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETRESETP"), F("Reset power module"));
    context.stream.print(F("M PowerBoard reseted"));
    print_line_end(context.stream);
}
void cmd_setresets(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETRESETS"), F("Reset solenoid board"));
    context.stream.print(F("M SolenoidBoard reseted"));
    print_line_end(context.stream);
}

void cmd_setlog1on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETLOG1ON"), F("Logging level1 on"));
    print_ok(context.stream);
}
void cmd_setlog2on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETLOG2ON"), F("Logging level2 on"));
    print_ok(context.stream);
}
void cmd_setlogoff(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETLOGOFF"), F("Logging off"));
    print_ok(context.stream);
}

void cmd_setfan1aon(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETFAN1AON"), F("Fan1 auto"));
    print_ok(context.stream);
}
void cmd_setfan1aoff(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETFAN1AOFF"), F("Fan1 manual"));
    print_ok(context.stream);
}
// Fan 2..4
void cmd_setfan2aon(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETFAN2AON"), F("Fan2 auto"));
    print_ok(context.stream);
}
void cmd_setfan2aoff(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETFAN2AOFF"), F("Fan2 manual"));
    print_ok(context.stream);
}
void cmd_setfan3aon(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETFAN3AON"), F("Fan3 auto"));
    print_ok(context.stream);
}
void cmd_setfan3aoff(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETFAN3AOFF"), F("Fan3 manual"));
    print_ok(context.stream);
}
void cmd_setfan4aon(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETFAN4AON"), F("Fan4 auto"));
    print_ok(context.stream);
}
void cmd_setfan4aoff(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETFAN4AOFF"), F("Fan4 manual"));
    print_ok(context.stream);
}

void cmd_setknfmode(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETKNFMODE"), F("Set KNF mode"));
    String req = request_string(context);
    if (req.indexOf(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(F("M KNFMode set to 1"));
    print_line_end(context.stream);
}
void cmd_settwatchon(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETTWATCHON"), F("Temperature watch on"));
    print_ok(context.stream);
}
void cmd_settwatchoff(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETTWATCHOFF"), F("Temperature watch off"));
    print_ok(context.stream);
}
void cmd_setminute(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETMINUTEXXXXX"), F("Set minute millis"));
    String req = request_string(context);
    if (req.indexOf(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(F("M Minute set to 1000"));
    print_line_end(context.stream);
}

void cmd_setreadeeprom(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETREADEEPROM"), F("Read EEPROM"));
    context.stream.print(F("M EEPROM: knfmode=1,caltimes=0,0,0"));
    print_line_end(context.stream);
}
void cmd_setreseteeprom(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETRESETEEPROM"), F("Reset EEPROM defaults"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
}
void cmd_settadd(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETTADD"), F("Set temp sensor address"));
    context.stream.print(F("M Follow instructions to write new address"));
    print_line_end(context.stream);
}
void cmd_setpk(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETPK"), F("Set secondary pressure correction"));
    String req = request_string(context);
    if (req.indexOf(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
}
void cmd_setresetce(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETRESETCE"), F("Reset critical error status"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
}
void cmd_setl(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETLXXTXXX"), F("Set limit value"));
    String req = request_string(context);
    if (req.indexOf(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(F("M Will ask for confirmation"));
    print_line_end(context.stream);
}
void cmd_setld(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETLD"), F("Reset limits to default"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
}
void cmd_setwdton(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETWDTON"), F("Watchdog on"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
}
void cmd_setwdtoff(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U SETWDTOFF"), F("Watchdog off"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
}

// Valve and sampling routing (V1..V4)
void cmd_v1on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V1ON"), F("V1 to 1"));
    print_ok(context.stream);
}
void cmd_v1off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V1OFF"), F("V1 to 0"));
    print_ok(context.stream);
}
void cmd_v1onl(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V1ONL"), F("V1 long pulse to 1"));
    print_ok(context.stream);
}
void cmd_v1ofl(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V1OFL"), F("V1 long pulse to 0"));
    print_ok(context.stream);
}
// V2..V4 similar
void cmd_v2on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V2ON"), F("V2 to 1"));
    print_ok(context.stream);
}
void cmd_v2off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V2OFF"), F("V2 to 0"));
    print_ok(context.stream);
}
void cmd_v2onl(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V2ONL"), F("V2 long to 1"));
    print_ok(context.stream);
}
void cmd_v2ofl(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V2OFL"), F("V2 long to 0"));
    print_ok(context.stream);
}
void cmd_v3on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V3ON"), F("V3 on"));
    print_ok(context.stream);
}
void cmd_v3off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V3OFF"), F("V3 off"));
    print_ok(context.stream);
}
void cmd_v3onl(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V3ONL"), F("V3 long on"));
    print_ok(context.stream);
}
void cmd_v3ofl(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V3OFL"), F("V3 long off"));
    print_ok(context.stream);
}
void cmd_v4on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V4ON"), F("V4 on"));
    print_ok(context.stream);
}
void cmd_v4off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V4OFF"), F("V4 off"));
    print_ok(context.stream);
}
void cmd_v4onl(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V4ONL"), F("V4 long on"));
    print_ok(context.stream);
}
void cmd_v4ofl(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U V4OFL"), F("V4 long off"));
    print_ok(context.stream);
}

void cmd_visoms(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U VISOMS"), F("Isolate MS from routing"));
    print_ok(context.stream);
}
void cmd_visoin(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U VISOIN"), F("Isolate membrane inlet"));
    print_ok(context.stream);
}
void cmd_vdir(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U VDIR"), F("Set inlet inline with MS"));
    print_ok(context.stream);
}
void cmd_vnafi(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U VNAFI"), F("Set nafion dried inlet"));
    print_ok(context.stream);
}
void cmd_vnafv(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U VNAFV"), F("Set nafion to roughing vacuum"));
    print_ok(context.stream);
}
void cmd_vmson(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U VMSON"), F("Open line to MS, check pressure"));
    context.stream.print(F("M OP"));
    print_line_end(context.stream);
}
void cmd_vmsonf(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U VMSONF"), F("Force open line to MS"));
    context.stream.print(F("M MS on"));
    print_line_end(context.stream);
}

// Heaters and regenerations
void cmd_hv1on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U HV1ON"), F("Nafion heater1 on"));
    print_ok(context.stream);
}
void cmd_hv1off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U HV1OFF"), F("Nafion heater1 off"));
    print_ok(context.stream);
}
// hv2/hv3
void cmd_hi1on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U HI1ON"), F("Ion chamber heater1 on"));
    print_ok(context.stream);
}
void cmd_hi1off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U HI1OFF"), F("Ion chamber heater1 off"));
    print_ok(context.stream);
}

// Calibrations and sampling
void cmd_cv0on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U CV0ON"), F("Close calibration ground relay"));
    print_ok(context.stream);
}
void cmd_cv0off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U CV0OFF"), F("Open calibration ground relay"));
    print_ok(context.stream);
}
void cmd_cv_on(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U CVON"), F("Close calibration solenoid #"));
    print_ok(context.stream);
}
void cmd_cv_off(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U CVOFF"), F("Open calibration solenoid #"));
    print_ok(context.stream);
}

void cmd_cfluid(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U CFLUID"), F("Set sampling to fluid"));
    print_ok(context.stream);
}
void cmd_cbgnd(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U CBGND"), F("Set sampling to background"));
    print_ok(context.stream);
}
void cmd_cal(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U CAL"), F("Set sampling path to bag #"));
    String req = request_string(context);
    if (req.indexOf(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
}

// Automatic sequences and higher-level commands
void cmd_astart1(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ASTART1"), F("Start sequence1"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
    context.stream.print(F("M START1 finished"));
    print_line_end(context.stream);
}
void cmd_astart2(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ASTART2"), F("Start sequence2"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
    context.stream.print(F("M START2 finished"));
    print_line_end(context.stream);
}
void cmd_astandby(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ASTANDBY"), F("Goto standby"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
    context.stream.print(F("M STANDBY mode activated"));
    print_line_end(context.stream);
}
void cmd_apoweroff(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U APOWEROFF"), F("Power off to minimum"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
    context.stream.print(F("M POWEROFF mode activated"));
    print_line_end(context.stream);
}

void cmd_acal(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ACAL"), F("Initiate calibration with bag#"));
    String req = request_string(context);
    if (req.indexOf(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
    context.stream.print(F("M CAL1 end"));
    print_line_end(context.stream);
}
void cmd_acal9(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ACAL9"), F("Full calibration series"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
    context.stream.print(F("M CAL end"));
    print_line_end(context.stream);
}

void cmd_anafreg(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ANAFREG"), F("Nafion regeneration"));
    String req = request_string(context);
    if (req.indexOf(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
    context.stream.print(F("M Nafion Reg finished"));
    print_line_end(context.stream);
}
void cmd_aionreg(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U AIONREG"), F("Ion chamber regen"));
    String req = request_string(context);
    if (req.indexOf(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
    context.stream.print(F("M Ion Reg finished"));
    print_line_end(context.stream);
}

void cmd_asample(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ASAMPLE"), F("Predefined sampling cycle"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
    context.stream.print(F("M Sampling time set to 10"));
    print_line_end(context.stream);
    context.stream.print(F("M NAF"));
    print_line_end(context.stream);
    context.stream.print(F("M DIR"));
    print_line_end(context.stream);
    context.stream.print(F("M Sampling Finished"));
    print_line_end(context.stream);
}
void cmd_asamplexx(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ASAMPLEXX"), F("Sampling with custom minutes"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
    context.stream.print(F("M Sampling time set to 05"));
    print_line_end(context.stream);
    context.stream.print(F("M NAF"));
    print_line_end(context.stream);
    context.stream.print(F("M DIR"));
    print_line_end(context.stream);
    context.stream.print(F("M Sampling Finished"));
    print_line_end(context.stream);
}

void cmd_asplfnxx(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ASPLFNXX"), F("Sample fluids nafion mode"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
    context.stream.print(F("M Sampling time set to 60"));
    print_line_end(context.stream);
    context.stream.print(F("M NAF"));
    print_line_end(context.stream);
    context.stream.print(F("M fluid"));
    print_line_end(context.stream);
    context.stream.print(F("M Sampling Finished"));
    print_line_end(context.stream);
}
void cmd_asplfdxx(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ASPLFDXX"), F("Sample fluids direct mode"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
    context.stream.print(F("M Sampling time set to 60"));
    print_line_end(context.stream);
    context.stream.print(F("M DIR"));
    print_line_end(context.stream);
    context.stream.print(F("M fluid"));
    print_line_end(context.stream);
    context.stream.print(F("M Sampling Finished"));
    print_line_end(context.stream);
}
void cmd_asplbnxx(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ASPLBNXX"), F("Sample background nafion"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
    context.stream.print(F("M Sampling time set to 60"));
    print_line_end(context.stream);
    context.stream.print(F("M NAF"));
    print_line_end(context.stream);
    context.stream.print(F("M background"));
    print_line_end(context.stream);
    context.stream.print(F("M Sampling Finished"));
    print_line_end(context.stream);
}
void cmd_asplbdxx(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ASPLBDXX"), F("Sample background direct"));
    context.stream.print(F("M OK"));
    print_line_end(context.stream);
    context.stream.print(F("M Sampling time set to 60"));
    print_line_end(context.stream);
    context.stream.print(F("M DIR"));
    print_line_end(context.stream);
    context.stream.print(F("M background"));
    print_line_end(context.stream);
    context.stream.print(F("M Sampling Finished"));
    print_line_end(context.stream);
}

void cmd_asplstp(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U ASPLSTP"), F("Stop ASPL sampling"));
    print_ok(context.stream);
}
void cmd_aabort(LazySerial::Context &context)
{
    LAZY_COMMAND(F("U AABORT"), F("Abort current sequence"));
    context.stream.print(F("M ABORTED"));
    print_line_end(context.stream);
}

// Data and misc
void cmd_beat2(LazySerial::Context &context) { /* duplicate name handled earlier */ }

// Fallback: malformed or unimplemented commands should reply EC when registered accordingly.
// Note: registration with the command dispatcher (mapping command strings to these functions)
// is expected elsewhere in the codebase.
