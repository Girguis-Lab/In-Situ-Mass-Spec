
/* Used for Arduino Mega for the ISMS
  This is for the legacy-style ISMS built for ifremer for Cedric & collegues in 2026
  F() macro is used for long strings so they get stored in the flash and not RAM

  Includes external pump control using the comand U SPRATEXXX where XXX is the pumping rate in percent
  Includes power control commands for the cryo cooler.

  Commands and output format are documented in the acompanying word document found in the Ifremer-isms-documentation folder.
  Some diffrences in commands and output exist from prior legacy ISMS versions,
  Only the data message sections dedicated to the turbo pump and power are kept.
  Unused functionality has been cut from prior versions

  Includes 'autostart' which will automatically start the system (power up Roughing1, then
  spins up turbo, then cryo cooler and FluidPump).
*/

#include <Wire.h>
#include <avr/wdt.h>
#include <EEPROM.h>

// ------------- Declarations------------------------

String firmware = "3.5";
String ID = ("Gold");

String ok = "OK";

// an array to store the addresses of all the I2C devices found
byte i2cAddresses[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const char term = '\n'; // newline - previously was carrige return!
const int comSpeedTurbo = 9600;
int comSpeedMain = 9600;
unsigned long waitingForBeat = 60000; // time before autostart (waiting for U BEAT from user to initialize manual operation)
unsigned long lastDefaultBroadcast = 0;
unsigned long defaultBroadcastDelay = 30000;
unsigned long BeatInterval = 40000;
unsigned long lastBeat = 0;

String toSendMain;
String received;
String toSend;

double y = 100; // setting the pumping rate value to 100%; adjust this value to change the automatic pump rate setting (0-100%; 0-320 mL/min)
double z = round((y / 100) * 255);

// boolean wtdEnabled=false;

// pfeifferTurbo
char Telegram[30] = {'\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n'}; // 30 is the maximum length
char TeleResponse[30] = {'\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n', '\n'};
char dataArray[16] = {
    '\n',
    '\n',
    '\n',
    '\n',
    '\n',
    '\n',
    '\n',
    '\n',
    '\n',
    '\n',
    '\n',
    '\n',
    '\n',
    '\n',
    '\n',
    '\n'};
char turboError[7] = {'\n', '\n', '\n', '\n', '\n', '\n', '\n'};
boolean turboInError = false;
unsigned long turboInfo[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 0 is current speed, 1 is overtemp drive, 2 is overtemp pump, 3 is drive current, 4 is drive voltage, 5 is eltemp, 6 is temp bottom pump, 7 is power stage temp, 8 is motor temp, 9 is set speed

byte turboPowerSetting;
boolean tmpCtrl = true;
boolean tmpon = false;       // change when power is applied
boolean initialized = false; // marker for beat
boolean defaulted = false;   // default startup in to automatic mode

//-------------------------------------------------------------- databroadcasting
boolean dataBroadcasting = false;
unsigned long DataBroadcastingInterval = 30000;
unsigned long lastBroadcast = 0;

boolean RoughingOn = false;

//--------------------------------------------------------------- reading controls

unsigned long TurboStatusReadingInterval = 2000; // L
unsigned long lastTStatRead = 0;

//--------------------------------------------------------------- pin assignments

// const int PwrTurbo = 40; // Not applicable to Ifremer 2026 ISMS model
// const int PwrRGA = 42; // Not applicable to Ifremer 2026 ISMS model
const int PwrRoughing1 = 51;
const int PwrCryo = 49;
const int PwrFluidPump = 53;
const int pumpratepin = 8;

char receivedMain[30];

//////////////////////////////////////////////////////////////////////////////////////SETUP///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  wdtStart();
  resetWDT();
  Serial.begin(comSpeedMain);
  //  if(debug)Serial.println("debug 00");//DEB
  Serial.print("M MCU initializing");
  Serial.write(term);
  // ----------------------------------------------------------------------set pin modes---------------------------------------------
  for (int i = 30; i < 54; i++)
  {
    pinMode(i, OUTPUT);
  }

  analogWrite(pumpratepin, 0);
  resetWDT();

  // ----------------------------------------------------------------------Prepare for turbo----------------------------------------

  Serial1.begin(comSpeedTurbo);
  powerTurbo(true);
  delay(1000);
  pfeifferTurboStart(false);
  delay(1000);
  powerTurbo(false);

  // ------------------------------------------------------------------initialise networks and sensor checks -------------------------

  resetWDT();
  Wire.begin();
  i2cScan();

  // ----------------------------------------------------------------Setup finished --------------------------------------------------
  toSendMain = "MainModule Online";
  communicate(0);
  // initiates the DB at 30sec interval
  dataBroadcasting = true;
  // 3.0 ENABLE watchdog
}

//////////////////////////////////////////////////////////////////////New Methods /////////////////////////////////////////////////////////////////////////////////////

boolean MainBeat()
{
  initialized = true;
  defaulted = false;
  toSendMain = "BEAT";
  communicate(0);
  return true;
}

//------------------------------------------------------------------------WDT------------------------------------------------------------------------------------------------

boolean wdtOn = true;

void wdtStart()
{
  Serial.println("M Watchdog_Timer_Enabled");
  wdtOn = true;
  wdt_enable(WDTO_8S);
}
void wdtStop()
{
  Serial.println("M Watchdog_Timer_Disabled");
  wdtOn = false;
  wdt_disable();
}
void resetWDT()
{
  if (wdtOn)
  {
    wdt_reset();
  }
}

// Flush the incoming buffer from serial1 (turbo pump)
void flushS1()
{
  while (Serial1.available() > 0)
  {
    Serial1.read();
  }
}

//----------------------------------------------------------------------- pfeifferTurbo-----------------------------------------------------------------------------------------------

boolean pfeifferTurboMain()
{ // pass directly a char array.

  toSendMain = ok;
  /* command list
    TON
    TOFF
    TSxxx  = set speed at % rpm
    TPxxx
    TAE    ackonledge error
  */
  if (receivedMain[0] == 'O')
  {
    if (receivedMain[1] == 'N')
    {
      return pfeifferTurboStart(true);
    }
    else if (receivedMain[1] == 'F')
    {
      return pfeifferTurboStart(false);
    }
  }

  else if (receivedMain[0] == 'S')
  {
    if (receivedMain[1] == '1')
    {
      return pfeifferTurboSetSpeed(100);
    }
    else
    {
      return pfeifferTurboSetSpeed((byte)((receivedMain[2] - 48) * 10 + receivedMain[3] - 48));
    }
  }
  else if (receivedMain[0] == 'P')
  {
    if (receivedMain[1] == '1')
    {
      return pfeifferTurboSetPower(100);
    }
    else
    {
      return pfeifferTurboSetPower((byte)((receivedMain[2] - 48) * 10 + receivedMain[3] - 48));
    }
  }
  else if (receivedMain[0] == 'A')
  {
    acknowledgeError();
    return true;
  }
  return false;
}

const char commands[3][15] = {
    // '0','0','1','1','0' will have to be added afterwards. because it is always the same, no need to take up space
    {'0', '1', '0', '0', '6', '1', '1', '1', '1', '1', '1', '0', '1', '5', '\r'}, // 0 is switching pumping station on
    {'0', '2', '3', '0', '6', '1', '1', '1', '1', '1', '1', '0', '1', '9', '\r'}, // 1 is swithcing motor on
    {'0', '1', '0', '0', '6', '0', '0', '0', '0', '0', '0', '0', '0', '9', '\r'}  // 2 is switching motor off
};

const char getInfo[10][11] = {
    // '0','0','1','0','0' will have to be added afterwards. because it is alwazs the same, no need to take up space
    {'3', '9', '8', '0', '2', '=', '?', '1', '1', '5', '\r'}, // get speed in RPM
    {'3', '0', '4', '0', '2', '=', '?', '1', '0', '2', '\r'}, // get over temp drive
    {'3', '0', '5', '0', '2', '=', '?', '1', '0', '3', '\r'}, // get over temp pump
    {'3', '1', '0', '0', '2', '=', '?', '0', '9', '9', '\r'}, // get drive current
    {'3', '1', '3', '0', '2', '=', '?', '1', '0', '2', '\r'}, // get drive voltage
    {'3', '2', '6', '0', '2', '=', '?', '1', '0', '6', '\r'}, // get electronic temps
    {'3', '3', '0', '0', '2', '=', '?', '1', '0', '1', '\r'}, // get pump bottom temp
    {'3', '2', '4', '0', '2', '=', '?', '1', '0', '4', '\r'}, // get power stage temp
    {'3', '4', '6', '0', '2', '=', '?', '1', '0', '8', '\r'}, // get motor temp
    {'3', '9', '7', '0', '2', '=', '?', '1', '1', '4', '\r'}  // get setSpeed
};

boolean getPfeifferTurboStatus()
{
  boolean err = pfeifferTurboGetError();
  pfeifferTurboGetInfo();
  return !err;
}

boolean pfeifferTurboStart(boolean on)
{
  resetTelegram();
  resetWDT();
  Telegram[0] = '0';
  Telegram[1] = '0';
  Telegram[2] = '1';
  Telegram[3] = '1';
  Telegram[4] = '0';
  if (on)
  {
    for (int j = 5; j < 20; j++)
    {
      Telegram[j] = commands[0][j - 5];
    }
    bool ok = sendCommandTelegram();
    delay(10); // Delay between commands to keep TP80 happy
    flushS1();
    for (int j = 5; j < 20; j++)
    {
      Telegram[j] = commands[1][j - 5];
    }
    ok = sendCommandTelegram();
    delay(10); // Delay between commands to keep TP80 happy
    flushS1();
    return ok;
  }
  else
  {
    for (int j = 5; j < 20; j++)
    {
      Telegram[j] = commands[2][j - 5];
    }
    bool ok = sendCommandTelegram();
    delay(10); // Delay between commands to keep TP80 happy
    flushS1();
    return ok;
  }
}
boolean pfeifferTurboSetPower(byte percent)
{
  dataArray[0] = (int)(percent / 100);
  dataArray[1] = (int)((percent - (dataArray[0] * 100)) / 10);
  dataArray[2] = percent - (dataArray[0] * 100) - (dataArray[1] * 10);
  dataArray[0] = dataArray[0] + 48;
  dataArray[1] = dataArray[1] + 48;
  dataArray[2] = dataArray[2] + 48;
  buildTelegram(1, 708, 3, dataArray);

  bool ok = sendCommandTelegram();
  delay(20); // Delay between commands to keep TP80 happy
  flushS1();
  if (ok)
  {
    turboPowerSetting = percent;
    return true;
  }
  return false;

} // use parameter 708 for %
boolean pfeifferTurboSetSpeed(byte percent)
{

  dataArray[0] = '0';
  dataArray[1] = '0';
  dataArray[2] = '1';
  buildTelegram(1, 26, 3, dataArray);
  bool ok = sendCommandTelegram();
  delay(20); // Delay between commands to keep TP80 happy
  flushS1();
  dataArray[0] = 0;
  dataArray[4] = 0;
  dataArray[5] = 0;
  dataArray[1] = (int)(percent / 100);
  dataArray[2] = (int)((percent - (dataArray[1] * 100)) / 10);
  dataArray[3] = percent - (dataArray[1] * 100) - (dataArray[2] * 10);

  for (int i = 0; i < 6; i++)
  {
    dataArray[i] = dataArray[i] + 48;
  }
  buildTelegram(1, 707, 6, dataArray);
  ok = sendCommandTelegram();
  delay(20); // Delay between commands to keep TP80 happy
  flushS1();
  return ok;

} // use parameter 707 for the value in %, and 026 to 1. Read parameters 397 for confirmation
boolean pfeifferTurboGetError()
{
  // error code reade 303, datatype is 4 (6 char string)
  // use parameter 009 for error acknoledgement and retrieve errors

  // get current error
  dataArray[0] = '=';
  dataArray[1] = '?';
  buildTelegram(0, 303, 2, dataArray);
  sendRequestTelegram(turboError, 6);
  delay(20); // Delay between commands to keep TP80 happy
  flushS1();
  if (turboError[5] != '0')
  { // for now we assume that no error will output Err000
    Serial.print(F("M TurboError "));
    Serial.print(turboError);
    Serial.write('\r');
    turboInError = true;
    return true;
  }
  turboInError = false;
  return false;
}
void acknowledgeError()
{
  dataArray[0] = '1';
  dataArray[1] = '1';
  dataArray[2] = '1';
  dataArray[3] = '1';
  dataArray[4] = '1';
  dataArray[5] = '1';
  buildTelegram(1, 9, 6, dataArray);
  bool ok = sendCommandTelegram();
  delay(20); // Delay between commands to keep TP80 happy
  flushS1();
  if (!ok)
  {
    Serial.println(F("M TurboError NoA"));
  }
}
boolean pfeifferTurboGetInfo()
{
  /*
    overtemp for drive 304, datatype is 0 (6 boolean), 0 for no
    overtemp for pump 305
    310 for drive current, type 2
    313 for drive voltage, type 2
    326 for electronice temp, type 1
    330 ofr temp pump bottom part, type 1
    324 for power stage temp, type 1
    346 for motor temp, type 1
    397 for setSpeed, type 1
    398 for actual speed, type 1
  */
  resetTelegram();
  Telegram[0] = '0';
  Telegram[1] = '0';
  Telegram[2] = '1';
  Telegram[3] = '0';
  Telegram[4] = '0';
  for (int i = 0; i < 10; i++)
  {
    for (int j = 5; j < 16; j++)
    {
      Telegram[j] = getInfo[i][j - 5];
    }
    turboInfo[i] = sendRequestTelegram();
  }
  turboInfo[0] = turboInfo[0] / 10;
  flushS1();
  return true;
}

boolean buildTelegram(byte action, int paraNumber, int dataLength, char data[16])
{
  if (12 + dataLength > 29)
    return false;
  // action is 0 for request, 1 for command
  // paraNumber is for the parameter number
  // data lenght is the actual number of char in data
  //  Adress
  resetTelegram();
  Telegram[0] = '0';
  Telegram[1] = '0';
  Telegram[2] = '1';

  // action
  if (action == 1)
    Telegram[3] = '1';
  else
    Telegram[3] = '0';
  Telegram[4] = '0';
  // parameter number. must be 3 characters
  int j = int(paraNumber / 100);
  Telegram[5] = j;
  j = int((paraNumber - (j * 100)) / 10);
  Telegram[6] = j;
  j = paraNumber - Telegram[5] * 100 - Telegram[6] * 10;
  Telegram[7] = j + 48;
  Telegram[6] = Telegram[6] + 48;
  Telegram[5] = Telegram[5] + 48;
  // data lenght. if below 10, then add a zero
  Telegram[8] = int(dataLength / 10) + 48;
  Telegram[9] = dataLength - int(dataLength / 10) + 48;
  for (int i = 0; i < dataLength; i++)
  {
    Telegram[10 + i] = data[i];
  }
  int modulo = 0;
  // Serial.print("modulo addition ");//ref
  for (int i = 0; i < 10 + dataLength; i++)
  {
    // Serial.print(modulo); Serial.print(" ");//ref
    modulo = modulo + Telegram[i];
  }
  // Serial.print("modulo created "); Serial.print(modulo);Serial.print("  ");//ref
  modulo = modulo % 256;
  // Serial.println(modulo); //ref

  Telegram[10 + dataLength] = int(modulo / 100);
  Telegram[11 + dataLength] = int((modulo - Telegram[10 + dataLength] * 100) / 10);

  Telegram[12 + dataLength] = (modulo - Telegram[10 + dataLength] * 100 - Telegram[11 + dataLength] * 10);

  Telegram[10 + dataLength] = Telegram[10 + dataLength] + 48;
  Telegram[11 + dataLength] = Telegram[11 + dataLength] + 48;
  Telegram[12 + dataLength] = Telegram[12 + dataLength] + 48;

  return true;
}
void resetTelegram()
{
  for (int i = 0; i < 30; i++)
  {
    Telegram[i] = '\r';
  }
}
boolean sendCommandTelegram()
{
  int i = 0;
  resetWDT();
  while (Telegram[i] != '\r')
  {
    // Serial.write(Telegram[i]);//ref
    Serial1.write(Telegram[i]);
    i++;
  }
  // Serial.write('\r');//ref
  Serial1.write('\r');
  // check response
  // if it is a command, it should receive exactly the same
  // if it is a data request, you get a data back
  // here is command, so should get the same back

  // Getting the answer
  Serial1.setTimeout(1500);
  i = Serial1.readBytesUntil('\r', TeleResponse, 30);
  resetWDT();
  if (i > 10)
  { // get into here if the response get a minimum number of char
    // get data lenght
    i = (int)(getDataLength(TeleResponse));
    //  Serial.print("received ");Serial.println(i);
    // int x=0;
    // while(true){Serial.print(TeleResponse[x]);if(TeleResponse[x++]=='\r')break;}//ref;if//ref
    if (i > 0)
    {
      // check if length is same as sent
      if (getDataLength(Telegram) == getDataLength(TeleResponse))
      {
        // if same lenght, check data are the same
        boolean same = true;
        for (i = 0; i < getDataLength(TeleResponse); i++)
        {
          same = same && (Telegram[i + 10] == TeleResponse[i + 10]);
        }
        if (same)
          return same;
      }
      // if not get send error. we only get here if the data wasn-t the same

      Serial.print(F("LOG Error turbo "));
      for (i = 0; i < getDataLength(TeleResponse); i++)
      {
        Serial.print(TeleResponse[i + 10]);
      }
      Serial.write('\r');
      return false;
    }
    else
      return false;
  }
  else
    return false;
}
unsigned long sendRequestTelegram()
{
  // REF
  // Serial.println("sending request to turbo");
  int i = 0;
  resetWDT();
  while (Telegram[i] != '\r')
  {
    // Serial.print(Telegram[i]);Serial.print(" ");//ref
    Serial1.write(Telegram[i]);
    i++;
  }
  // Serial.write('\r');//ref
  Serial1.write('\r');
  // check response
  // if it is a command, it should receive exactly the same
  // if it is a data request, you get a data back
  // here is request, so should get data

  // Getting the answer

  i = Serial1.readBytesUntil('\r', TeleResponse, 30);
  // Serial.println(TeleResponse);
  resetWDT();
  if (i > 10)
  { // get into here if the response get a minimum number of char
    // get data lenght
    // Serial.print("received ");Serial.println(i);//ref

    i = (int)(getDataLength(TeleResponse));
    if (i > 0)
    {
      // int x=0;
      // while(true){Serial.print(TeleResponse[x]);if(TeleResponse[x++]=='\r')break;}//ref;if//ref
      // retrieve the data
      char tmp[i + 1];
      tmp[i] = '\0';
      for (i = 0; i < getDataLength(TeleResponse); i++)
      {
        tmp[i] = TeleResponse[i + 10];
      }
      return (unsigned long)(atol(tmp));
    }
    else
      return 0;
  }
  else
    return 0;
}

void sendRequestTelegram(char dataTmp[], byte dataL)
{ // pass a buffer to get the data in, and the length (6 or 16)
  // REF
  // Serial.println("sending request to turbo");
  int i = 0;
  resetWDT();
  while (Telegram[i] != '\r')
  {
    // Serial.write(Telegram[i]);Serial.print(" ");//ref
    Serial1.write(Telegram[i]);
    i++;
  }
  // Serial.print('\r');//ref
  Serial1.write('\r');
  // check response
  // if it is a command, it should receive exactly the same
  // if it is a data request, you get a data back
  // here is request, so should get data

  // Getting the answer
  i = Serial1.readBytesUntil('\r', TeleResponse, 30);
  resetWDT();
  if (i > 10)
  { // get into here if the response get a minimum number of char
    // get data lenght
    // Serial.print("received ");Serial.println(i);
    // int x=0;
    // while(true){Serial.print(TeleResponse[x]);if(TeleResponse[x++]=='\r')break;}//ref;if//ref

    i = (int)(getDataLength(TeleResponse));
    if (i > 0)
    {
      // retrieve the data
      if (i > dataL)
        i = dataL;
      for (int j = 0; j < i; j++)
      {
        dataTmp[j] = TeleResponse[j + 10];
      }
    }
    else
      dataTmp[0] = '\0';
  }
  else
    dataTmp[0] = '\0';
}

byte getDataLength(char tele[])
{
  return (10 * (tele[8] - 48) + (tele[9] - 48));
}

int freeRam()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}
//---------------------------------------------------------------------------------------------------------------POWER CONTROL------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// power to the RGA and turbo pump are always on in the ifremer 2026 model
char PowerStates[10] = {'1', '1', '0', '0', '0', '0', '0', '0', '0', '0'}; // as follow, 0= Turbo,  1 RGA, 2 Roughing1, 3 Roughing2, 4-7 Fan 1-4, 8 Cryo Cooler, 9 FluidPump

