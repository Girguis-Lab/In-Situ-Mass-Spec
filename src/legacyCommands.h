/*

NOTE: THIS IS A PARTIAL CONVERSION FROM THE OLD CODE AND IS NOT YET COMPLETE!

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
    stream.print(("M EC"));
    print_line_end(stream);
}

// Mock command implementations. Each command sends a reasonable mock response.
// If a parameterized command is missing its parameter this will reply "M EC".

void cmd_beat(LazySerial::Context &context)
{
    LAZY_COMMAND(("U BEAT"), "", ("Checking that the module is alive - responds with BEAT"));
    context.stream.print(("M BEAT"));
    print_line_end(context.stream);
}

void cmd_firm(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ?FIRM"), "", ("Request the firmware version - responds with version string"));
    context.stream.print("M " FIRMWARE_VERSION);
    print_line_end(context.stream);
}

void cmd_data(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ?DATA"), "", ("Request the data telegram"));
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
    LAZY_COMMAND(("U PTMPON"), "", ("Turn turbo pump on"));
    print_ok(context.stream);
}
void cmd_ptmpof(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PTMPOFF"), "", ("Turn turbo pump off"));
    print_ok(context.stream);
}
void cmd_prgaon(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PRGAON"), "", ("Turn RGA on"));
    print_ok(context.stream);
}
void cmd_prgaof(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PRGAOFF"), "", ("Turn RGA off"));
    print_ok(context.stream);
}
void cmd_pknf1on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PKNF1ON"), "", ("Main roughing pump on"));
    print_ok(context.stream);
}
void cmd_pknf1of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PKNF1OFF"), "", ("Main roughing pump off"));
    print_ok(context.stream);
}
void cmd_pknf2on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PKNF2ON"), "", ("Aux roughing pump on"));
    print_ok(context.stream);
}
void cmd_pknf2of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PKNF2OFF"), "", ("Aux roughing pump off"));
    print_ok(context.stream);
}

// Fans (1..4) ON/OFF
void cmd_pfan1on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PFAN1ON"), "", ("Fan1 on"));
    print_ok(context.stream);
}
void cmd_pfan1of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PFAN1OFF"), "", ("Fan1 off"));
    print_ok(context.stream);
}
void cmd_pfan2on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PFAN2ON"), "", ("Fan2 on"));
    print_ok(context.stream);
}
void cmd_pfan2of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PFAN2OFF"), "", ("Fan2 off"));
    print_ok(context.stream);
}
void cmd_pfan3on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PFAN3ON"), "", ("Fan3 on"));
    print_ok(context.stream);
}
void cmd_pfan3of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PFAN3OFF"), "", ("Fan3 off"));
    print_ok(context.stream);
}
void cmd_pfan4on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PFAN4ON"), "", ("Fan4 on"));
    print_ok(context.stream);
}
void cmd_pfan4of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PFAN4OFF"), "", ("Fan4 off"));
    print_ok(context.stream);
}

// Auxiliary valves and devices
void cmd_paux1on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PAUX1ON"), "", ("Aux vac valve1 on"));
    print_ok(context.stream);
}
void cmd_paux1of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PAUX1OFF"), "", ("Aux vac valve1 off"));
    print_ok(context.stream);
}
void cmd_paux2on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PAUX2ON"), "", ("Aux vac valve2 on"));
    print_ok(context.stream);
}
void cmd_paux2of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PAUX2OFF"), "", ("Aux vac valve2 off"));
    print_ok(context.stream);
}
void cmd_pphon(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PPHON"), "", ("pH meter on"));
    print_ok(context.stream);
}
void cmd_pphof(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PPHOFF"), "", ("pH meter off"));
    print_ok(context.stream);
}
void cmd_psbeon(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PSBEON"), "", ("SBE CTD pump on"));
    print_ok(context.stream);
}
void cmd_psbeof(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PSBEOFF"), "", ("SBE CTD pump off"));
    print_ok(context.stream);
}
void cmd_pheaton(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PHEATON"), "", ("Heaters power on"));
    print_ok(context.stream);
}
void cmd_pheatof(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PHEATOFF"), "", ("Heaters power off"));
    print_ok(context.stream);
}

// KNF start/stop high-level
void cmd_pknf0on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PKNF0ON"), "", ("Start KNF considering mode"));
    print_ok(context.stream);
}
void cmd_pknf0of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U PKNF0OFF"), "", ("Stop KNF considering mode"));
    print_ok(context.stream);
}

// External sensors control
void cmd_eiso(LazySerial::Context &context)
{
    LAZY_COMMAND(("U EISO"), "", ("Isolate external sensors"));
    print_ok(context.stream);
}
void cmd_eph(LazySerial::Context &context)
{
    LAZY_COMMAND(("U EPH"), "", ("EPH obsolete"));
    context.stream.print(("M EPH EC"));
    print_line_end(context.stream);
} // not valid anymore
void cmd_et(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ET"), "", ("Enable external temperature bus"));
    print_ok(context.stream);
}

// Queries
void cmd_caltime(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ?CALTIME"), "", ("Calibration bag usage times"));
    context.stream.print(("M 120,30,0"));
    print_line_end(context.stream);
}
void cmd_volt(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ?VOLT"), "", ("Voltage V1-V5"));
    context.stream.print(("M Reading Voltages\n V1: 12.34 V\n V2: 5.00 V\n V3: 3.30 V\n V4: 0.00 V\n V5: 4.98 V"));
    print_line_end(context.stream);
}
void cmd_extt(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ?EXTT"), "", ("Enumerate external temp sensor addresses"));
    context.stream.print(("M 28-FF-4C-92-60-1A-03-5A"));
    print_line_end(context.stream);
}
void cmd_id(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ?ID"), "", ("Get serial number"));
    context.stream.print(("M LTDISMS123"));
    print_line_end(context.stream);
}
void cmd_l(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ?L"), "", ("Request limits/status"));
    context.stream.print(("M Limits: Pmin=0,Pmax=5000;Tmin=-10,Tmax=60"));
    print_line_end(context.stream);
}
void cmd_s(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ?S"), "", ("Status byte"));
    context.stream.print(("M 0x5A"));
    print_line_end(context.stream);
}
void cmd_a(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ?A"), "", ("Firmware diagnostics"));
    context.stream.print(("M Diagnostics: All systems nominal"));
    print_line_end(context.stream);
}

// Settings commands
void cmd_setrate_ext(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETRATEEXT"), "", ("Set external rate"));
    String req = request_string(context);
    if (req.indexO(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(("M OK"));
    print_line_end(context.stream);
}
void cmd_setrate_int(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETRATEINT"), "", ("Set internal rate"));
    String req = request_string(context);
    if (req.indexO(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(("M OK"));
    print_line_end(context.stream);
}
void cmd_setrate_pow(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETRATEPOW"), "", ("Set power rate"));
    String req = request_string(context);
    if (req.indexO(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(("M OK"));
    print_line_end(context.stream);
}
void cmd_setrate_sol(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETRATESOL"), "", ("Set solenoid rate"));
    String req = request_string(context);
    if (req.indexO(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(("M OK"));
    print_line_end(context.stream);
}
void cmd_setrate_aut(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETRATEAUT"), "", ("Set autocheck rate"));
    String req = request_string(context);
    if (req.indexO(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(("M OK"));
    print_line_end(context.stream);
}
void cmd_setrate_dlr(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETRATEDLR"), "", ("Set data broadcast rate"));
    String req = request_string(context);
    if (req.indexO(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(("M OK"));
    print_line_end(context.stream);
}

void cmd_setdbon(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETDBON"), "", ("Turn data broadcast on"));
    print_ok(context.stream);
}
void cmd_setdbof(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETDBOFF"), "", ("Turn data broadcast off"));
    print_ok(context.stream);
}
void cmd_setprwon(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETPRWON"), "", ("Pressure watch on"));
    print_ok(context.stream);
}
void cmd_setprwof(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETPRWOFF"), "", ("Pressure watch off"));
    print_ok(context.stream);
}
void cmd_setprli(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETPRLI"), "", ("Set pressure limit"));
    String req = request_string(context);
    if (req.indexO(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(("M OK"));
    print_line_end(context.stream);
}

void cmd_setresetcoms2(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETRESETCOMS2"), "", ("Reset COM2"));
    context.stream.print(("M C2Reseted"));
    print_line_end(context.stream);
}
void cmd_setresetcoms3(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETRESETCOMS3"), "", ("Reset COM3"));
    context.stream.print(("M C3Reseted"));
    print_line_end(context.stream);
}
void cmd_setresetp(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETRESETP"), "", ("Reset power module"));
    context.stream.print(("M PowerBoard reseted"));
    print_line_end(context.stream);
}
void cmd_setresets(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETRESETS"), "", ("Reset solenoid board"));
    context.stream.print(("M SolenoidBoard reseted"));
    print_line_end(context.stream);
}

void cmd_setlog1on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETLOG1ON"), "", ("Logging level1 on"));
    print_ok(context.stream);
}
void cmd_setlog2on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETLOG2ON"), "", ("Logging level2 on"));
    print_ok(context.stream);
}
void cmd_setlogof(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETLOGOFF"), "", ("Logging off"));
    print_ok(context.stream);
}

void cmd_setfan1aon(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETFAN1AON"), "", ("Fan1 auto"));
    print_ok(context.stream);
}
void cmd_setfan1aof(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETFAN1AOFF"), "", ("Fan1 manual"));
    print_ok(context.stream);
}
// Fan 2..4
void cmd_setfan2aon(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETFAN2AON"), "", ("Fan2 auto"));
    print_ok(context.stream);
}
void cmd_setfan2aof(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETFAN2AOFF"), "", ("Fan2 manual"));
    print_ok(context.stream);
}
void cmd_setfan3aon(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETFAN3AON"), "", ("Fan3 auto"));
    print_ok(context.stream);
}
void cmd_setfan3aof(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETFAN3AOFF"), "", ("Fan3 manual"));
    print_ok(context.stream);
}
void cmd_setfan4aon(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETFAN4AON"), "", ("Fan4 auto"));
    print_ok(context.stream);
}
void cmd_setfan4aof(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETFAN4AOFF"), "", ("Fan4 manual"));
    print_ok(context.stream);
}

void cmd_setknfmode(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETKNFMODE"), "", ("Set KNF mode"));
    String req = request_string(context);
    if (req.indexO(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(("M KNFMode set to 1"));
    print_line_end(context.stream);
}
void cmd_settwatchon(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETTWATCHON"), "", ("Temperature watch on"));
    print_ok(context.stream);
}
void cmd_settwatchof(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETTWATCHOFF"), "", ("Temperature watch off"));
    print_ok(context.stream);
}
void cmd_setminute(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETMINUTEXXXXX"), "", ("Set minute millis"));
    String req = request_string(context);
    if (req.indexO(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(("M Minute set to 1000"));
    print_line_end(context.stream);
}

void cmd_setreadeeprom(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETREADEEPROM"), "", ("Read EEPROM"));
    context.stream.print(("M EEPROM: knfmode=1,caltimes=0,0,0"));
    print_line_end(context.stream);
}
void cmd_setreseteeprom(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETRESETEEPROM"), "", ("Reset EEPROM defaults"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
}
void cmd_settadd(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETTADD"), "", ("Set temp sensor address"));
    context.stream.print(("M Follow instructions to write new address"));
    print_line_end(context.stream);
}
void cmd_setpk(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETPK"), "", ("Set secondary pressure correction"));
    String req = request_string(context);
    if (req.indexO(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(("M OK"));
    print_line_end(context.stream);
}
void cmd_setresetce(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETRESETCE"), "", ("Reset critical error status"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
}
void cmd_setl(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETLXXTXXX"), "", ("Set limit value"));
    String req = request_string(context);
    if (req.indexO(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(("M Will ask for confirmation"));
    print_line_end(context.stream);
}
void cmd_setld(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETLD"), "", ("Reset limits to default"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
}
void cmd_setwdton(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETWDTON"), "", ("Watchdog on"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
}
void cmd_setwdtof(LazySerial::Context &context)
{
    LAZY_COMMAND(("U SETWDTOFF"), "", ("Watchdog off"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
}

// Valve and sampling routing (V1..V4)
void cmd_v1on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V1ON"), "", ("V1 to 1"));
    print_ok(context.stream);
}
void cmd_v1of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V1OFF"), "", ("V1 to 0"));
    print_ok(context.stream);
}
void cmd_v1onl(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V1ONL"), "", ("V1 long pulse to 1"));
    print_ok(context.stream);
}
void cmd_v1ofl(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V1OFL"), "", ("V1 long pulse to 0"));
    print_ok(context.stream);
}
// V2..V4 similar
void cmd_v2on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V2ON"), "", ("V2 to 1"));
    print_ok(context.stream);
}
void cmd_v2of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V2OFF"), "", ("V2 to 0"));
    print_ok(context.stream);
}
void cmd_v2onl(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V2ONL"), "", ("V2 long to 1"));
    print_ok(context.stream);
}
void cmd_v2ofl(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V2OFL"), "", ("V2 long to 0"));
    print_ok(context.stream);
}
void cmd_v3on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V3ON"), "", ("V3 on"));
    print_ok(context.stream);
}
void cmd_v3of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V3OFF"), "", ("V3 off"));
    print_ok(context.stream);
}
void cmd_v3onl(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V3ONL"), "", ("V3 long on"));
    print_ok(context.stream);
}
void cmd_v3ofl(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V3OFL"), "", ("V3 long off"));
    print_ok(context.stream);
}
void cmd_v4on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V4ON"), "", ("V4 on"));
    print_ok(context.stream);
}
void cmd_v4of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V4OFF"), "", ("V4 off"));
    print_ok(context.stream);
}
void cmd_v4onl(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V4ONL"), "", ("V4 long on"));
    print_ok(context.stream);
}
void cmd_v4ofl(LazySerial::Context &context)
{
    LAZY_COMMAND(("U V4OFL"), "", ("V4 long off"));
    print_ok(context.stream);
}

void cmd_visoms(LazySerial::Context &context)
{
    LAZY_COMMAND(("U VISOMS"), "", ("Isolate MS from routing"));
    print_ok(context.stream);
}
void cmd_visoin(LazySerial::Context &context)
{
    LAZY_COMMAND(("U VISOIN"), "", ("Isolate membrane inlet"));
    print_ok(context.stream);
}
void cmd_vdir(LazySerial::Context &context)
{
    LAZY_COMMAND(("U VDIR"), "", ("Set inlet inline with MS"));
    print_ok(context.stream);
}
void cmd_vnafi(LazySerial::Context &context)
{
    LAZY_COMMAND(("U VNAFI"), "", ("Set nafion dried inlet"));
    print_ok(context.stream);
}
void cmd_vnafv(LazySerial::Context &context)
{
    LAZY_COMMAND(("U VNAFV"), "", ("Set nafion to roughing vacuum"));
    print_ok(context.stream);
}
void cmd_vmson(LazySerial::Context &context)
{
    LAZY_COMMAND(("U VMSON"), "", ("Open line to MS, check pressure"));
    context.stream.print(("M OP"));
    print_line_end(context.stream);
}
void cmd_vmson(LazySerial::Context &context)
{
    LAZY_COMMAND(("U VMSONF"), "", ("Force open line to MS"));
    context.stream.print(("M MS on"));
    print_line_end(context.stream);
}

// Heaters and regenerations
void cmd_hv1on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U HV1ON"), "", ("Nafion heater1 on"));
    print_ok(context.stream);
}
void cmd_hv1of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U HV1OFF"), "", ("Nafion heater1 off"));
    print_ok(context.stream);
}
// hv2/hv3
void cmd_hi1on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U HI1ON"), "", ("Ion chamber heater1 on"));
    print_ok(context.stream);
}
void cmd_hi1of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U HI1OFF"), "", ("Ion chamber heater1 off"));
    print_ok(context.stream);
}

// Calibrations and sampling
void cmd_cv0on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U CV0ON"), "", ("Close calibration ground relay"));
    print_ok(context.stream);
}
void cmd_cv0of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U CV0OFF"), "", ("Open calibration ground relay"));
    print_ok(context.stream);
}
void cmd_cv_on(LazySerial::Context &context)
{
    LAZY_COMMAND(("U CVON"), "", ("Close calibration solenoid #"));
    print_ok(context.stream);
}
void cmd_cv_of(LazySerial::Context &context)
{
    LAZY_COMMAND(("U CVOFF"), "", ("Open calibration solenoid #"));
    print_ok(context.stream);
}

void cmd_cfluid(LazySerial::Context &context)
{
    LAZY_COMMAND(("U CFLUID"), "", ("Set sampling to fluid"));
    print_ok(context.stream);
}
void cmd_cbgnd(LazySerial::Context &context)
{
    LAZY_COMMAND(("U CBGND"), "", ("Set sampling to background"));
    print_ok(context.stream);
}
void cmd_cal(LazySerial::Context &context)
{
    LAZY_COMMAND(("U CAL"), "", ("Set sampling path to bag #"));
    String req = request_string(context);
    if (req.indexO(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(("M OK"));
    print_line_end(context.stream);
}

// Automatic sequences and higher-level commands
void cmd_astart1(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ASTART1"), "", ("Start sequence1"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
    context.stream.print(("M START1 finished"));
    print_line_end(context.stream);
}
void cmd_astart2(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ASTART2"), "", ("Start sequence2"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
    context.stream.print(("M START2 finished"));
    print_line_end(context.stream);
}
void cmd_astandby(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ASTANDBY"), "", ("Goto standby"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
    context.stream.print(("M STANDBY mode activated"));
    print_line_end(context.stream);
}
void cmd_apowerof(LazySerial::Context &context)
{
    LAZY_COMMAND(("U APOWEROFF"), "", ("Power off to minimum"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
    context.stream.print(("M POWEROFF mode activated"));
    print_line_end(context.stream);
}

void cmd_acal(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ACAL"), "", ("Initiate calibration with bag#"));
    String req = request_string(context);
    if (req.indexO(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(("M OK"));
    print_line_end(context.stream);
    context.stream.print(("M CAL1 end"));
    print_line_end(context.stream);
}
void cmd_acal9(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ACAL9"), "", ("Full calibration series"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
    context.stream.print(("M CAL end"));
    print_line_end(context.stream);
}

void cmd_anafreg(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ANAFREG"), "", ("Nafion regeneration"));
    String req = request_string(context);
    if (req.indexO(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(("M OK"));
    print_line_end(context.stream);
    context.stream.print(("M Nafion Reg finished"));
    print_line_end(context.stream);
}
void cmd_aionreg(LazySerial::Context &context)
{
    LAZY_COMMAND(("U AIONREG"), "", ("Ion chamber regen"));
    String req = request_string(context);
    if (req.indexO(' ') < 0)
    {
        print_ec(context.stream);
        return;
    }
    context.stream.print(("M OK"));
    print_line_end(context.stream);
    context.stream.print(("M Ion Reg finished"));
    print_line_end(context.stream);
}

void cmd_asample(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ASAMPLE"), "", ("Predefined sampling cycle"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
    context.stream.print(("M Sampling time set to 10"));
    print_line_end(context.stream);
    context.stream.print(("M NAF"));
    print_line_end(context.stream);
    context.stream.print(("M DIR"));
    print_line_end(context.stream);
    context.stream.print(("M Sampling Finished"));
    print_line_end(context.stream);
}
void cmd_asamplexx(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ASAMPLEXX"), "", ("Sampling with custom minutes"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
    context.stream.print(("M Sampling time set to 05"));
    print_line_end(context.stream);
    context.stream.print(("M NAF"));
    print_line_end(context.stream);
    context.stream.print(("M DIR"));
    print_line_end(context.stream);
    context.stream.print(("M Sampling Finished"));
    print_line_end(context.stream);
}

void cmd_asplfnxx(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ASPLFNXX"), "", ("Sample fluids nafion mode"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
    context.stream.print(("M Sampling time set to 60"));
    print_line_end(context.stream);
    context.stream.print(("M NAF"));
    print_line_end(context.stream);
    context.stream.print(("M fluid"));
    print_line_end(context.stream);
    context.stream.print(("M Sampling Finished"));
    print_line_end(context.stream);
}
void cmd_asplfdxx(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ASPLFDXX"), "", ("Sample fluids direct mode"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
    context.stream.print(("M Sampling time set to 60"));
    print_line_end(context.stream);
    context.stream.print(("M DIR"));
    print_line_end(context.stream);
    context.stream.print(("M fluid"));
    print_line_end(context.stream);
    context.stream.print(("M Sampling Finished"));
    print_line_end(context.stream);
}
void cmd_asplbnxx(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ASPLBNXX"), "", ("Sample background nafion"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
    context.stream.print(("M Sampling time set to 60"));
    print_line_end(context.stream);
    context.stream.print(("M NAF"));
    print_line_end(context.stream);
    context.stream.print(("M background"));
    print_line_end(context.stream);
    context.stream.print(("M Sampling Finished"));
    print_line_end(context.stream);
}
void cmd_asplbdxx(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ASPLBDXX"), "", ("Sample background direct"));
    context.stream.print(("M OK"));
    print_line_end(context.stream);
    context.stream.print(("M Sampling time set to 60"));
    print_line_end(context.stream);
    context.stream.print(("M DIR"));
    print_line_end(context.stream);
    context.stream.print(("M background"));
    print_line_end(context.stream);
    context.stream.print(("M Sampling Finished"));
    print_line_end(context.stream);
}

void cmd_asplstp(LazySerial::Context &context)
{
    LAZY_COMMAND(("U ASPLSTP"), "", ("Stop ASPL sampling"));
    print_ok(context.stream);
}
void cmd_aabort(LazySerial::Context &context)
{
    LAZY_COMMAND(("U AABORT"), "", ("Abort current sequence"));
    context.stream.print(("M ABORTED"));
    print_line_end(context.stream);
}

// Data and misc
void cmd_beat2(LazySerial::Context &context) { /* duplicate name handled earlier */ }

// Fallback: malformed or unimplemented commands should reply EC when registered accordingly.
// Note: registration with the command dispatcher (mapping command strings to these functions)
// is expected elsewhere in the codebase.
