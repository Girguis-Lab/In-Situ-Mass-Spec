<b style="color:#9b0000; font-size: 1.7rem">User Guide</b>

# In-Situ Mass Spectrometer V3

![Header-Image](<./Images/user-guide/Header-Image.jpg>)



## System Overview

![External-Component-Connections-Overview](<./Images/user-guide/External-Component-Connections-Overview.png>)

<center><b>External components of the ISMS</b></center>

A standard ISMS setup consists of the main instrument bottle with several external components connected by a flexible plastic fluid line as well as a branching power/data cable. The sampling fluid (typically seawater) is sucked in at the sampling wand, brought to local ambient temperature by an optional short heat exchanger, past the membrane inlet. The membrane is selectively permiable to gases and volatiles while excluding the liquid water. These gases enter the vacuum chamber to be measured by the internal quad pole mass spectrometer. Once flowed past the membrane inlet, the sampled water continues through the fluid pump and passes through an optional pH probe before being expelled back into the environment. Communication with and power for the ISMS are handled through a single 8-pin subconn connector with two RS232 serial connections \- one for interacting with the ISMS main control board and another for directly communicating with and reading data from the onboard Residual Gas Analyzer (Mass Spectrometer). Power is supplied through two separate power inputs within the 8-pin subconn each supporting power input between 24 & 48 volts.

## Vehicle Setup

See the [ISMS V3 Vehicle Integration Guide.md](<../Fabrication/ISMS V3 Vehicle Integration Guide.md>)
![Recommended-Vehicle-Layout](<./Images/user-guide/Recommended-Vehicle-Layout.png>)
<center><b>Typical ISMS layout on a vehicle deployment</b></center>



## Operation

When running the ISMS on the bench and at sea there are is a common set of steps to run the instrument, take measurements, and properly shut down the instrument.

1. Locate the 8-pin subconn connector from the ISMS cable or the pass-through vehicle connections that connect to the ISMS.
   1. On the bench or where the 8-pins subconn is accessible, connect the included splitter cable to the subconn which should provide two power inputs as well as two DB9 connectors \- one for the ISMS serial connection and another for the internal RGA connection (which has the “special handshake” cable end).
2. Connect the DB9 RS232 plug from the ISMS labeled “**ISMS Main Serial**” to your computer via the provided RS232 to usb adapter cables.
3. Open serial studio (or termite/putty/etc) window and check which port appears when clicking the port selection dropdown when the usb adapter cable is plugged into the computer USB port (on Windows they will look like COMx and on mac/linux they will look like cu.usbserial-xxx) \- you may need to close and reopen the menu to update the list of connected serial ports \- more on how to use serial studio is found in the next section.
4. Connect the other DB9 RS232 plug from the ISMS labeled “**ISMS RGA Serial**” to your computer via an RS232 to usb adapter cable.
5. Connect a 24 or 48 volt power supply that can handle up to 7 amps at 24v to both of the power inputs (alternatively connect two separate power supplies that can handle 4 amps to each of the power inputs)
   1. If the vehicle provides power, turn on the relevant power switches,
6. One serial studio window should show “Waiting for startup…” followed by several boot messages, while the other may show gibberish or nothing.
   1. If neither serial connection shows anything, insert “null modem” adapters between BOTH DB9 plugs \- swapping tx and rx is a common source of error.