boolean powerMain()
{
  toSendMain = ok;
  if (receivedMain[0] == 'T')
  {
    if (receivedMain[1] == 'M' && receivedMain[2] == 'P')
    {
      if (receivedMain[4] == 'N')
      {
        powerTurbo(true);
        communicate(0);
        return true;
      }
      else if (receivedMain[4] == 'F')
      {
        powerTurbo(false);
        communicate(0);
        return true;
      }
    }
  }
  else if (receivedMain[0] == 'R' && receivedMain[1] == 'O' && receivedMain[2] == 'U' && receivedMain[3] == 'G' && receivedMain[4] == 'H')
  {
    if (receivedMain[6] == 'N')
    {
      powerRoughing(true);
      communicate(0);
      return true;
    }
    else if (receivedMain[6] == 'F')
    {
      powerRoughing(false);
      communicate(0);
      return true;
    }
  }
  else if (receivedMain[0] == 'R' && receivedMain[1] == 'G' && receivedMain[2] == 'A')
  {
    if (receivedMain[4] == 'N')
    {
      powerRGA(true);
      communicate(0);
      return true;
    }
    else if (receivedMain[4] == 'F')
    {
      powerRGA(false);
      communicate(0);
      return true;
    }
  }
  else if (receivedMain[0] == 'C' && receivedMain[1] == 'R' && receivedMain[2] == 'Y' && receivedMain[3] == 'O')
  {
    if (receivedMain[5] == 'N')
    {
      powerCryo(true);
      communicate(0);
      return true;
    }
    else if (receivedMain[5] == 'F')
    {
      powerCryo(false);
      communicate(0);
      return true;
    }
  }
  else if (receivedMain[0] == 'F' && receivedMain[1] == 'L' && receivedMain[2] == 'U' && receivedMain[3] == 'I' && receivedMain[4] == 'D')
  {
    if (receivedMain[6] == 'N')
    {
      powerFluidPump(true);
      communicate(0);
      return true;
    }
    else if (receivedMain[6] == 'F')
    {
      powerFluidPump(false);
      communicate(0);
      return true;
    }
  }
  return false;
}

void powerRoughing(boolean on)
{
  toggle(PwrRoughing1, on);
  if (on)
    PowerStates[2] = '1';
  else
    PowerStates[2] = '0';
  RoughingOn = on;
}

void powerTurbo(boolean on)
{
  //  2026 Ifremer design: Only enables/Disables querying the status of the turbo pump and does not actually disable or enable power to it.
  // toggle(PwrTurbo, on);
  // if (on)
  // {
  //   PowerStates[0] = '1';
  // }
  // else
  // {
  //   PowerStates[0] = '0';
  // };
  tmpon = on;
}

void powerRGA(boolean on)
{
  // not applicable to this model (RGA is always on)
  // toggle(PwrRGA, on);
  // if (on)
  //   PowerStates[1] = '1';
  // else
  //   PowerStates[1] = '0';
}

void powerCryo(boolean on)
{
  toggle(PwrCryo, on);
  if (on)
    PowerStates[8] = '1';
  else
    PowerStates[8] = '0';
}
void powerFluidPump(boolean on)
{
  toggle(PwrFluidPump, on);
  if (on)
    PowerStates[9] = '1';
  else
    PowerStates[9] = '0';
}
void toggle(int pin, boolean ON)
{
  if (ON)
  {
    digitalWrite(pin, HIGH);
  }
  else
  {
    digitalWrite(pin, LOW);
  }
}