7. Open the SRS RGASoft software (Download [here](https://www.thinksrs.com/downloads/soft.html)) and choose the RGA which should appear on the other serial port.
8. Use the next section to Setup the serial studio dashboard for using the ISMS in a nicer way. (This is optional as all the system information is readable via the raw serial console.)
9. Startup the ISMS by either clicking on “Startup All” in Serial Studio or sending the command “STARTUP” (without quotes) to the **ISMS Main Serial** connection.
   1. This will start the roughing pump & fluid pumps first and then start the turbo pumps 1 minute later, alternatively start the pumps manually by sending “ROUGHING ON” and “FLUIDPUMP ON” followed by “TURBO ON” after at least 30 seconds wait.
10. Watch the turbo ramp up time and current \- it should only take at most 40 seconds and have peak current about 4.3 amps, if longer times or higher currents are seen this could indicate a slow leak \- turn off turbo and try leaving the roughing pump on longer before enabling the turbo \- see troubleshooting guide.
    1. It’s a good idea to leave the roughing and turbo pump running with the inlet capped for at least 30 minutes before deploying and 24 hours if the internal vacuum chamber was opened to ensure residual moisture and volatiles that may have stuck to the chamber walls are flushed before sampling the target fluid.



### Shutdown

1. First stop the current scan in the SRS RGA software and **turn off the filament** by going to toolbar > probe menu \> turn off filament.
2. Turn off the turbo pump motor in Serial Studio (Serial command: `TURBO OFF`) and wait for it to spin down which **may take up to 25 minutes.**
3. Unplug/disable power input to the ISMS
4. Click disconnect in Serial Studio and unplug serial connections.



## Serial Studio Dashboard

ISMS v3 has a visual dashboard that utilizes the free Serial Studio software found here: [https://serial-studio.com](https://serial-studio.com/)

Download the project file from [here](https://github.com/Girguis-Lab/In-Situ-Mass-Spec/blob/ifremer-2026-build/Ifremer-isms-documentation/Ifremer%20Modern%20ISMS%20Serial%20Studio%20Dashboard.ssproj) and open it in Serial Studio.

![Serial-Studio-Connection](<./Images/user-guide/Serial-Studio-Connection.png>)

1. Choose data export options to record the system data while the instrument is running.
   \-\> CSV Spreadsheet and Console export options are recommended.
2. Select the COM port for the ISMS with the shown options.
3. Hit connect in the upper right \-\> If the com port is correct, The ISMS will restart and you will see “Waiting for startup…” followed by more lines and the dashboard will open. If you see garbled messages, nothing, or data is not updating try a different com port, ensure serial parameters are as shown or try adding a null modem plug to the RS232 lines \- RX & TX get swapped all the time.

![Serial-Studio-ISMS-View](<./Images/user-guide/Serial-Studio-ISMS-View.png>)

The dashboard should look like this \- if the raw console opens in a new window, you can close it.

![Serial-Studio-Console-Button](<./Images/user-guide/Serial-Studio-Console-Button.png>)

At the top are action buttons for controlling the ISMS:

![Serial-Studio-ISMS-Action-Bar](<./Images/user-guide/Serial-Studio-ISMS-Action-Bar.png>)

| In the bottom left corner are any active warnings coming from the ISMS as well as utility buttons such as to open the raw serial console, a stopwatch, a play/pause control for the incoming data, etc… | In the lower right corner you can choose different views of the data such as plots or the default overview. |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| ![Serial-Studio-Botom-Bar](<./Images/user-guide/Serial-Studio-Botom-Bar.png>) | ![Serial-Studio-View-Selection](<./Images/user-guide/Serial-Studio-View-Selection.png>) |

To access the command line interface of the ISMS, use the console button. Commands can be typed in the box at the bottom and the output will be displayed on the console.

![Serial-Studio-Console-Button](<./Images/user-guide/Serial-Studio-Console-Button.png>)

## Command Line Interface

Commands are case insensitive, and use underscores for the command and space between the command and argument(s)

### Standard Commands

**HELP** *DEBUG (optional)*

* **Example:** `HELP DEBUG`
  Shows a help message; add DEBUG to show additional troubleshooting commands.

**BEAT**

* **Example:** `BEAT`
  Send BEAT to stop autostart for this boot cycle; used to temporarily enforce fully manual control.

**STARTUP**

* **Example:** `STARTUP`
  Run full auto startup sequence.

**ROUGHING** *ON|OFF*

* **Example:** `ROUGHING ON`
  Turn the roughing pump power on or off.

**CRYO** *ON|OFF*

* **Example:** `CRYO OFF`
  Turn the cryotrap power on or off.

**FLUIDPUMP** *ON|OFF*

* **Example:** `FLUIDPUMP ON`
  Turn the fluid pump power on or off.

**FLUIDPUMP\_RATE** *\-100-100*

* **Example:** `FLUIDPUMP_RATE -50`
  Sets the fluid pumping rate as a percentage of full speed. Negative values run the pump in reverse. 0 will turn off pump power.

**TURBO** *ON|OFF*

* **Example:** `TURBO ON`
  Power ON/OFF turbo pump at configured speed.

**TURBO\_SPEED** *0.0-100.0*

* **Example:** `TURBO_SPEED 75.0`
  Sets the turbo pump to run at a target speed as percent of max speed (100% is 90,000 RPM for the Pfeiffer TC80). Send 0 to reset to Pfeiffer default speed control mode.

**TURBO\_LIMIT\_PWR** *0-100*

* **Example:** `TURBO_LIMIT_PWR 60`
  Sets the turbo pump power limit as a percentage of full power (Pfeiffer max draw is 5A).

**TURBO\_CLEAR\_ERRORS**

* **Example:** `TURBO_CLEAR_ERRORS`
  Clears turbo pump errors and warning messages.

**STATS\_INTERVAL** *MS|OFF*

* **Example:** `STATS_INTERVAL 1000`
  Sets how often the system broadcasts stats to topside serial in milliseconds. Send OFF to disable stats logging.

**SET\_AUTOSTART** *ON|OFF|DELAY*

* **Example:** `SET_AUTOSTART 5000`
  Set whether the system should startup automatically. If an integer is passed, it is the delay in milliseconds after boot when the automatic startup routine will begin (unless the  BEAT command is sent beforehand).

### Troubleshooting Commands

**VERSION**

* **Example:** `VERSION`
  Prints firmware version.

**PINOUT**

* **Example:** `PINOUT`
  Prints configured Arduino pinout and I2C/RS485 addresses.

**DEBUG\_LOGGING** *OFF|LOW|HIGH*

* **Example:** `DEBUG_LOGGING HIGH`
  Enable additional logging to serial for troubleshooting purposes.

**RESET\_SETTINGS**

* **Example:** `RESET_SETTINGS`
  Resets settings saved on the ISMS to their default values (does not change TURBO pump or RGA parameters, which are saved within their respective instrument).

**TURBO\_RESET**

* **Example:** `TURBO_RESET`
  Resets turbo pump parameters for default operation in normal conditions (light gases, default speed control mode, power limit 100%). Use this if the turbo is misbehaving.

**TURBO\_QUERY** *PARAMETER\_NUMBER*

* **Example:** `TURBO_QUERY 398`
  Query a parameter from the turbo pump with the 3-digit parameter number. See TC80 Manual Page

**TURBO\_CMD** *PARAM DATA*

* **Example:** `TURBO_CMD 702 00123`
  Send a command to the turbo pump with the 3-digit parameter number and optional data string.

**TURBO\_RAW** *COMMAND*

* **Example:** `TURBO_RAW 0010039802=?115`
  Send raw ASCII to the turbo pump. See

**GPIO** *PIN\_NUMBER ON|OFF*

* **Example:** `GPIO 13 ON`
  Set any Arduino pin high or low – TESTING ONLY, potentially dangerous.



### Output Format

The modern code serial outputs follow a consistent pattern:

* Lines starting with  `+`    |     `->`  or  `<-`   are responses from the ISMS that can be ignored by system stats parsing code.
* Lines starting with  `->`  are messages sent to the Turbo Pump (When logging ≥ debug)
* Lines starting with  `<-`  are responses from the Turbo Pump (When logging ≥ debug)
* Lines starting with    `!`    are warnings or errors.

Other lines are the system status update that is sent periodically and follows a key-value pair format:

`Fluidpump_Rate:⁠-50%,Turbo_RPM_Setpoint:90000,Turbo_RPM:32000(!),
Turbo_Current:4.40A(!),Turbo_Rotor_Temp:21C,Turbo_Elec_Temp:28C,
Turbo_Bottom_Temp:21C,Roughing_on:1,Fluidpump_on:1,Cryo_on:0`

* Keys and values are separated by a colon, and each pair is separated by comma.
* Values are always numeric but may start with minus and end in a unit and/or the characters (\!) which indicate that the value is outside of the expected range of normal operation. Note that the (\!) is expected to appear while the instrument is pumping out or while the instrument is spinning down.
* Values for Roughing\_on, Fluidpump\_on, and Cryo\_on indicate if that component is powered where 0 indicates OFF and 1 indicates ON

* If the instrument is interrupted while polling sensors and sending this line, it will print ,TRUNICATED after the last valid field.