//---------------------------------------------------------------------------------------------------------------Com receving------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void serialEvent()
{ // is accessed when receveid serial data on main
  // Read it
  resetReceivedMain();
  Serial.readBytesUntil(term, receivedMain, 30);

  if (!(receivedMain[0] == 'U' && receivedMain[1] == ' '))
  {
    reportComError(0);
    return;
  }
  else if (!interpreteMain())
  {
    toSendMain = "EN";
    communicate(0);
  }
}

void resetReceivedMain()
{
  for (int i = 0; i < 20; i++)
  {
    receivedMain[i] = '\0';
  }
}
void reportComError(int serial)
{
  switch (serial)
  {
  case 0:
    toSendMain = "EC";
    communicate(serial);
    break;
  }
}
void trimReceived()
{
  for (int i = 0; i < 17; i++)
  {
    receivedMain[i] = receivedMain[i + 3];
  }
  receivedMain[17] = '\0';
  receivedMain[18] = '\0';
  receivedMain[19] = '\0';
}
boolean interpreteMain()
{ // general switching for main incoming messages
  // the command should start with P, S, B, T or ?
  switch (receivedMain[2])
  {
  case '?':
    trimReceived();
    return queryMain();
    break;
  case 'P':
    trimReceived();
    return powerMain();
    break;
  case 'S':
    trimReceived();
    return setMain();
    break;
  case 'B':
    return MainBeat();
    break;
  case 'T':
    trimReceived();
    return pfeifferTurboMain();
    break;
  default:
    return false;
  }
}

boolean queryMain()
{
  /*
      I is ID
      F is firmware
      D is for a data telegram
      2 is to call an I2C scan
      R is to get current free ram
  */
  // 3.0 if(receivedMain.substring(1,3)=="ID"){
  if (receivedMain[0] == 'I')
  {
    toSendMain = String(ID);
    communicate(0);
    return true;
  }
  // 3.0 if(receivedMain.substring(1,5)=="FIRM"){
  else if (receivedMain[0] == 'F')
  {
    toSendMain = String(firmware);
    communicate(0);
    return true;
  }

  // 3.0 if(receivedMain.substring(1,5)=="DATA"){
  else if (receivedMain[0] == 'D')
  {
    data();
    return true;
  }
  else if (receivedMain[0] == '2')
  {
    i2cScan();
    return true;
  }
  else if (receivedMain[0] == 'R')
  {
    toSendMain = String(freeRam());
    communicate(0);
    return true;
  }
  return false;
}

boolean setMain()
{
  if (receivedMain[0] == 'R' && receivedMain[1] == 'E')
  {
    if (receivedMain[2] == 'S' && receivedMain[3] == 'E' && receivedMain[4] == 'T' && receivedMain[5] == 'C' && receivedMain[6] == 'O' && receivedMain[7] == 'M' && receivedMain[8] == '1')
    {
      Serial1.end();
      delay(100);
      Serial1.begin(comSpeedTurbo);
      toSendMain = "C1Reset";
      communicate(0);
      return true;
    }
  }
  else if (receivedMain[0] == 'R' && receivedMain[1] == 'A')
  {
    if (receivedMain[2] == 'T' && receivedMain[3] == 'E')
    {

      unsigned long r = (receivedMain[5] - 48) * 1000000 + (receivedMain[6] - 48) * 100000 + (receivedMain[7] - 48) * 10000 + (receivedMain[8] - 48) * 1000 + (receivedMain[9] - 48) * 100 + (receivedMain[10] - 48) * 10 + (receivedMain[11] - 48);
      switch (receivedMain[4])
      {
      case 'T':
        TurboStatusReadingInterval = r;
        break;
      case 'D':
        DataBroadcastingInterval = r;
        break;
      default:
        return false;
      }
      toSendMain = ok;
      communicate(0);
      return true;
    }
  }
  else if (receivedMain[0] == 'D' && receivedMain[1] == 'B')
  {
    if (receivedMain[3] == 'N')
    {
      dataBroadcasting = true;
      toSendMain = ok;
      communicate(0);
      return true;
    }
    else if (receivedMain[3] == 'F')
    {
      dataBroadcasting = false;
      toSendMain = ok;
      communicate(0);
      return true;
    }
  }
  else if (receivedMain[0] == 'W' && receivedMain[1] == 'D')
  {
    if (receivedMain[3] == 'N')
    {
      wdtStart();
      toSendMain = ok;
      communicate(0);
      return true;
    }
    else if (receivedMain[3] == 'F')
    {
      wdtStop();
      toSendMain = ok;
      communicate(0);
      return true;
    }
  }
  else if (receivedMain[0] == 'P' && receivedMain[1] == 'R')
  {
    if (receivedMain[2] == 'A' && receivedMain[3] == 'T')
    {

      y = ((receivedMain[5] - 48) * 100) + ((receivedMain[6] - 48) * 10) + (receivedMain[7] - 48);
      z = round((y / 100) * 255);
      analogWrite(pumpratepin, z);
      toSendMain = ok;
      communicate(0);
      return true;
    }
  }
  else if (receivedMain[0] == 'A' && receivedMain[1] == 'U')
  {
    if (receivedMain[2] == 'T' && receivedMain[3] == 'O')
    {
      initAuto();
      toSendMain = ok;
      communicate(0);
      return true;
    }
  }
  return false;
}

//---------------------------------------------------------------------------------------------------------------DATA BROADCAST------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
  xxx int InternalSensors[8]={-1,-1,-1,-1,-1,-1,-1,-1}; // 0 is pressure, 1 is board temp, 2 is nafion temp 1, 3 is nafion temp 2, 4 is nafion temp 3, 5 is position x, 6 is position y, 7 is position z)
  xxx float SensorsValues[1]={-1};// 0 is cryo
  xxx char turboError[7];
  xxx! boolean turboInError=false;
  xxx unsigned long turboInfo[10]={0,0,0,0,0,0,0,0,0,0};  // 0 is current speed, 1 is overtemp drive, 2 is overtemp pump, 3 is drive current, 4 is drive voltage, 5 is eltemp, 6 is temp bottom pump, 7 is power stage temp, 8 is motor temp, 9 is set speed
  xxx! byte turboPowerSetting;
  xxx! boolean tmpCtrl=true;
  xxx! boolean tmpon=false; // change when power is applied
  xxx char PowerStates[10]={'0','0','0','0','0','0','0','0','0','0'};   //as follow, 0= Turbo,  1 RGA, 2 Roughing1, 3 Roughing2, 4-7 Fan 1-4, 8 cryo, 9 fluidpump
  xxx char SolenoidStates[4]={'2','2','2','2'};

  int ExtraValues[4]={0,0,0,0};  // 0 is heater mode, 1 is heater 1 status, 2 is heater 2 status, 3 is heating time elapsed in seconds.


*/
void data()
{
  Serial.print(F("DATA,at"));
  Serial.print(millis());
  Serial.print(F(",TMP"));
  for (int i = 0; i < 11; i++)
  { // 1 is speed, 2 is amp, 3 is T1, 4 is T2
    Serial.print(F(":"));
    if (i == 10)
    {
      Serial.print(turboError);
    }
    else
      Serial.print(turboInfo[i]);
  }

  Serial.print(F(",POWST"));
  for (int i = 0; i < 10; i++)
  { // POW: Turbo:RGA: Roughing1: Roughing2: Fan 1-4:Aux24: cryo: fluidpump: heat: aux12:cal,
    Serial.print(F(":"));
    Serial.print(PowerStates[i]);
  }
  Serial.print(F(",ENDDATA"));
  Serial.print(term);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////LOOP////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
  resetWDT();

  // beat timing
  if ((lastBeat > millis()) || (millis() - lastBeat) > BeatInterval)
  {
    lastBeat = millis();
  }

  // beat and standby management
  if (initialized == false)
  { // checking for receiving the BEAT after start up
    if ((millis() > (waitingForBeat)))
    {
      // then turn into default mode
      defaulted = true;
      wdtStop();
      // run the methods to initiate automatic startup
      initAuto();
    }
  }
  if (defaulted)
  {
    if ((lastDefaultBroadcast > millis()) || (millis() - lastDefaultBroadcast) > defaultBroadcastDelay)
    {
      // signal its presence
      lastDefaultBroadcast = millis();
      initialized = true;
    }
  }

  if (tmpCtrl)
  {
    if (tmpon && ((lastTStatRead > millis()) || (millis() > (lastTStatRead + TurboStatusReadingInterval))))
    {
      getPfeifferTurboStatus();
      lastTStatRead = millis();
    }
  }
  // data broadcasting
  if (dataBroadcasting && ((lastBroadcast > millis()) || (millis() - lastBroadcast) > DataBroadcastingInterval))
  {
    data();
    lastBroadcast = millis();
  }
}

void communicate(int serial)
{ // send the receivedMainToSend after proper formating,

  switch (serial)
  {
  case 0:
    toSendMain = "M " + toSendMain;
    Serial.print(toSendMain);
    Serial.print(term);
    return;
  case 1:
    toSend = "#000" + toSend;
    Serial1.print(toSend);
    Serial1.println("");
    return;
  default:
    return;
  }
}

byte i2cScan()
{ // scans the I2C bus
  byte error_, address, nDevices;
  resetWDT();
  Serial.print(F("LOG Scanning I2C"));
  Serial.write(term);
  nDevices = 0;

  for (address = 10; address < 127; address++)
  {

    Wire.beginTransmission(address);
    error_ = Wire.endTransmission();

    if (error_ == 0)
    {
      i2cAddresses[nDevices++] = address;
      Serial.print(F("LOG Device found at address Ox"));
      Serial.print(address, HEX);
      Serial.write(term);
    }

    if (nDevices == 10)
      return nDevices;
  }

  return nDevices;
}

//----------------AutoRun Sequence------------

void initAuto()
{
  wdtStop();
  defaulted = true;
  DataBroadcastingInterval = 30000;
  // sends power instructions
  Serial.println(F("M Entering Autostart Protocol"));
  delay(5000);
  powerRoughing(true);
  Serial.println(F("Auto: Initializing Roughing Pump"));
  delay(120000);
  powerTurbo(true);
  delay(5000);
  pfeifferTurboStart(true);
  Serial.println(F("Auto: Initializing Turbo Pump"));
  delay(30000);
  powerCryo(true);
  Serial.println(F("Auto: Initializing Cryo Cooler"));
  delay(5000);
  powerFluidPump(true);
  Serial.println(F("Auto: Initializing Fluid Pump"));
  delay(5000);
  analogWrite(pumpratepin, z); // setting the pumping rate to the previously set percent, adjust the value of y accordingly (in declarations)
  Serial.println(F("Auto: Setting Pumping Rate"));
  delay(2000);
  return;
}
