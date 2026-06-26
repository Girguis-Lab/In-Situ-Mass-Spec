
#ifndef PFEIFFER_VAC_PROTOCOL_INCLUDE_DEBUG
#define PFEIFFER_VAC_PROTOCOL_INCLUDE_DEBUG 1 // Enable debug features by default (uses more rom memory)
#endif

#ifndef PFEIFFER_PROTOCOL_CONSTS_H
#define PFEIFFER_PROTOCOL_CONSTS_H

#include <Arduino.h>
#include "PfeifferVacProtocolDatatypes.h"

namespace PfeifferVacProtocol
{
    enum class TelegramError
    {
        None,
        InvalidParameter, // sent as "NO_DEF", Parameter no longer used or invalid
        OutOfRange,       // sent as "_RANGE", Parameter value out of range
        LogicError,       // sent as "_LOGIC", Logical access error
        InvalidChecksum,  // Checksum does not match the calculated checksum of the telegram
    };

    enum class Action
    {
        Query = '0',  // Query action
        Command = '1' // Command action
    };

    // ---- Function Enums ----

    // For Heating, Stand-by, etc.: On/Off state
    enum class FuncOnOff
    {
        Off = 0, // Off (0)
        On = 1   // On (1)
    };

    // For EnableVent, Yes/No type
    enum class FuncYesNo
    {
        No = 0, // No (0)
        Yes = 1 // Yes (1)
    };

    // For ErrorAckn: Malfunction acknowledgement
    enum class FuncMalfunctionAckn
    {
        // Only value 1 is valid
        Acknowledge = 1 // Malfunction acknowledgement (1)
    };

    // For PumpgStatn: Pumping station state
    enum class FuncPumpingStation
    {
        Off = 0,      // Off (0)
        OnAndAckn = 1 // On and malfunction acknowledgement (1)
    };

    // For CfgSpdSwPt: Rotation speed switchpoint configuration
    enum class FuncRotationSwitchpointCfg
    {
        Switchpoint1 = 0,     // Rotation speed switchpoint 1 (0)
        Switchpoints1And2 = 1 // Rotation speed switchpoints 1 & 2 (1)
    };

    // For CfgDO2, CfgDO1: Output DO2/DO1 configuration
    enum class FuncOutputDO2Cfg
    {
        RotationSpeedSwitchpointReached = 0, // Rotation speed switchpoint reached (0)
        NoError = 1,                         // No error (1)
        Error = 2,                           // Error (2)
        Warning = 3,                         // Warning (3)
        ErrorAndOrWarning = 4,               // Error and/or warning (4)
        SetRotationSpeedReached = 5,         // Set rotation speed reached (5)
        PumpOn = 6,                          // Pump on (6)
        PumpAccelerating = 7,                // Pump accelerating (7)
        PumpDecelerating = 8,                // Pump decelerating (8)
        Always0 = 9,                         // Always "0" (9)
        Always1 = 10,                        // Always "1" (10)
        RemotePriorityActive = 11,           // Remote priority active (11)
        Heating = 12,                        // Heating (12)
        BackingPump = 13,                    // Backing pump (13)
        SealingGas = 14,                     // Sealing gas (14)
        PumpingStation = 15,                 // Pumping station (15)
        PumpRotating = 16,                   // Pump rotating (16)
        PumpHasStopped = 17,                 // Pump has stopped (17)
        PressureSwitchPoint1NotReached = 19, // Pressure switch point 1 not reached (19)
        PressureSwitchPoint2NotReached = 20, // Pressure switch point 2 not reached (20)
        ForeVacuumValveDelayed = 21,         // Fore-vacuum valve, delayed (21)
        BackingPumpStandby = 22              // Backing pump standby (22)
    };
    typedef FuncOutputDO2Cfg FuncOutputDO1Cfg; // Same as OutputDO2Cfg

    // For OpMode BKP: Backing pump operating mode
    enum class FuncBackingPumpOpMode
    {
        Continuous = 0,     // Continuous operation (0)
        Intermittent = 1,   // Intermittent operation (1)
        DelayedOn = 2,      // Delayed switching on (2)
        DelayedInterval = 3 // Delayed interval operation (3)
    };

    // For GasMode: Gas mode
    enum class FuncGasMode
    {
        HeavyGases = 0, // Heavy gases (0)
        LightGases = 1, // Light gases (1)
        Helium = 2      // Helium (2)
    };

    // For VentMode: Venting mode
    enum class FuncVentMode
    {
        DelayedVenting = 0, // Delayed venting (0)
        NoVenting = 1,      // No venting (1)
        DirectVenting = 2   // Direct venting (2)
    };

    // For CfgAccA1, CfgAccB1, CfgAccC1, CfgAccD1: Accessory connection configuration
    enum class FuncAccA1Cfg
    {
        FanContinuous = 0,            // Fan (continuous operation) (0)
        VentValveClosedNoCurrent = 1, // Venting valve, closed without current (1)
        Heating = 2,                  // Heating (2)
        BackingPump = 3,              // Backing pump (3)
        FanTempControlled = 4,        // Fan (temperature controlled) (4)
        SealingGas = 5,               // Sealing gas (5)
        Always0 = 6,                  // Always "0" (6)
        Always1 = 7,                  // Always "1" (7)
        PowerFailureVentingUnit = 8,  // Power failure venting unit (8)
        SecondVentValve = 12,         // Second venting valve (12)
        NoFunction = 13               // No function (13)
    };
    typedef FuncAccA1Cfg FuncAccB1Cfg; // Same as AccA1Cfg: Accessory connection configuration
    typedef FuncAccA1Cfg FuncAccC1Cfg; // Same as AccA1Cfg: Accessory connection configuration
    typedef FuncAccA1Cfg FuncAccD1Cfg; // Same as AccA1Cfg: Accessory connection configuration

    // For Press1HVen: Release HV sensor integrated
    enum class FuncPress1HVen
    {
        Off = 0,                           // Off (0)
        On = 1,                            // On (1)
        OnWithRotSpdSwitchpoint = 2,       // On, with rotation speed switch point reached (2)
        OnWithPressureSwitchNotReached = 3 // On, with pressure switch point not reached (3)
    };

    // For CfgAO1: Output AO1 configuration
    enum class FuncOutputAO1Cfg
    {
        ActualRotationSpeed = 0, // Actual rotation speed (0)
        Output = 1,              // Output (1)
        Current = 2,             // Current (2)
        Always0V = 3,            // Always 0V (3)
        Always10V = 4,           // Always 10V (4)
        PressureValue1 = 6,      // Pressure value 1 (6)
        PressureValue2 = 7,      // Pressure value 2 (7)
        ForeVacuumControl = 8    // Fore-vacuum control (8)
    };

    // For TmpMgtMode: Temperature management configuration
    enum class FuncTmpMgtMode
    {
        PumpTempLEQ60C = 0,     // Pump housing temperature ≤60C (0)
        PumpTempLEQ80C = 1,     // Pump housing temperature ≤80C (1)
        PowerCharacteristic = 2 // Power characteristic [P:027] (2)
    };

    // For CtrlVialnt: Operate via interface
    enum class FuncCtrlVialnt
    {
        Remote = 1,                    // Remote (1)
        RS485 = 2,                     // RS-485 (2)
        PVCAN = 4,                     // PV.can (4)
        UnlockInterfaceSelection = 255 // Unlock interface selection (255)
    };

    // For CfgDI1, CfgD12: Input DI1/DI2 configuration
    enum class FuncDI1Cfg
    {
        Deactivated = 0,              // Deactivated (0)
        EnableVenting = 1,            // Enable venting (1)
        Heating = 2,                  // Heating (2)
        SealingGas = 3,               // Sealing gas (3)
        RunUpTimeMonitoring = 4,      // Run-up time monitoring (4)
        RotationSpeedSettingMode = 5, // Rotation speed setting mode (5)
        Motor = 6,                    // Motor (6)
        EnableHVSensor1 = 7           // Enable HV sensor 1 (7)
    };
    typedef FuncDI1Cfg FuncDI2Cfg; // Same as DI1Cfg

    // Control command constants from table 6.2: Control Commands in manual - Each const is commented for intellisense
    namespace ControlCommand
    {
        constexpr int Heating = 1;     // Heating: 0=off 1=on | boolean_old | RW | min=0 max=1 default=0
        constexpr int StandBy = 2;     // Stand-by: 0=off 1=on | boolean_old | RW | min=0 max=1 default=0
        constexpr int RUTimeCtrl = 4;  // Run-up time monitoring: 0=off 1=on | boolean_old | RW | min=0 max=1 default=1
        constexpr int ErrorAckn = 9;   // Malfunction acknowledgement: 1=acknowledge | boolean_old | W | min=1 max=1
        constexpr int PumpgStatn = 10; // Pumping station: 0=off 1=on and malfunction acknowledgment | boolean_old | RW | min=0 max=1 default=0
        constexpr int EnableVent = 12; // Enable venting: 0=no 1=yes | boolean_old | RW | min=0 max=1 default=0
        constexpr int CfgSpdSwPt = 17; // Rotation speed switchpoint configuration: 0=Switchpoint1 1=Switchpoints1And2 | u_short_int | RW | min=0 max=1 default=0
        constexpr int CfgDO2 = 19;     // Output DO2 configuration: see OutputDO2Cfg | u_short_int | RW | min=0 max=22 default=1
        constexpr int MotorPump = 23;  // Motor pump: 0=off 1=on | boolean_old | RW | min=0 max=1 default=1
        constexpr int CfgDO1 = 24;     // Output DO1 configuration: see OutputDO1Cfg | u_short_int | RW | min=0 max=22 default=0
        constexpr int OpModeBKP = 25;  // Backing pump operating mode: see BackingPumpOpMode | u_short_int | RW | min=0 max=3 default=0
        constexpr int SpdSetMode = 26; // Rotation speed setting mode: 0=off 1=on | u_short_int | RW | min=0 max=1 default=0
        constexpr int GasMode = 27;    // Gas mode: see GasMode | u_short_int | RW | min=0 max=2 default=0
        constexpr int VentMode = 30;   // Venting mode: see VentMode | u_short_int | RW | min=0 max=2 default=2
        constexpr int CfgAccA1 = 35;   // Configuration accessory connection A1: see AccA1Cfg | u_short_int | RW | min=0 max=13 default=0
        constexpr int CfgAccB1 = 36;   // Configuration accessory connection B1: see AccB1Cfg | u_short_int | RW | min=0 max=13 default=1
        constexpr int Press1HVen = 41; // Release HV sensor integrated: see Press1HVen | u_short_int | RW | min=0 max=3 default=2
        constexpr int SealingGas = 50; // Sealing gas: 0=off 1=on | boolean_old | RW | min=0 max=1 default=0
        constexpr int CfgAO1 = 55;     // Output AO1 configuration: see OutputAO1Cfg | u_short_int | RW | min=0 max=8 default=0
        constexpr int TmpMgtMode = 58; // Temperature management configuration: see TmpMgtMode | u_short_int | RW | min=0 max=2 default=0
        constexpr int CtrlVialnt = 60; // Operate via interface: see CtrlVialnt | u_short_int | RW | min=1 max=255 default=1
        constexpr int IntSelLckd = 61; // Interface selection locked: 0=off 1=on | boolean_old | RW | min=0 max=1 default=0
        constexpr int CfgDI1 = 62;     // Input DI1 configuration: see DI1Cfg | u_short_int | RW | min=0 max=7 default=1
        constexpr int CfgD12 = 63;     // Input D12 configuration: see D12Cfg | u_short_int | RW | min=0 max=7 default=2
        constexpr int CfgAccC1 = 68;   // Configuration accessory connection C1: see AccC1Cfg | u_short_int | RW | min=0 max=13 default=0
        constexpr int CfgAccD1 = 69;   // Configuration accessory connection D1: see AccD1Cfg | u_short_int | RW | min=0 max=13 default=0
    } // namespace ControlCommand

    // Status request constants from table 6.3: Status Requests in manual - Each const is commented for intellisense
    namespace StatusRequest
    {
        constexpr int RemotePrio = 300;    // Remote priority: 0=no 1=yes | boolean_old | R | min=0 max=1
        constexpr int SpdSwPtAtt = 302;    // Rotation speed switchpoint reached: 0=no 1=yes | boolean_old | R | min=0 max=1
        constexpr int ErrorCode = 303;     // Error code | string6 | R
        constexpr int OvTempElec = 304;    // Excess temperature drive electronics: 0=no 1=yes | boolean_old | R | min=0 max=1
        constexpr int OvTempPump = 305;    // Excess temperature pump: 0=no 1=yes | boolean_old | R | min=0 max=1
        constexpr int SetSpdAtt = 306;     // Set rotation speed reached: 0=no 1=yes | boolean_old | R | min=0 max=1
        constexpr int PumpAccel = 307;     // Pump accelerating: 0=no 1=yes | boolean_old | R | min=0 max=1
        constexpr int SetRotSpdHz = 308;   // Set rotation speed (Hz) | u_integer | R | Hz | min=0 max=999999
        constexpr int ActualSpdHz = 309;   // Actual rotational speed (Hz) | u_integer | R | Hz | min=0 max=999999
        constexpr int DrvCurrent = 310;    // Drive current | u_real | R | A | min=0 max=9999.99
        constexpr int OpHrsPump = 311;     // Pump operating hours | u_integer | R | h | min=0 max=65535
        constexpr int FwVersion = 312;     // Drive electronics software version | string6 | R
        constexpr int DrvVoltage = 313;    // Drive voltage | u_real | R | V | min=0 max=9999.99
        constexpr int OpHrsElec = 314;     // Drive electronics operating hours | u_integer | R | h | min=0 max=65535
        constexpr int NominalSpdHz = 315;  // Nominal rotation speed (Hz) | u_integer | R | Hz | min=0 max=999999
        constexpr int DrvPower = 316;      // Drive power | u_integer | R | W | min=0 max=999999
        constexpr int PumpCycles = 319;    // Pump cycles | u_integer | R | min=0 max=65535
        constexpr int TmpPwrStg = 324;     // Temperature power stage | u_integer | R | °C | min=0 max=999999
        constexpr int TempElec = 326;      // Electronics temperature | u_integer | R | °C | min=0 max=999999
        constexpr int TempPmpBot = 330;    // Pump lower part temperature | u_integer | R | °C | min=0 max=999999
        constexpr int AccelDecel = 336;    // Acceleration/deceleration | u_integer | R | rpm/s | min=0 max=999999
        constexpr int ElecName = 349;      // Electronic drive unit designation | string6 | R
        constexpr int HWVersion = 354;     // Hardware version drive electronics | string6 | R
        constexpr int SerialNo = 355;      // Serial number | string16 | R
        constexpr int ErrHist1 = 360;      // Error code history, item 1 | string6 | R
        constexpr int ErrHist2 = 361;      // Error code history, item 2 | string6 | R
        constexpr int ErrHist3 = 362;      // Error code history, item 3 | string6 | R
        constexpr int ErrHist4 = 363;      // Error code history, item 4 | string6 | R
        constexpr int ErrHist5 = 364;      // Error code history, item 5 | string6 | R
        constexpr int ErrHist6 = 365;      // Error code history, item 6 | string6 | R
        constexpr int ErrHist7 = 366;      // Error code history, item 7 | string6 | R
        constexpr int ErrHist8 = 367;      // Error code history, item 8 | string6 | R
        constexpr int ErrHist9 = 368;      // Error code history, item 9 | string6 | R
        constexpr int ErrHist10 = 369;     // Error code history, item 10 | string6 | R
        constexpr int TempRotor = 384;     // Rotor temperature | u_integer | R | °C | min=0 max=999999
        constexpr int OrderCode = 388;     // Order number | string16 | R
        constexpr int AddID = 396;         // Pump ID | u_integer | R
        constexpr int SetRotSpdRpm = 397;  // Set rotation speed (rpm) | u_integer | R | rpm | min=0 max=999999
        constexpr int ActualSpdRpm = 398;  // Actual rotational speed (rpm) | u_integer | R | rpm | min=0 max=999999
        constexpr int NominalSpdRpm = 399; // Nominal rotation speed (rpm) | u_integer | R | rpm | min=0 max=999999
    } // namespace StatusRequest

    // Reference value input constants from table 6.4: Reference Value Inputs in manual - Each const is commented for intellisense
    namespace ReferenceValueInput
    {
        constexpr int RUTimeSVal = 700; // Set value run-up time | u_integer | RW | min=1 max=120 default=8
        constexpr int SpdSwPt1 = 701;   // Rotation speed switch point 1 | u_integer | RW | % | min=50 max=97 default=80
        constexpr int SpdSVal = 707;    // Set value in rotation speed setting mode | u_real | RW | % | min=20 max=100 default=65
        constexpr int PwrSVal = 708;    // Set value power consumption | u_short_int | RW | % | min=10 max=100 default=100
        constexpr int SwoffBKP = 710;   // Backing pump switch-off threshold for intermittent operation | u_integer | RW | W | min=0 max=1000 default=0
        constexpr int SwOnBKP = 711;    // Backing pump switch-on threshold for intermittent operation | u_integer | RW | W | min=0 max=1000 default=0
        constexpr int StdbySVal = 717;  // Rotation speed set value in stand-by operation | u_real | RW | % | min=20 max=100 default=66.7
        constexpr int SpdSwPt2 = 719;   // Rotation speed switch point 2 | u_integer | RW | % | min=5 max=97 default=20
        constexpr int VentSpd = 720;    // Venting at rotation speed, delayed venting | u_short_int | RW | % | min=40 max=98 default=50
        constexpr int VentTime = 721;   // Venting time, delayed venting | u_integer | RW | s | min=6 max=3600 default=3600
        constexpr int mxPwrOutTm = 726; // Max. time for the output voltage in power back-up mode | u_integer | RW | s | min=1 max=255 default=10
        constexpr int fanOnTemp = 728;  // Start-up temperature of the fan in temperature-controlled mode | u_integer | RW | °C | min=6 max=75 default=45
        constexpr int PrsSwPt1 = 730;   // Pressure switch point 1 | u_expo_new | RW | hPa | default=1000
        constexpr int PrsSwPt2 = 732;   // Pressure switch point 2 | u_expo_new | RW | hPa | default=1000
        constexpr int PwrOutVolt = 733; // Output voltage in power back-up mode | u_real | RW | V | min=20.50 max=26.50 default=23.00
        constexpr int PwrOutThrs = 734; // Power threshold from which the voltage is output from P733 | u_integer | RW | W | min=15 max=150 default=20
        constexpr int PrsSn1Name = 739; // Name sensor 1 | string6 | R
        constexpr int Pressure1 = 740;  // Pressure value 1 | u_expo_new | RW | hPa
        constexpr int PrsCorrPi1 = 742; // Correction factor 1 | u_real | RW | min=0.1 max=8.0 default=0
        constexpr int PrsSn2Name = 749; // Name sensor 2 | string6 | R
        constexpr int Pressure2 = 750;  // Pressure value 2 | u_expo_new | RW | hPa
        constexpr int PrsCorrPi2 = 752; // Correction factor 2 | u_real | RW | min=0.1 max=8.0 default=0
        constexpr int NomSpdConf = 777; // Confirmation of nominal rotation speed | u_integer | RW | Hz | min=0 max=1500 default=0
        constexpr int RS485Adr = 797;   // RS-485 Interface address | u_integer | RW | min=1 max=255 default=1
    } // namespace ReferenceValueInput

    // Additional info constants from table 6.5: Additional Parameters for Control Unit in manual - Each const is commented for intellisense
    namespace AdditionalParams
    {
        constexpr int Pressure = 340;    // Actual pressure value (ActiveLine) | u_short_int | R | hPa | min=1e-10 max=1e3
        constexpr int CtrName = 350;     // Control unit type | string6 | R
        constexpr int CtrSoftware = 351; // Control unit software version | string6 | R
        constexpr int GaugeType = 738;   // Type of pressure gauge | string6 | RW
        constexpr int ParamSet = 794;    // Parameter set: 0=Basic parameter set 1=Extended parameter set | u_short_int | RW | min=0 max=1 default=0
        constexpr int Servicelin = 795;  // Insert service line | u_short_int | RW | default=795
    } // namespace AdditionalParams

#if PFEIFFER_VAC_PROTOCOL_INCLUDE_DEBUG

    // ---- Debug helper functions ----

    // Helper function to lookup an index in a given array of flash string names, returning "Unknown" if out of range or empty
    inline String func_unknown_if(const __FlashStringHelper *const names[], size_t size, uint8_t idx, const char *func_name)
    {
        if (idx < size && names[idx] != nullptr)
        {
            // Copy from PROGMEM to RAM and return as String
            String result = String(reinterpret_cast<const __FlashStringHelper *>(names[idx]));
            if (result.length() > 0)
            {
                return result;
            }
        }
        char unknown_buf[32];
        snprintf(unknown_buf, sizeof(unknown_buf), "Unknown (%s:%u)", func_name, idx);
        return String(unknown_buf);
    }

    // Lookup functions for each enum type, returning a String (copying from PROGMEM to RAM) or "Unknown" if out of range or empty
    // Note: These functions are only included if PFEIFFER_VAC_PROTOCOL_INCLUDE_DEBUG is set to 1

    static const String lookupFuncOnOff(uint8_t idx)
    {
        static const __FlashStringHelper *names[] = {
            F("Off"),
            F("On")};
        return func_unknown_if(names, sizeof(names) / sizeof(names[0]), idx, "FuncOnOff");
    }

    static const String lookupFuncYesNo(uint8_t idx)
    {
        static const __FlashStringHelper *names[] = {
            F("No"),
            F("Yes")};
        return func_unknown_if(names, sizeof(names) / sizeof(names[0]), idx, "FuncYesNo");
    }

    static const String lookupFuncMalfunctionAckn(uint8_t idx)
    {
        static const __FlashStringHelper *names[] = {
            F("No acknowledge"),
            F("Acknowledge")};
        return func_unknown_if(names, sizeof(names) / sizeof(names[0]), idx, "FuncMalfunctionAckn");
    }

    static const String lookupFuncPumpingStation(uint8_t idx)
    {
        static const __FlashStringHelper *names[] = {
            F("Off"),
            F("On and malfunction acknowledgement")};
        return func_unknown_if(names, sizeof(names) / sizeof(names[0]), idx, "FuncPumpingStation");
    }

    static const String lookupFuncRotationSwitchpointCfg(uint8_t idx)
    {
        static const __FlashStringHelper *names[] = {
            F("Rotation speed switchpoint 1"),
            F("Rotation speed switchpoints 1 & 2")};
        return func_unknown_if(names, sizeof(names) / sizeof(names[0]), idx, "FuncRotationSwitchpointCfg");
    }

    static const String lookupFuncOutputDO2Cfg(uint8_t idx)
    {
        static const __FlashStringHelper *names[] = {
            F("Rotation speed switchpoint reached"),
            F("No error"),
            F("Error"),
            F("Warning"),
            F("Error and/or warning"),
            F("Set rotation speed reached"),
            F("Pump on"),
            F("Pump accelerating"),
            F("Pump decelerating"),
            F(""), // 9
            F("Always '0'"),
            F("Always '1'"),
            F("Remote priority active"),
            F("Heating"),
            F("Backing pump"),
            F("Sealing gas"),
            F("Pumping station"),
            F("Pump rotating"),
            F("Pump has stopped"),
            F(""), // 18
            F("Pressure switch point 1 not reached"),
            F("Pressure switch point 2 not reached"),
            F("Fore-vacuum valve, delayed"),
            F("Backing pump standby")};
        return func_unknown_if(names, sizeof(names) / sizeof(names[0]), idx, "FuncOutputDO2Cfg");
    }

    static const String lookupFuncBackingPumpOpMode(uint8_t idx)
    {
        static const __FlashStringHelper *names[] = {
            F("Continuous operation"),
            F("Intermittent operation"),
            F("Delayed switching on"),
            F("Delayed interval operation")};
        return func_unknown_if(names, sizeof(names) / sizeof(names[0]), idx, "FuncBackingPumpOpMode");
    }

    static const String lookupFuncGasMode(uint8_t idx)
    {
        static const __FlashStringHelper *names[] = {
            F("Heavy gases"),
            F("Light gases"),
            F("Helium")};
        return func_unknown_if(names, sizeof(names) / sizeof(names[0]), idx, "FuncGasMode");
    }

    static const String lookupFuncVentMode(uint8_t idx)
    {
        static const __FlashStringHelper *names[] = {
            F("Delayed venting"),
            F("No venting"),
            F("Direct venting")};
        return func_unknown_if(names, sizeof(names) / sizeof(names[0]), idx, "FuncVentMode");
    }

    static const String lookupFuncAccA1Cfg(uint8_t idx)
    {
        static const __FlashStringHelper *names[] = {
            F("Fan (continuous operation)"),
            F("Venting valve, closed without current"),
            F("Heating"),
            F("Backing pump"),
            F("Fan (temperature controlled)"),
            F("Sealing gas"),
            F("Always '0'"),
            F("Always '1'"),
            F("Power failure venting unit"),
            F(""), // 9
            F(""), // 10
            F(""), // 11
            F("Second venting valve"),
            F("No function")};
        return func_unknown_if(names, sizeof(names) / sizeof(names[0]), idx, "FuncAccA1Cfg");
    }

    static const String lookupFuncPress1HVen(uint8_t idx)
    {
        static const __FlashStringHelper *names[] = {
            F("Off"),
            F("On"),
            F("On, with rotation speed switch point reached"),
            F("On, with pressure switch point not reached")};
        return func_unknown_if(names, sizeof(names) / sizeof(names[0]), idx, "FuncPress1HVen");
    }

    static const String lookupFuncOutputAO1Cfg(uint8_t idx)
    {
        static const __FlashStringHelper *names[] = {
            F("Actual rotation speed"),
            F("Output"),
            F("Current"),
            F("Always 0V"),
            F("Always 10V"),
            F(""), // 5
            F("Pressure value 1"),
            F("Pressure value 2"),
            F("Fore-vacuum control")};
        return func_unknown_if(names, sizeof(names) / sizeof(names[0]), idx, "FuncOutputAO1Cfg");
    }

    static const String lookupFuncTmpMgtMode(uint8_t idx)
    {
        static const __FlashStringHelper *names[] = {
            F("Pump housing temperature ≤60C"),
            F("Pump housing temperature ≤80C"),
            F("Power characteristic [P:027]")};
        return func_unknown_if(names, sizeof(names) / sizeof(names[0]), idx, "FuncTmpMgtMode");
    }

    static const String lookupFuncCtrlVialnt(uint8_t idx)
    {
        static const __FlashStringHelper *names[256] = {F("")};
        names[1] = F("Remote");
        names[2] = F("RS-485");
        names[4] = F("PV.can");
        names[255] = F("Unlock interface selection");
        return func_unknown_if(names, sizeof(names) / sizeof(names[0]), idx, "FuncCtrlVialnt");
    }

    static const String lookupFuncDI1Cfg(uint8_t idx)
    {
        static const __FlashStringHelper *names[] = {
            F("Deactivated"),
            F("Enable venting"),
            F("Heating"),
            F("Sealing gas"),
            F("Run-up time monitoring"),
            F("Rotation speed setting mode"),
            F("Motor"),
            F("Enable HV sensor 1")};
        return func_unknown_if(names, sizeof(names) / sizeof(names[0]), idx, "FuncDI1Cfg");
    }

    // Stores details for parameters about datatype, description, and functions for debugging/lookup
    typedef const String (*LookupFuncFunction)(uint8_t idx);
    struct ParameterDebugEntry
    {
        int number;
        const __FlashStringHelper *name; // Short name
        const __FlashStringHelper *description;
        const __FlashStringHelper *details; // Optional details such as function values or unit/min/max/default, can be nullptr
        DataType datatype;
        LookupFuncFunction lookupFuncFunction; // Pointer to function that takes uint8_t and returns String
    };

    // debug entries

    // ControlCommand::Heating debug strings
    const char debug_Heating_name[] PROGMEM = "Heating";
    const char debug_Heating_sdesc[] PROGMEM = "Heating";
    const char debug_Heating_ldesc[] PROGMEM = "FuncOnOff, Values: Off=0, On=1, Type: boolean_old, RW, min=0, max=1, default=0";

    // ControlCommand::StandBy debug strings
    const char debug_StandBy_name[] PROGMEM = "StandBy";
    const char debug_StandBy_sdesc[] PROGMEM = "Stand-by";
    const char debug_StandBy_ldesc[] PROGMEM = "FuncOnOff, Values: Off=0, On=1, Type: boolean_old, RW, min=0, max=1, default=0";

    // ControlCommand::RUTimeCtrl debug strings
    const char debug_RUTimeCtrl_name[] PROGMEM = "RUTimeCtrl";
    const char debug_RUTimeCtrl_sdesc[] PROGMEM = "Run-up time monitoring";
    const char debug_RUTimeCtrl_ldesc[] PROGMEM = "FuncOnOff, Values: Off=0, On=1, Type: boolean_old, RW, min=0, max=1, default=1";

    // ControlCommand::ErrorAckn debug strings
    const char debug_ErrorAckn_name[] PROGMEM = "ErrorAckn";
    const char debug_ErrorAckn_sdesc[] PROGMEM = "Malfunction acknowledgement";
    const char debug_ErrorAckn_ldesc[] PROGMEM = "FuncMalfunctionAckn, Values: Acknowledge=1, Type: boolean_old, W, min=1, max=1";

    // ControlCommand::PumpgStatn debug strings
    const char debug_PumpgStatn_name[] PROGMEM = "PumpgStatn";
    const char debug_PumpgStatn_sdesc[] PROGMEM = "Pumping station";
    const char debug_PumpgStatn_ldesc[] PROGMEM = "FuncPumpingStation, Values: Off=0, OnAndAckn=1, Type: boolean_old, RW, min=0, max=1, default=0";

    // ControlCommand::EnableVent debug strings
    const char debug_EnableVent_name[] PROGMEM = "EnableVent";
    const char debug_EnableVent_sdesc[] PROGMEM = "Enable venting";
    const char debug_EnableVent_ldesc[] PROGMEM = "FuncYesNo, Values: No=0, Yes=1, Type: boolean_old, RW, min=0, max=1, default=0";

    // ControlCommand::CfgSpdSwPt debug strings
    const char debug_CfgSpdSwPt_name[] PROGMEM = "CfgSpdSwPt";
    const char debug_CfgSpdSwPt_sdesc[] PROGMEM = "Rotation speed switchpoint configuration";
    const char debug_CfgSpdSwPt_ldesc[] PROGMEM = "FuncRotationSwitchpointCfg, Values: Switchpoint1=0, Switchpoints1And2=1, Type: u_short_int, RW, min=0, max=1, default=0";

    // ControlCommand::CfgDO2 debug strings
    const char debug_CfgDO2_name[] PROGMEM = "CfgDO2";
    const char debug_CfgDO2_sdesc[] PROGMEM = "Output DO2 configuration";
    const char debug_CfgDO2_ldesc[] PROGMEM = "FuncOutputDO2Cfg, Type: u_short_int, RW, min=0, max=22, default=1";

    // ControlCommand::MotorPump debug strings
    const char debug_MotorPump_name[] PROGMEM = "MotorPump";
    const char debug_MotorPump_sdesc[] PROGMEM = "Motor pump";
    const char debug_MotorPump_ldesc[] PROGMEM = "FuncOnOff, Values: Off=0, On=1, Type: boolean_old, RW, min=0, max=1, default=1";

    // ControlCommand::CfgDO1 debug strings
    const char debug_CfgDO1_name[] PROGMEM = "CfgDO1";
    const char debug_CfgDO1_sdesc[] PROGMEM = "Output DO1 configuration";
    const char debug_CfgDO1_ldesc[] PROGMEM = "FuncOutputDO1Cfg, Type: u_short_int, RW, min=0, max=22, default=0";

    // ControlCommand::OpModeBKP debug strings
    const char debug_OpModeBKP_name[] PROGMEM = "OpModeBKP";
    const char debug_OpModeBKP_sdesc[] PROGMEM = "Backing pump operating mode";
    const char debug_OpModeBKP_ldesc[] PROGMEM = "FuncBackingPumpOpMode, Values: Continuous=0, Intermittent=1, DelayedOn=2, DelayedInterval=3, Type: u_short_int, RW, min=0, max=3, default=0";

    // ControlCommand::SpdSetMode debug strings
    const char debug_SpdSetMode_name[] PROGMEM = "SpdSetMode";
    const char debug_SpdSetMode_sdesc[] PROGMEM = "Rotation speed setting mode";
    const char debug_SpdSetMode_ldesc[] PROGMEM = "FuncOnOff, Values: Off=0, On=1, Type: u_short_int, RW, min=0, max=1, default=0";

    // ControlCommand::GasMode debug strings
    const char debug_GasMode_name[] PROGMEM = "GasMode";
    const char debug_GasMode_sdesc[] PROGMEM = "Gas mode";
    const char debug_GasMode_ldesc[] PROGMEM = "FuncGasMode, Values: HeavyGases=0, LightGases=1, Helium=2, Type: u_short_int, RW, min=0, max=2, default=0";

    // ControlCommand::VentMode debug strings
    const char debug_VentMode_name[] PROGMEM = "VentMode";
    const char debug_VentMode_sdesc[] PROGMEM = "Venting mode";
    const char debug_VentMode_ldesc[] PROGMEM = "FuncVentMode, Values: DelayedVenting=0, NoVenting=1, DirectVenting=2, Type: u_short_int, RW, min=0, max=2, default=2";

    // ControlCommand::CfgAccA1 debug strings
    const char debug_CfgAccA1_name[] PROGMEM = "CfgAccA1";
    const char debug_CfgAccA1_sdesc[] PROGMEM = "Configuration accessory connection A1";
    const char debug_CfgAccA1_ldesc[] PROGMEM = "FuncAccA1Cfg, Type: u_short_int, RW, min=0, max=13, default=0";

    // ControlCommand::CfgAccB1 debug strings
    const char debug_CfgAccB1_name[] PROGMEM = "CfgAccB1";
    const char debug_CfgAccB1_sdesc[] PROGMEM = "Configuration accessory connection B1";
    const char debug_CfgAccB1_ldesc[] PROGMEM = "FuncAccB1Cfg, Type: u_short_int, RW, min=0, max=13, default=1";

    // ControlCommand::Press1HVen debug strings
    const char debug_Press1HVen_name[] PROGMEM = "Press1HVen";
    const char debug_Press1HVen_sdesc[] PROGMEM = "Release HV sensor integrated";
    const char debug_Press1HVen_ldesc[] PROGMEM = "FuncPress1HVen, Values: Off=0, On=1, OnWithRotSpdSwitchpoint=2, OnWithPressureSwitchNotReached=3, Type: u_short_int, RW, min=0, max=3, default=2";

    // ControlCommand::SealingGas debug strings
    const char debug_SealingGas_name[] PROGMEM = "SealingGas";
    const char debug_SealingGas_sdesc[] PROGMEM = "Sealing gas";
    const char debug_SealingGas_ldesc[] PROGMEM = "FuncOnOff, Values: Off=0, On=1, Type: boolean_old, RW, min=0, max=1, default=0";

    // ControlCommand::CfgAO1 debug strings
    const char debug_CfgAO1_name[] PROGMEM = "CfgAO1";
    const char debug_CfgAO1_sdesc[] PROGMEM = "Output AO1 configuration";
    const char debug_CfgAO1_ldesc[] PROGMEM = "FuncOutputAO1Cfg, Type: u_short_int, RW, min=0, max=8, default=0";

    // ControlCommand::TmpMgtMode debug strings
    const char debug_TmpMgtMode_name[] PROGMEM = "TmpMgtMode";
    const char debug_TmpMgtMode_sdesc[] PROGMEM = "Temperature management configuration";
    const char debug_TmpMgtMode_ldesc[] PROGMEM = "FuncTmpMgtMode, Values: PumpTempLEQ60C=0, PumpTempLEQ80C=1, PowerCharacteristic=2, Type: u_short_int, RW, min=0, max=2, default=0";

    // ControlCommand::CtrlVialnt debug strings
    const char debug_CtrlVialnt_name[] PROGMEM = "CtrlVialnt";
    const char debug_CtrlVialnt_sdesc[] PROGMEM = "Operate via interface";
    const char debug_CtrlVialnt_ldesc[] PROGMEM = "FuncCtrlVialnt, Values: Remote=1, RS485=2, PVCAN=4, UnlockInterfaceSelection=255, Type: u_short_int, RW, min=1, max=255, default=1";

    // ControlCommand::IntSelLckd debug strings
    const char debug_IntSelLckd_name[] PROGMEM = "IntSelLckd";
    const char debug_IntSelLckd_sdesc[] PROGMEM = "Interface selection locked";
    const char debug_IntSelLckd_ldesc[] PROGMEM = "FuncOnOff, Values: Off=0, On=1, Type: boolean_old, RW, min=0, max=1, default=0";

    // ControlCommand::CfgDI1 debug strings
    const char debug_CfgDI1_name[] PROGMEM = "CfgDI1";
    const char debug_CfgDI1_sdesc[] PROGMEM = "Input DI1 configuration";
    const char debug_CfgDI1_ldesc[] PROGMEM = "FuncDI1Cfg, Type: u_short_int, RW, min=0, max=7, default=1";

    // ControlCommand::CfgD12 debug strings
    const char debug_CfgD12_name[] PROGMEM = "CfgD12";
    const char debug_CfgD12_sdesc[] PROGMEM = "Input D12 configuration";
    const char debug_CfgD12_ldesc[] PROGMEM = "FuncDI2Cfg, Type: u_short_int, RW, min=0, max=7, default=2";

    // ControlCommand::CfgAccC1 debug strings
    const char debug_CfgAccC1_name[] PROGMEM = "CfgAccC1";
    const char debug_CfgAccC1_sdesc[] PROGMEM = "Configuration accessory connection C1";
    const char debug_CfgAccC1_ldesc[] PROGMEM = "FuncAccC1Cfg, Type: u_short_int, RW, min=0, max=13, default=0";

    // ControlCommand::CfgAccD1 debug strings
    const char debug_CfgAccD1_name[] PROGMEM = "CfgAccD1";
    const char debug_CfgAccD1_sdesc[] PROGMEM = "Configuration accessory connection D1";
    const char debug_CfgAccD1_ldesc[] PROGMEM = "FuncAccD1Cfg, Type: u_short_int, RW, min=0, max=13, default=0";

    // StatusRequest::RemotePrio debug strings
    const char debug_RemotePrio_name[] PROGMEM = "RemotePrio";
    const char debug_RemotePrio_sdesc[] PROGMEM = "Remote priority";
    const char debug_RemotePrio_ldesc[] PROGMEM = "Type: boolean_old, R, min=0, max=1";

    // StatusRequest::SpdSwPtAtt debug strings
    const char debug_SpdSwPtAtt_name[] PROGMEM = "SpdSwPtAtt";
    const char debug_SpdSwPtAtt_sdesc[] PROGMEM = "Rotation speed switchpoint reached";
    const char debug_SpdSwPtAtt_ldesc[] PROGMEM = "Type: boolean_old, R, min=0, max=1";

    // StatusRequest::ErrorCode debug strings
    const char debug_ErrorCode_name[] PROGMEM = "ErrorCode";
    const char debug_ErrorCode_sdesc[] PROGMEM = "Error code";
    const char debug_ErrorCode_ldesc[] PROGMEM = "Type: string6, R";

    // StatusRequest::OvTempElec debug strings
    const char debug_OvTempElec_name[] PROGMEM = "OvTempElec";
    const char debug_OvTempElec_sdesc[] PROGMEM = "Excess temperature drive electronics";
    const char debug_OvTempElec_ldesc[] PROGMEM = "Type: boolean_old, R, min=0, max=1";

    // StatusRequest::OvTempPump debug strings
    const char debug_OvTempPump_name[] PROGMEM = "OvTempPump";
    const char debug_OvTempPump_sdesc[] PROGMEM = "Excess temperature pump";
    const char debug_OvTempPump_ldesc[] PROGMEM = "Type: boolean_old, R, min=0, max=1";

    // StatusRequest::SetSpdAtt debug strings
    const char debug_SetSpdAtt_name[] PROGMEM = "SetSpdAtt";
    const char debug_SetSpdAtt_sdesc[] PROGMEM = "Set rotation speed reached";
    const char debug_SetSpdAtt_ldesc[] PROGMEM = "Type: boolean_old, R, min=0, max=1";

    // StatusRequest::PumpAccel debug strings
    const char debug_PumpAccel_name[] PROGMEM = "PumpAccel";
    const char debug_PumpAccel_sdesc[] PROGMEM = "Pump accelerating";
    const char debug_PumpAccel_ldesc[] PROGMEM = "Type: boolean_old, R, min=0, max=1";

    // StatusRequest::SetRotSpdHz debug strings
    const char debug_SetRotSpdHz_name[] PROGMEM = "SetRotSpdHz";
    const char debug_SetRotSpdHz_sdesc[] PROGMEM = "Set rotation speed (Hz)";
    const char debug_SetRotSpdHz_ldesc[] PROGMEM = "Type: u_integer, R, Hz, min=0, max=999999";

    // StatusRequest::ActualSpdHz debug strings
    const char debug_ActualSpdHz_name[] PROGMEM = "ActualSpdHz";
    const char debug_ActualSpdHz_sdesc[] PROGMEM = "Actual rotational speed (Hz)";
    const char debug_ActualSpdHz_ldesc[] PROGMEM = "Type: u_integer, R, Hz, min=0, max=999999";

    // StatusRequest::DrvCurrent debug strings
    const char debug_DrvCurrent_name[] PROGMEM = "DrvCurrent";
    const char debug_DrvCurrent_sdesc[] PROGMEM = "Drive current";
    const char debug_DrvCurrent_ldesc[] PROGMEM = "Type: u_real, R, A, min=0, max=9999.99";

    // StatusRequest::OpHrsPump debug strings
    const char debug_OpHrsPump_name[] PROGMEM = "OpHrsPump";
    const char debug_OpHrsPump_sdesc[] PROGMEM = "Pump operating hours";
    const char debug_OpHrsPump_ldesc[] PROGMEM = "Type: u_integer, R, h, min=0, max=65535";

    // StatusRequest::FwVersion debug strings
    const char debug_FwVersion_name[] PROGMEM = "FwVersion";
    const char debug_FwVersion_sdesc[] PROGMEM = "Drive electronics software version";
    const char debug_FwVersion_ldesc[] PROGMEM = "Type: string6, R";

    // StatusRequest::DrvVoltage debug strings
    const char debug_DrvVoltage_name[] PROGMEM = "DrvVoltage";
    const char debug_DrvVoltage_sdesc[] PROGMEM = "Drive voltage";
    const char debug_DrvVoltage_ldesc[] PROGMEM = "Type: u_real, R, V, min=0, max=9999.99";

    // StatusRequest::OpHrsElec debug strings
    const char debug_OpHrsElec_name[] PROGMEM = "OpHrsElec";
    const char debug_OpHrsElec_sdesc[] PROGMEM = "Drive electronics operating hours";
    const char debug_OpHrsElec_ldesc[] PROGMEM = "Type: u_integer, R, h, min=0, max=65535";

    // StatusRequest::NominalSpdHz debug strings
    const char debug_NominalSpdHz_name[] PROGMEM = "NominalSpdHz";
    const char debug_NominalSpdHz_sdesc[] PROGMEM = "Nominal rotation speed (Hz)";
    const char debug_NominalSpdHz_ldesc[] PROGMEM = "Type: u_integer, R, Hz, min=0, max=999999";

    // StatusRequest::DrvPower debug strings
    const char debug_DrvPower_name[] PROGMEM = "DrvPower";
    const char debug_DrvPower_sdesc[] PROGMEM = "Drive power";
    const char debug_DrvPower_ldesc[] PROGMEM = "Type: u_integer, R, W, min=0, max=999999";

    // StatusRequest::PumpCycles debug strings
    const char debug_PumpCycles_name[] PROGMEM = "PumpCycles";
    const char debug_PumpCycles_sdesc[] PROGMEM = "Pump cycles";
    const char debug_PumpCycles_ldesc[] PROGMEM = "Type: u_integer, R, min=0, max=65535";

    // StatusRequest::TmpPwrStg debug strings
    const char debug_TmpPwrStg_name[] PROGMEM = "TmpPwrStg";
    const char debug_TmpPwrStg_sdesc[] PROGMEM = "Temperature power stage";
    const char debug_TmpPwrStg_ldesc[] PROGMEM = "Type: u_integer, R, °C, min=0, max=999999";

    // StatusRequest::TempElec debug strings
    const char debug_TempElec_name[] PROGMEM = "TempElec";
    const char debug_TempElec_sdesc[] PROGMEM = "Electronics temperature";
    const char debug_TempElec_ldesc[] PROGMEM = "Type: u_integer, R, °C, min=0, max=999999";

    // StatusRequest::TempPmpBot debug strings
    const char debug_TempPmpBot_name[] PROGMEM = "TempPmpBot";
    const char debug_TempPmpBot_sdesc[] PROGMEM = "Pump lower part temperature";
    const char debug_TempPmpBot_ldesc[] PROGMEM = "Type: u_integer, R, °C, min=0, max=999999";

    // StatusRequest::AccelDecel debug strings
    const char debug_AccelDecel_name[] PROGMEM = "AccelDecel";
    const char debug_AccelDecel_sdesc[] PROGMEM = "Acceleration/deceleration";
    const char debug_AccelDecel_ldesc[] PROGMEM = "Type: u_integer, R, rpm/s, min=0, max=999999";

    // StatusRequest::ElecName debug strings
    const char debug_ElecName_name[] PROGMEM = "ElecName";
    const char debug_ElecName_sdesc[] PROGMEM = "Electronic drive unit designation";
    const char debug_ElecName_ldesc[] PROGMEM = "Type: string6, R";

    // StatusRequest::HWVersion debug strings
    const char debug_HWVersion_name[] PROGMEM = "HWVersion";
    const char debug_HWVersion_sdesc[] PROGMEM = "Hardware version drive electronics";
    const char debug_HWVersion_ldesc[] PROGMEM = "Type: string6, R";

    // StatusRequest::SerialNo debug strings
    const char debug_SerialNo_name[] PROGMEM = "SerialNo";
    const char debug_SerialNo_sdesc[] PROGMEM = "Serial number";
    const char debug_SerialNo_ldesc[] PROGMEM = "Type: string16, R";

    // StatusRequest::ErrHist1 debug strings
    const char debug_ErrHist1_name[] PROGMEM = "ErrHist1";
    const char debug_ErrHist1_sdesc[] PROGMEM = "Error code history, item 1";
    const char debug_ErrHist1_ldesc[] PROGMEM = "Type: string6, R";

    // StatusRequest::ErrHist2 debug strings
    const char debug_ErrHist2_name[] PROGMEM = "ErrHist2";
    const char debug_ErrHist2_sdesc[] PROGMEM = "Error code history, item 2";
    const char debug_ErrHist2_ldesc[] PROGMEM = "Type: string6, R";

    // StatusRequest::ErrHist3 debug strings
    const char debug_ErrHist3_name[] PROGMEM = "ErrHist3";
    const char debug_ErrHist3_sdesc[] PROGMEM = "Error code history, item 3";
    const char debug_ErrHist3_ldesc[] PROGMEM = "Type: string6, R";

    // StatusRequest::ErrHist4 debug strings
    const char debug_ErrHist4_name[] PROGMEM = "ErrHist4";
    const char debug_ErrHist4_sdesc[] PROGMEM = "Error code history, item 4";
    const char debug_ErrHist4_ldesc[] PROGMEM = "Type: string6, R";

    // StatusRequest::ErrHist5 debug strings
    const char debug_ErrHist5_name[] PROGMEM = "ErrHist5";
    const char debug_ErrHist5_sdesc[] PROGMEM = "Error code history, item 5";
    const char debug_ErrHist5_ldesc[] PROGMEM = "Type: string6, R";

    // StatusRequest::ErrHist6 debug strings
    const char debug_ErrHist6_name[] PROGMEM = "ErrHist6";
    const char debug_ErrHist6_sdesc[] PROGMEM = "Error code history, item 6";
    const char debug_ErrHist6_ldesc[] PROGMEM = "Type: string6, R";

    // StatusRequest::ErrHist7 debug strings
    const char debug_ErrHist7_name[] PROGMEM = "ErrHist7";
    const char debug_ErrHist7_sdesc[] PROGMEM = "Error code history, item 7";
    const char debug_ErrHist7_ldesc[] PROGMEM = "Type: string6, R";

    // StatusRequest::ErrHist8 debug strings
    const char debug_ErrHist8_name[] PROGMEM = "ErrHist8";
    const char debug_ErrHist8_sdesc[] PROGMEM = "Error code history, item 8";
    const char debug_ErrHist8_ldesc[] PROGMEM = "Type: string6, R";

    // StatusRequest::ErrHist9 debug strings
    const char debug_ErrHist9_name[] PROGMEM = "ErrHist9";
    const char debug_ErrHist9_sdesc[] PROGMEM = "Error code history, item 9";
    const char debug_ErrHist9_ldesc[] PROGMEM = "Type: string6, R";

    // StatusRequest::ErrHist10 debug strings
    const char debug_ErrHist10_name[] PROGMEM = "ErrHist10";
    const char debug_ErrHist10_sdesc[] PROGMEM = "Error code history, item 10";
    const char debug_ErrHist10_ldesc[] PROGMEM = "Type: string6, R";

    // StatusRequest::TempRotor debug strings
    const char debug_TempRotor_name[] PROGMEM = "TempRotor";
    const char debug_TempRotor_sdesc[] PROGMEM = "Rotor temperature";
    const char debug_TempRotor_ldesc[] PROGMEM = "Type: u_integer, R, °C, min=0, max=999999";

    // StatusRequest::OrderCode debug strings
    const char debug_OrderCode_name[] PROGMEM = "OrderCode";
    const char debug_OrderCode_sdesc[] PROGMEM = "Order number";
    const char debug_OrderCode_ldesc[] PROGMEM = "Type: string16, R";

    // StatusRequest::AddID debug strings
    const char debug_AddID_name[] PROGMEM = "AddID";
    const char debug_AddID_sdesc[] PROGMEM = "Pump ID";
    const char debug_AddID_ldesc[] PROGMEM = "Type: u_integer, R";

    // StatusRequest::SetRotSpdRpm debug strings
    const char debug_SetRotSpdRpm_name[] PROGMEM = "SetRotSpdRpm";
    const char debug_SetRotSpdRpm_sdesc[] PROGMEM = "Set rotation speed (rpm)";
    const char debug_SetRotSpdRpm_ldesc[] PROGMEM = "Type: u_integer, R, rpm, min=0, max=999999";

    // StatusRequest::ActualSpdRpm debug strings
    const char debug_ActualSpdRpm_name[] PROGMEM = "ActualSpdRpm";
    const char debug_ActualSpdRpm_sdesc[] PROGMEM = "Actual rotational speed (rpm)";
    const char debug_ActualSpdRpm_ldesc[] PROGMEM = "Type: u_integer, R, rpm, min=0, max=999999";

    // StatusRequest::NominalSpdRpm debug strings
    const char debug_NominalSpdRpm_name[] PROGMEM = "NominalSpdRpm";
    const char debug_NominalSpdRpm_sdesc[] PROGMEM = "Nominal rotation speed (rpm)";
    const char debug_NominalSpdRpm_ldesc[] PROGMEM = "Type: u_integer, R, rpm, min=0, max=999999";

    // ReferenceValueInput::RUTimeSVal debug strings
    const char debug_RUTimeSVal_name[] PROGMEM = "RUTimeSVal";
    const char debug_RUTimeSVal_sdesc[] PROGMEM = "Set value run-up time";
    const char debug_RUTimeSVal_ldesc[] PROGMEM = "Type: u_integer, RW, min=1, max=120, default=8";

    // ReferenceValueInput::SpdSwPt1 debug strings
    const char debug_SpdSwPt1_name[] PROGMEM = "SpdSwPt1";
    const char debug_SpdSwPt1_sdesc[] PROGMEM = "Rotation speed switch point 1";
    const char debug_SpdSwPt1_ldesc[] PROGMEM = "Type: u_integer, RW, %, min=50, max=97, default=80";

    // ReferenceValueInput::SpdSVal debug strings
    const char debug_SpdSVal_name[] PROGMEM = "SpdSVal";
    const char debug_SpdSVal_sdesc[] PROGMEM = "Set value in rotation speed setting mode";
    const char debug_SpdSVal_ldesc[] PROGMEM = "Type: u_real, RW, %, min=20, max=100, default=65";

    // ReferenceValueInput::PwrSVal debug strings
    const char debug_PwrSVal_name[] PROGMEM = "PwrSVal";
    const char debug_PwrSVal_sdesc[] PROGMEM = "Set value power consumption";
    const char debug_PwrSVal_ldesc[] PROGMEM = "Type: u_short_int, RW, %, min=10, max=100, default=100";

    // ReferenceValueInput::SwoffBKP debug strings
    const char debug_SwoffBKP_name[] PROGMEM = "SwoffBKP";
    const char debug_SwoffBKP_sdesc[] PROGMEM = "Backing pump switch-off threshold for intermittent operation";
    const char debug_SwoffBKP_ldesc[] PROGMEM = "Type: u_integer, RW, W, min=0, max=1000, default=0";

    // ReferenceValueInput::SwOnBKP debug strings
    const char debug_SwOnBKP_name[] PROGMEM = "SwOnBKP";
    const char debug_SwOnBKP_sdesc[] PROGMEM = "Backing pump switch-on threshold for intermittent operation";
    const char debug_SwOnBKP_ldesc[] PROGMEM = "Type: u_integer, RW, W, min=0, max=1000, default=0";

    // ReferenceValueInput::StdbySVal debug strings
    const char debug_StdbySVal_name[] PROGMEM = "StdbySVal";
    const char debug_StdbySVal_sdesc[] PROGMEM = "Rotation speed set value in stand-by operation";
    const char debug_StdbySVal_ldesc[] PROGMEM = "Type: u_real, RW, %, min=20, max=100, default=66.7";

    // ReferenceValueInput::SpdSwPt2 debug strings
    const char debug_SpdSwPt2_name[] PROGMEM = "SpdSwPt2";
    const char debug_SpdSwPt2_sdesc[] PROGMEM = "Rotation speed switch point 2";
    const char debug_SpdSwPt2_ldesc[] PROGMEM = "Type: u_integer, RW, %, min=5, max=97, default=20";

    // ReferenceValueInput::VentSpd debug strings
    const char debug_VentSpd_name[] PROGMEM = "VentSpd";
    const char debug_VentSpd_sdesc[] PROGMEM = "Venting at rotation speed, delayed venting";
    const char debug_VentSpd_ldesc[] PROGMEM = "Type: u_short_int, RW, %, min=40, max=98, default=50";

    // ReferenceValueInput::VentTime debug strings
    const char debug_VentTime_name[] PROGMEM = "VentTime";
    const char debug_VentTime_sdesc[] PROGMEM = "Venting time, delayed venting";
    const char debug_VentTime_ldesc[] PROGMEM = "Type: u_integer, RW, s, min=6, max=3600, default=3600";

    // ReferenceValueInput::mxPwrOutTm debug strings
    const char debug_mxPwrOutTm_name[] PROGMEM = "mxPwrOutTm";
    const char debug_mxPwrOutTm_sdesc[] PROGMEM = "Max. time for the output voltage in power back-up mode";
    const char debug_mxPwrOutTm_ldesc[] PROGMEM = "Type: u_integer, RW, s, min=1, max=255, default=10";

    // ReferenceValueInput::fanOnTemp debug strings
    const char debug_fanOnTemp_name[] PROGMEM = "fanOnTemp";
    const char debug_fanOnTemp_sdesc[] PROGMEM = "Start-up temperature of the fan in temperature-controlled mode";
    const char debug_fanOnTemp_ldesc[] PROGMEM = "Type: u_integer, RW, °C, min=6, max=75, default=45";

    // ReferenceValueInput::PrsSwPt1 debug strings
    const char debug_PrsSwPt1_name[] PROGMEM = "PrsSwPt1";
    const char debug_PrsSwPt1_sdesc[] PROGMEM = "Pressure switch point 1";
    const char debug_PrsSwPt1_ldesc[] PROGMEM = "Type: u_expo_new, RW, hPa, default=1000";

    // ReferenceValueInput::PrsSwPt2 debug strings
    const char debug_PrsSwPt2_name[] PROGMEM = "PrsSwPt2";
    const char debug_PrsSwPt2_sdesc[] PROGMEM = "Pressure switch point 2";
    const char debug_PrsSwPt2_ldesc[] PROGMEM = "Type: u_expo_new, RW, hPa, default=1000";

    // ReferenceValueInput::PwrOutVolt debug strings
    const char debug_PwrOutVolt_name[] PROGMEM = "PwrOutVolt";
    const char debug_PwrOutVolt_sdesc[] PROGMEM = "Output voltage in power back-up mode";
    const char debug_PwrOutVolt_ldesc[] PROGMEM = "Type: u_real, RW, V, min=20.50, max=26.50, default=23.00";

    // ReferenceValueInput::PwrOutThrs debug strings
    const char debug_PwrOutThrs_name[] PROGMEM = "PwrOutThrs";
    const char debug_PwrOutThrs_sdesc[] PROGMEM = "Power threshold from which the voltage is output from P733";
    const char debug_PwrOutThrs_ldesc[] PROGMEM = "Type: u_integer, RW, W, min=15, max=150, default=20";

    // ReferenceValueInput::PrsSn1Name debug strings
    const char debug_PrsSn1Name_name[] PROGMEM = "PrsSn1Name";
    const char debug_PrsSn1Name_sdesc[] PROGMEM = "Name sensor 1";
    const char debug_PrsSn1Name_ldesc[] PROGMEM = "Type: string6, R";

    // ReferenceValueInput::Pressure1 debug strings
    const char debug_Pressure1_name[] PROGMEM = "Pressure1";
    const char debug_Pressure1_sdesc[] PROGMEM = "Pressure value 1";
    const char debug_Pressure1_ldesc[] PROGMEM = "Type: u_expo_new, RW, hPa";

    // ReferenceValueInput::PrsCorrPi1 debug strings
    const char debug_PrsCorrPi1_name[] PROGMEM = "PrsCorrPi1";
    const char debug_PrsCorrPi1_sdesc[] PROGMEM = "Correction factor 1";
    const char debug_PrsCorrPi1_ldesc[] PROGMEM = "Type: u_real, RW, min=0.1, max=8.0, default=0";

    // ReferenceValueInput::PrsSn2Name debug strings
    const char debug_PrsSn2Name_name[] PROGMEM = "PrsSn2Name";
    const char debug_PrsSn2Name_sdesc[] PROGMEM = "Name sensor 2";
    const char debug_PrsSn2Name_ldesc[] PROGMEM = "Type: string6, R";

    // ReferenceValueInput::Pressure2 debug strings
    const char debug_Pressure2_name[] PROGMEM = "Pressure2";
    const char debug_Pressure2_sdesc[] PROGMEM = "Pressure value 2";
    const char debug_Pressure2_ldesc[] PROGMEM = "Type: u_expo_new, RW, hPa";

    // ReferenceValueInput::PrsCorrPi2 debug strings
    const char debug_PrsCorrPi2_name[] PROGMEM = "PrsCorrPi2";
    const char debug_PrsCorrPi2_sdesc[] PROGMEM = "Correction factor 2";
    const char debug_PrsCorrPi2_ldesc[] PROGMEM = "Type: u_real, RW, min=0.1, max=8.0, default=0";

    // ReferenceValueInput::NomSpdConf debug strings
    const char debug_NomSpdConf_name[] PROGMEM = "NomSpdConf";
    const char debug_NomSpdConf_sdesc[] PROGMEM = "Confirmation of nominal rotation speed";
    const char debug_NomSpdConf_ldesc[] PROGMEM = "Type: u_integer, RW, Hz, min=0, max=1500, default=0";

    // ReferenceValueInput::RS485Adr debug strings
    const char debug_RS485Adr_name[] PROGMEM = "RS485Adr";
    const char debug_RS485Adr_sdesc[] PROGMEM = "RS-485 Interface address";
    const char debug_RS485Adr_ldesc[] PROGMEM = "Type: u_integer, RW, min=1, max=255, default=1";

    // AdditionalParams::Pressure debug strings
    const char debug_Pressure_name[] PROGMEM = "Pressure";
    const char debug_Pressure_sdesc[] PROGMEM = "Actual pressure value (ActiveLine)";
    const char debug_Pressure_ldesc[] PROGMEM = "Type: u_short_int, R, hPa, min=1e-10, max=1e3";

    // AdditionalParams::CtrName debug strings
    const char debug_CtrName_name[] PROGMEM = "CtrName";
    const char debug_CtrName_sdesc[] PROGMEM = "Control unit type";
    const char debug_CtrName_ldesc[] PROGMEM = "Type: string6, R";

    // AdditionalParams::CtrSoftware debug strings
    const char debug_CtrSoftware_name[] PROGMEM = "CtrSoftware";
    const char debug_CtrSoftware_sdesc[] PROGMEM = "Control unit software version";
    const char debug_CtrSoftware_ldesc[] PROGMEM = "Type: string6, R";

    // AdditionalParams::GaugeType debug strings
    const char debug_GaugeType_name[] PROGMEM = "GaugeType";
    const char debug_GaugeType_sdesc[] PROGMEM = "Type of pressure gauge";
    const char debug_GaugeType_ldesc[] PROGMEM = "Type: string6, RW";

    // AdditionalParams::ParamSet debug strings
    const char debug_ParamSet_name[] PROGMEM = "ParamSet";
    const char debug_ParamSet_sdesc[] PROGMEM = "Parameter set";
    const char debug_ParamSet_ldesc[] PROGMEM = "FuncOnOff, Values: Basic=0, Extended=1, Type: u_short_int, RW, min=0, max=1, default=0";

    // AdditionalParams::Servicelin debug strings
    const char debug_Servicelin_name[] PROGMEM = "Servicelin";
    const char debug_Servicelin_sdesc[] PROGMEM = "Insert service line";
    const char debug_Servicelin_ldesc[] PROGMEM = "Type: u_short_int, RW, default=795";

    // Static array of all parameters, sorted by number
    static const ParameterDebugEntry ParameterDebugMap[] PROGMEM = {
        // Control Commands
        {(uint16_t)ControlCommand::Heating, reinterpret_cast<const __FlashStringHelper *>(debug_Heating_name), reinterpret_cast<const __FlashStringHelper *>(debug_Heating_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_Heating_ldesc), DataType::BooleanOld, lookupFuncOnOff},
        {(uint16_t)ControlCommand::StandBy, reinterpret_cast<const __FlashStringHelper *>(debug_StandBy_name), reinterpret_cast<const __FlashStringHelper *>(debug_StandBy_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_StandBy_ldesc), DataType::BooleanOld, lookupFuncOnOff},
        {(uint16_t)ControlCommand::RUTimeCtrl, reinterpret_cast<const __FlashStringHelper *>(debug_RUTimeCtrl_name), reinterpret_cast<const __FlashStringHelper *>(debug_RUTimeCtrl_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_RUTimeCtrl_ldesc), DataType::BooleanOld, lookupFuncOnOff},
        {(uint16_t)ControlCommand::ErrorAckn, reinterpret_cast<const __FlashStringHelper *>(debug_ErrorAckn_name), reinterpret_cast<const __FlashStringHelper *>(debug_ErrorAckn_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ErrorAckn_ldesc), DataType::BooleanOld, lookupFuncMalfunctionAckn},
        {(uint16_t)ControlCommand::PumpgStatn, reinterpret_cast<const __FlashStringHelper *>(debug_PumpgStatn_name), reinterpret_cast<const __FlashStringHelper *>(debug_PumpgStatn_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_PumpgStatn_ldesc), DataType::BooleanOld, lookupFuncPumpingStation},
        {(uint16_t)ControlCommand::EnableVent, reinterpret_cast<const __FlashStringHelper *>(debug_EnableVent_name), reinterpret_cast<const __FlashStringHelper *>(debug_EnableVent_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_EnableVent_ldesc), DataType::BooleanOld, lookupFuncYesNo},
        {(uint16_t)ControlCommand::CfgSpdSwPt, reinterpret_cast<const __FlashStringHelper *>(debug_CfgSpdSwPt_name), reinterpret_cast<const __FlashStringHelper *>(debug_CfgSpdSwPt_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_CfgSpdSwPt_ldesc), DataType::UShortInt, lookupFuncRotationSwitchpointCfg},
        {(uint16_t)ControlCommand::CfgDO2, reinterpret_cast<const __FlashStringHelper *>(debug_CfgDO2_name), reinterpret_cast<const __FlashStringHelper *>(debug_CfgDO2_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_CfgDO2_ldesc), DataType::UShortInt, lookupFuncOutputDO2Cfg},
        {(uint16_t)ControlCommand::MotorPump, reinterpret_cast<const __FlashStringHelper *>(debug_MotorPump_name), reinterpret_cast<const __FlashStringHelper *>(debug_MotorPump_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_MotorPump_ldesc), DataType::BooleanOld, lookupFuncOnOff},
        {(uint16_t)ControlCommand::CfgDO1, reinterpret_cast<const __FlashStringHelper *>(debug_CfgDO1_name), reinterpret_cast<const __FlashStringHelper *>(debug_CfgDO1_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_CfgDO1_ldesc), DataType::UShortInt, lookupFuncOutputDO2Cfg},
        {(uint16_t)ControlCommand::OpModeBKP, reinterpret_cast<const __FlashStringHelper *>(debug_OpModeBKP_name), reinterpret_cast<const __FlashStringHelper *>(debug_OpModeBKP_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_OpModeBKP_ldesc), DataType::UShortInt, lookupFuncBackingPumpOpMode},
        {(uint16_t)ControlCommand::SpdSetMode, reinterpret_cast<const __FlashStringHelper *>(debug_SpdSetMode_name), reinterpret_cast<const __FlashStringHelper *>(debug_SpdSetMode_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_SpdSetMode_ldesc), DataType::UShortInt, lookupFuncOnOff},
        {(uint16_t)ControlCommand::GasMode, reinterpret_cast<const __FlashStringHelper *>(debug_GasMode_name), reinterpret_cast<const __FlashStringHelper *>(debug_GasMode_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_GasMode_ldesc), DataType::UShortInt, lookupFuncGasMode},
        {(uint16_t)ControlCommand::VentMode, reinterpret_cast<const __FlashStringHelper *>(debug_VentMode_name), reinterpret_cast<const __FlashStringHelper *>(debug_VentMode_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_VentMode_ldesc), DataType::UShortInt, lookupFuncVentMode},
        {(uint16_t)ControlCommand::CfgAccA1, reinterpret_cast<const __FlashStringHelper *>(debug_CfgAccA1_name), reinterpret_cast<const __FlashStringHelper *>(debug_CfgAccA1_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_CfgAccA1_ldesc), DataType::UShortInt, lookupFuncAccA1Cfg},
        {(uint16_t)ControlCommand::CfgAccB1, reinterpret_cast<const __FlashStringHelper *>(debug_CfgAccB1_name), reinterpret_cast<const __FlashStringHelper *>(debug_CfgAccB1_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_CfgAccB1_ldesc), DataType::UShortInt, lookupFuncAccA1Cfg},
        {(uint16_t)ControlCommand::Press1HVen, reinterpret_cast<const __FlashStringHelper *>(debug_Press1HVen_name), reinterpret_cast<const __FlashStringHelper *>(debug_Press1HVen_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_Press1HVen_ldesc), DataType::UShortInt, lookupFuncPress1HVen},
        {(uint16_t)ControlCommand::SealingGas, reinterpret_cast<const __FlashStringHelper *>(debug_SealingGas_name), reinterpret_cast<const __FlashStringHelper *>(debug_SealingGas_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_SealingGas_ldesc), DataType::BooleanOld, lookupFuncOnOff},
        {(uint16_t)ControlCommand::CfgAO1, reinterpret_cast<const __FlashStringHelper *>(debug_CfgAO1_name), reinterpret_cast<const __FlashStringHelper *>(debug_CfgAO1_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_CfgAO1_ldesc), DataType::UShortInt, lookupFuncOutputAO1Cfg},
        {(uint16_t)ControlCommand::TmpMgtMode, reinterpret_cast<const __FlashStringHelper *>(debug_TmpMgtMode_name), reinterpret_cast<const __FlashStringHelper *>(debug_TmpMgtMode_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_TmpMgtMode_ldesc), DataType::UShortInt, lookupFuncTmpMgtMode},
        {(uint16_t)ControlCommand::CtrlVialnt, reinterpret_cast<const __FlashStringHelper *>(debug_CtrlVialnt_name), reinterpret_cast<const __FlashStringHelper *>(debug_CtrlVialnt_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_CtrlVialnt_ldesc), DataType::UShortInt, lookupFuncCtrlVialnt},
        {(uint16_t)ControlCommand::IntSelLckd, reinterpret_cast<const __FlashStringHelper *>(debug_IntSelLckd_name), reinterpret_cast<const __FlashStringHelper *>(debug_IntSelLckd_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_IntSelLckd_ldesc), DataType::BooleanOld, lookupFuncOnOff},
        {(uint16_t)ControlCommand::CfgDI1, reinterpret_cast<const __FlashStringHelper *>(debug_CfgDI1_name), reinterpret_cast<const __FlashStringHelper *>(debug_CfgDI1_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_CfgDI1_ldesc), DataType::UShortInt, lookupFuncDI1Cfg},
        {(uint16_t)ControlCommand::CfgD12, reinterpret_cast<const __FlashStringHelper *>(debug_CfgD12_name), reinterpret_cast<const __FlashStringHelper *>(debug_CfgD12_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_CfgD12_ldesc), DataType::UShortInt, lookupFuncDI1Cfg},
        {(uint16_t)ControlCommand::CfgAccC1, reinterpret_cast<const __FlashStringHelper *>(debug_CfgAccC1_name), reinterpret_cast<const __FlashStringHelper *>(debug_CfgAccC1_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_CfgAccC1_ldesc), DataType::UShortInt, lookupFuncAccA1Cfg},
        {(uint16_t)ControlCommand::CfgAccD1, reinterpret_cast<const __FlashStringHelper *>(debug_CfgAccD1_name), reinterpret_cast<const __FlashStringHelper *>(debug_CfgAccD1_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_CfgAccD1_ldesc), DataType::UShortInt, lookupFuncAccA1Cfg},
        // Status Requests
        {(uint16_t)StatusRequest::RemotePrio, reinterpret_cast<const __FlashStringHelper *>(debug_RemotePrio_name), reinterpret_cast<const __FlashStringHelper *>(debug_RemotePrio_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_RemotePrio_ldesc), DataType::BooleanOld, lookupFuncYesNo},
        {(uint16_t)StatusRequest::SpdSwPtAtt, reinterpret_cast<const __FlashStringHelper *>(debug_SpdSwPtAtt_name), reinterpret_cast<const __FlashStringHelper *>(debug_SpdSwPtAtt_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_SpdSwPtAtt_ldesc), DataType::BooleanOld, lookupFuncYesNo},
        {(uint16_t)StatusRequest::ErrorCode, reinterpret_cast<const __FlashStringHelper *>(debug_ErrorCode_name), reinterpret_cast<const __FlashStringHelper *>(debug_ErrorCode_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ErrorCode_ldesc), DataType::String6, nullptr},
        {(uint16_t)StatusRequest::OvTempElec, reinterpret_cast<const __FlashStringHelper *>(debug_OvTempElec_name), reinterpret_cast<const __FlashStringHelper *>(debug_OvTempElec_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_OvTempElec_ldesc), DataType::BooleanOld, lookupFuncYesNo},
        {(uint16_t)StatusRequest::OvTempPump, reinterpret_cast<const __FlashStringHelper *>(debug_OvTempPump_name), reinterpret_cast<const __FlashStringHelper *>(debug_OvTempPump_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_OvTempPump_ldesc), DataType::BooleanOld, lookupFuncYesNo},
        {(uint16_t)StatusRequest::SetSpdAtt, reinterpret_cast<const __FlashStringHelper *>(debug_SetSpdAtt_name), reinterpret_cast<const __FlashStringHelper *>(debug_SetSpdAtt_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_SetSpdAtt_ldesc), DataType::BooleanOld, lookupFuncYesNo},
        {(uint16_t)StatusRequest::PumpAccel, reinterpret_cast<const __FlashStringHelper *>(debug_PumpAccel_name), reinterpret_cast<const __FlashStringHelper *>(debug_PumpAccel_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_PumpAccel_ldesc), DataType::BooleanOld, lookupFuncYesNo},
        {(uint16_t)StatusRequest::SetRotSpdHz, reinterpret_cast<const __FlashStringHelper *>(debug_SetRotSpdHz_name), reinterpret_cast<const __FlashStringHelper *>(debug_SetRotSpdHz_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_SetRotSpdHz_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)StatusRequest::ActualSpdHz, reinterpret_cast<const __FlashStringHelper *>(debug_ActualSpdHz_name), reinterpret_cast<const __FlashStringHelper *>(debug_ActualSpdHz_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ActualSpdHz_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)StatusRequest::DrvCurrent, reinterpret_cast<const __FlashStringHelper *>(debug_DrvCurrent_name), reinterpret_cast<const __FlashStringHelper *>(debug_DrvCurrent_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_DrvCurrent_ldesc), DataType::UReal, nullptr},
        {(uint16_t)StatusRequest::OpHrsPump, reinterpret_cast<const __FlashStringHelper *>(debug_OpHrsPump_name), reinterpret_cast<const __FlashStringHelper *>(debug_OpHrsPump_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_OpHrsPump_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)StatusRequest::FwVersion, reinterpret_cast<const __FlashStringHelper *>(debug_FwVersion_name), reinterpret_cast<const __FlashStringHelper *>(debug_FwVersion_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_FwVersion_ldesc), DataType::String6, nullptr},
        {(uint16_t)StatusRequest::DrvVoltage, reinterpret_cast<const __FlashStringHelper *>(debug_DrvVoltage_name), reinterpret_cast<const __FlashStringHelper *>(debug_DrvVoltage_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_DrvVoltage_ldesc), DataType::UReal, nullptr},
        {(uint16_t)StatusRequest::OpHrsElec, reinterpret_cast<const __FlashStringHelper *>(debug_OpHrsElec_name), reinterpret_cast<const __FlashStringHelper *>(debug_OpHrsElec_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_OpHrsElec_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)StatusRequest::NominalSpdHz, reinterpret_cast<const __FlashStringHelper *>(debug_NominalSpdHz_name), reinterpret_cast<const __FlashStringHelper *>(debug_NominalSpdHz_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_NominalSpdHz_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)StatusRequest::DrvPower, reinterpret_cast<const __FlashStringHelper *>(debug_DrvPower_name), reinterpret_cast<const __FlashStringHelper *>(debug_DrvPower_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_DrvPower_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)StatusRequest::PumpCycles, reinterpret_cast<const __FlashStringHelper *>(debug_PumpCycles_name), reinterpret_cast<const __FlashStringHelper *>(debug_PumpCycles_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_PumpCycles_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)StatusRequest::TmpPwrStg, reinterpret_cast<const __FlashStringHelper *>(debug_TmpPwrStg_name), reinterpret_cast<const __FlashStringHelper *>(debug_TmpPwrStg_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_TmpPwrStg_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)StatusRequest::TempElec, reinterpret_cast<const __FlashStringHelper *>(debug_TempElec_name), reinterpret_cast<const __FlashStringHelper *>(debug_TempElec_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_TempElec_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)StatusRequest::TempPmpBot, reinterpret_cast<const __FlashStringHelper *>(debug_TempPmpBot_name), reinterpret_cast<const __FlashStringHelper *>(debug_TempPmpBot_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_TempPmpBot_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)StatusRequest::AccelDecel, reinterpret_cast<const __FlashStringHelper *>(debug_AccelDecel_name), reinterpret_cast<const __FlashStringHelper *>(debug_AccelDecel_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_AccelDecel_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)StatusRequest::ElecName, reinterpret_cast<const __FlashStringHelper *>(debug_ElecName_name), reinterpret_cast<const __FlashStringHelper *>(debug_ElecName_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ElecName_ldesc), DataType::String6, nullptr},
        {(uint16_t)StatusRequest::HWVersion, reinterpret_cast<const __FlashStringHelper *>(debug_HWVersion_name), reinterpret_cast<const __FlashStringHelper *>(debug_HWVersion_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_HWVersion_ldesc), DataType::String6, nullptr},
        {(uint16_t)StatusRequest::SerialNo, reinterpret_cast<const __FlashStringHelper *>(debug_SerialNo_name), reinterpret_cast<const __FlashStringHelper *>(debug_SerialNo_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_SerialNo_ldesc), DataType::String16, nullptr},
        {(uint16_t)StatusRequest::ErrHist1, reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist1_name), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist1_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist1_ldesc), DataType::String6, nullptr},
        {(uint16_t)StatusRequest::ErrHist2, reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist2_name), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist2_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist2_ldesc), DataType::String6, nullptr},
        {(uint16_t)StatusRequest::ErrHist3, reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist3_name), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist3_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist3_ldesc), DataType::String6, nullptr},
        {(uint16_t)StatusRequest::ErrHist4, reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist4_name), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist4_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist4_ldesc), DataType::String6, nullptr},
        {(uint16_t)StatusRequest::ErrHist5, reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist5_name), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist5_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist5_ldesc), DataType::String6, nullptr},
        {(uint16_t)StatusRequest::ErrHist6, reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist6_name), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist6_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist6_ldesc), DataType::String6, nullptr},
        {(uint16_t)StatusRequest::ErrHist7, reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist7_name), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist7_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist7_ldesc), DataType::String6, nullptr},
        {(uint16_t)StatusRequest::ErrHist8, reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist8_name), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist8_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist8_ldesc), DataType::String6, nullptr},
        {(uint16_t)StatusRequest::ErrHist9, reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist9_name), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist9_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist9_ldesc), DataType::String6, nullptr},
        {(uint16_t)StatusRequest::ErrHist10, reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist10_name), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist10_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ErrHist10_ldesc), DataType::String6, nullptr},
        {(uint16_t)StatusRequest::TempRotor, reinterpret_cast<const __FlashStringHelper *>(debug_TempRotor_name), reinterpret_cast<const __FlashStringHelper *>(debug_TempRotor_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_TempRotor_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)StatusRequest::OrderCode, reinterpret_cast<const __FlashStringHelper *>(debug_OrderCode_name), reinterpret_cast<const __FlashStringHelper *>(debug_OrderCode_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_OrderCode_ldesc), DataType::String16, nullptr},
        {(uint16_t)StatusRequest::AddID, reinterpret_cast<const __FlashStringHelper *>(debug_AddID_name), reinterpret_cast<const __FlashStringHelper *>(debug_AddID_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_AddID_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)StatusRequest::SetRotSpdRpm, reinterpret_cast<const __FlashStringHelper *>(debug_SetRotSpdRpm_name), reinterpret_cast<const __FlashStringHelper *>(debug_SetRotSpdRpm_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_SetRotSpdRpm_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)StatusRequest::ActualSpdRpm, reinterpret_cast<const __FlashStringHelper *>(debug_ActualSpdRpm_name), reinterpret_cast<const __FlashStringHelper *>(debug_ActualSpdRpm_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ActualSpdRpm_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)StatusRequest::NominalSpdRpm, reinterpret_cast<const __FlashStringHelper *>(debug_NominalSpdRpm_name), reinterpret_cast<const __FlashStringHelper *>(debug_NominalSpdRpm_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_NominalSpdRpm_ldesc), DataType::UInteger, nullptr},
        // Reference Value Inputs
        {(uint16_t)ReferenceValueInput::RUTimeSVal, reinterpret_cast<const __FlashStringHelper *>(debug_RUTimeSVal_name), reinterpret_cast<const __FlashStringHelper *>(debug_RUTimeSVal_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_RUTimeSVal_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)ReferenceValueInput::SpdSwPt1, reinterpret_cast<const __FlashStringHelper *>(debug_SpdSwPt1_name), reinterpret_cast<const __FlashStringHelper *>(debug_SpdSwPt1_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_SpdSwPt1_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)ReferenceValueInput::SpdSVal, reinterpret_cast<const __FlashStringHelper *>(debug_SpdSVal_name), reinterpret_cast<const __FlashStringHelper *>(debug_SpdSVal_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_SpdSVal_ldesc), DataType::UReal, nullptr},
        {(uint16_t)ReferenceValueInput::PwrSVal, reinterpret_cast<const __FlashStringHelper *>(debug_PwrSVal_name), reinterpret_cast<const __FlashStringHelper *>(debug_PwrSVal_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_PwrSVal_ldesc), DataType::UShortInt, nullptr},
        {(uint16_t)ReferenceValueInput::SwoffBKP, reinterpret_cast<const __FlashStringHelper *>(debug_SwoffBKP_name), reinterpret_cast<const __FlashStringHelper *>(debug_SwoffBKP_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_SwoffBKP_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)ReferenceValueInput::SwOnBKP, reinterpret_cast<const __FlashStringHelper *>(debug_SwOnBKP_name), reinterpret_cast<const __FlashStringHelper *>(debug_SwOnBKP_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_SwOnBKP_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)ReferenceValueInput::StdbySVal, reinterpret_cast<const __FlashStringHelper *>(debug_StdbySVal_name), reinterpret_cast<const __FlashStringHelper *>(debug_StdbySVal_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_StdbySVal_ldesc), DataType::UReal, nullptr},
        {(uint16_t)ReferenceValueInput::SpdSwPt2, reinterpret_cast<const __FlashStringHelper *>(debug_SpdSwPt2_name), reinterpret_cast<const __FlashStringHelper *>(debug_SpdSwPt2_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_SpdSwPt2_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)ReferenceValueInput::VentSpd, reinterpret_cast<const __FlashStringHelper *>(debug_VentSpd_name), reinterpret_cast<const __FlashStringHelper *>(debug_VentSpd_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_VentSpd_ldesc), DataType::UShortInt, nullptr},
        {(uint16_t)ReferenceValueInput::VentTime, reinterpret_cast<const __FlashStringHelper *>(debug_VentTime_name), reinterpret_cast<const __FlashStringHelper *>(debug_VentTime_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_VentTime_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)ReferenceValueInput::mxPwrOutTm, reinterpret_cast<const __FlashStringHelper *>(debug_mxPwrOutTm_name), reinterpret_cast<const __FlashStringHelper *>(debug_mxPwrOutTm_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_mxPwrOutTm_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)ReferenceValueInput::fanOnTemp, reinterpret_cast<const __FlashStringHelper *>(debug_fanOnTemp_name), reinterpret_cast<const __FlashStringHelper *>(debug_fanOnTemp_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_fanOnTemp_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)ReferenceValueInput::PrsSwPt1, reinterpret_cast<const __FlashStringHelper *>(debug_PrsSwPt1_name), reinterpret_cast<const __FlashStringHelper *>(debug_PrsSwPt1_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_PrsSwPt1_ldesc), DataType::UExpoNew, nullptr},
        {(uint16_t)ReferenceValueInput::PrsSwPt2, reinterpret_cast<const __FlashStringHelper *>(debug_PrsSwPt2_name), reinterpret_cast<const __FlashStringHelper *>(debug_PrsSwPt2_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_PrsSwPt2_ldesc), DataType::UExpoNew, nullptr},
        {(uint16_t)ReferenceValueInput::PwrOutVolt, reinterpret_cast<const __FlashStringHelper *>(debug_PwrOutVolt_name), reinterpret_cast<const __FlashStringHelper *>(debug_PwrOutVolt_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_PwrOutVolt_ldesc), DataType::UReal, nullptr},
        {(uint16_t)ReferenceValueInput::PwrOutThrs, reinterpret_cast<const __FlashStringHelper *>(debug_PwrOutThrs_name), reinterpret_cast<const __FlashStringHelper *>(debug_PwrOutThrs_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_PwrOutThrs_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)ReferenceValueInput::PrsSn1Name, reinterpret_cast<const __FlashStringHelper *>(debug_PrsSn1Name_name), reinterpret_cast<const __FlashStringHelper *>(debug_PrsSn1Name_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_PrsSn1Name_ldesc), DataType::String6, nullptr},
        {(uint16_t)ReferenceValueInput::Pressure1, reinterpret_cast<const __FlashStringHelper *>(debug_Pressure1_name), reinterpret_cast<const __FlashStringHelper *>(debug_Pressure1_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_Pressure1_ldesc), DataType::UExpoNew, nullptr},
        {(uint16_t)ReferenceValueInput::PrsCorrPi1, reinterpret_cast<const __FlashStringHelper *>(debug_PrsCorrPi1_name), reinterpret_cast<const __FlashStringHelper *>(debug_PrsCorrPi1_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_PrsCorrPi1_ldesc), DataType::UReal, nullptr},
        {(uint16_t)ReferenceValueInput::PrsSn2Name, reinterpret_cast<const __FlashStringHelper *>(debug_PrsSn2Name_name), reinterpret_cast<const __FlashStringHelper *>(debug_PrsSn2Name_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_PrsSn2Name_ldesc), DataType::String6, nullptr},
        {(uint16_t)ReferenceValueInput::Pressure2, reinterpret_cast<const __FlashStringHelper *>(debug_Pressure2_name), reinterpret_cast<const __FlashStringHelper *>(debug_Pressure2_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_Pressure2_ldesc), DataType::UExpoNew, nullptr},
        {(uint16_t)ReferenceValueInput::PrsCorrPi2, reinterpret_cast<const __FlashStringHelper *>(debug_PrsCorrPi2_name), reinterpret_cast<const __FlashStringHelper *>(debug_PrsCorrPi2_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_PrsCorrPi2_ldesc), DataType::UReal, nullptr},
        {(uint16_t)ReferenceValueInput::NomSpdConf, reinterpret_cast<const __FlashStringHelper *>(debug_NomSpdConf_name), reinterpret_cast<const __FlashStringHelper *>(debug_NomSpdConf_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_NomSpdConf_ldesc), DataType::UInteger, nullptr},
        {(uint16_t)ReferenceValueInput::RS485Adr, reinterpret_cast<const __FlashStringHelper *>(debug_RS485Adr_name), reinterpret_cast<const __FlashStringHelper *>(debug_RS485Adr_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_RS485Adr_ldesc), DataType::UInteger, nullptr},
        // Additional Parameters
        {(uint16_t)AdditionalParams::Pressure, reinterpret_cast<const __FlashStringHelper *>(debug_Pressure_name), reinterpret_cast<const __FlashStringHelper *>(debug_Pressure_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_Pressure_ldesc), DataType::UShortInt, nullptr},
        {(uint16_t)AdditionalParams::CtrName, reinterpret_cast<const __FlashStringHelper *>(debug_CtrName_name), reinterpret_cast<const __FlashStringHelper *>(debug_CtrName_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_CtrName_ldesc), DataType::String6, nullptr},
        {(uint16_t)AdditionalParams::CtrSoftware, reinterpret_cast<const __FlashStringHelper *>(debug_CtrSoftware_name), reinterpret_cast<const __FlashStringHelper *>(debug_CtrSoftware_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_CtrSoftware_ldesc), DataType::String6, nullptr},
        {(uint16_t)AdditionalParams::GaugeType, reinterpret_cast<const __FlashStringHelper *>(debug_GaugeType_name), reinterpret_cast<const __FlashStringHelper *>(debug_GaugeType_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_GaugeType_ldesc), DataType::String6, nullptr},
        {(uint16_t)AdditionalParams::ParamSet, reinterpret_cast<const __FlashStringHelper *>(debug_ParamSet_name), reinterpret_cast<const __FlashStringHelper *>(debug_ParamSet_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_ParamSet_ldesc), DataType::UShortInt, nullptr},
        {(uint16_t)AdditionalParams::Servicelin, reinterpret_cast<const __FlashStringHelper *>(debug_Servicelin_name), reinterpret_cast<const __FlashStringHelper *>(debug_Servicelin_sdesc), reinterpret_cast<const __FlashStringHelper *>(debug_Servicelin_ldesc), DataType::UShortInt, nullptr}};

    bool getParameterDebugEntry(int number, ParameterDebugEntry *entry)
    {
        const size_t entryCount = sizeof(ParameterDebugMap) / sizeof(ParameterDebugMap[0]);

        // Find the parameter by number
        ParameterDebugEntry temp;
        for (size_t i = 0; i < entryCount; ++i)
        {
            // Copy one struct from FLASH (ParameterDebugMap[i]) to RAM (temp)
            memcpy_P(&temp, &ParameterDebugMap[i], sizeof(ParameterDebugEntry));
            // check if the the number matches.
            if (temp.number == number)
            {
                //
                memcpy_P(&entry, &temp, sizeof(ParameterDebugEntry));
                return true; // return false to indicate the parameter was found
            }
        }
        return false; // return false to indicate the parameter could not be found
    }

    // Stores a human redable problem text for possible pump error codes:
    typedef const String (*LookupFuncFunction)(uint8_t idx);
    struct ErrorCodeDebugEntry
    {
        const __FlashStringHelper *errorCode; // 6 character error/warning code like "Err001"
        const __FlashStringHelper *problem;   // Human-readable description of the problem
    };

    const ErrorCodeDebugEntry *getHumanReadableErrorMessage(String errorCode)
    {
        // Static array of all parameters
        // Static array of all parameters
        static const ErrorCodeDebugEntry ErrorCodeDebugMap[] = {
            {F("Err001"), F("Excess rotation speed")},
            {F("Err002"), F("Excess voltage")},
            {F("Err006"), F("Run-up error")},
            {F("Err007"), F("Operating fluid low")},
            {F("Err008"), F("Electronic drive unit - turbopump connection faulty")},
            {F("Err010"), F("Internal device error")},
            {F("Err021"), F("Electronic drive unit does not detect turbo-pump")},
            {F("Err041"), F("Drive fault")},
            {F("Err043"), F("Internal configuration error")},
            {F("Err044"), F("Excess temperature, electronics")},
            {F("Err045"), F("Excess temperature, motor")},
            {F("Err046"), F("Internal initialization error")},
            {F("Err073"), F("Axial magnetic bearing overload")},
            {F("Err074"), F("Radial magnetic bearing overload")},
            {F("Err089"), F("Rotor instable")},
            {F("Err091"), F("Internal device error")},
            {F("Err092"), F("Unknown connection panel")},
            {F("Err093"), F("Motor temperature evaluation faulty")},
            {F("Err094"), F("Electronics temperature evaluation faulty")},
            {F("Err098"), F("Internal communication error")},
            {F("Err106"), F("High rotor temperature")},
            {F("Err107"), F("Final stage group error")},
            {F("Err108"), F("Rotation speed measurement faulty")},
            {F("Err109"), F("Software not released")},
            {F("Err110"), F("Operating fluid evaluation faulty")},
            {F("Err111"), F("Operating fluid pump communication error")},
            {F("Err112"), F("Operating fluid pump group error")},
            {F("Err113"), F("Rotor temperature evaluation faulty")},
            {F("Err114"), F("Final stage temperature evaluation faulty")},
            {F("Err117"), F("Excess temperature, pump lower part")},
            {F("Err118"), F("Excess temperature, final stage")},
            {F("Err119"), F("Excess temperature, bearing")},
            {F("Err143"), F("Operating fluid pump excess temperature")},
            {F("Err777"), F("Nominal rotation speed not confirmed")},
            {F("Err800"), F("Magnetic bearing overflow")},
            {F("Err802"), F("Magnetic bearing sensor technology fault")},
            {F("Err810"), F("Internal configuration error")},
            {F("Err815"), F("Magnetic bearing overflow")},
            {F("Err890"), F("Safety bearing worn")},
            {F("Err891"), F("Rotor imbalance too high")},
            {F("Wrn001"), F("TMS heat-up time expired")},
            {F("Wrn003"), F("TMS temperature invalid")},
            {F("Wrn007"), F("Undervoltage or power failure")},
            {F("Wrn016"), F("Accessory configuration invalid")},
            {F("Wrn018"), F("Operating supremacy conflict")},
            {F("Wrn021"), F("Blocking signal invalid")},
            {F("Wrn034"), F("Sealing gas flow too low")},
            {F("Wrn045"), F("Motor high temperature")},
            {F("Wrn076"), F("Electronics high temperature")},
            {F("Wrn089"), F("Imbalance high")},
            {F("Wrn097"), F("Invalid pump information")},
            {F("Wrn098"), F("Incomplete pump information")},
            {F("Wrn100"), F("Minimum speed not reached")},
            {F("Wrn106"), F("High rotor temperature")},
            {F("Wrn113"), F("Inaccurate rotor temperature")},
            {F("Wrn115"), F("Pump lower part temperature evaluation faulty")},
            {F("Wrn116"), F("Bearing temperature evaluation faulty")},
            {F("Wrn117"), F("Pump lower part high temperature")},
            {F("Wrn118"), F("Final stage high temperature")},
            {F("Wrn119"), F("Bearing high temperature")},
            {F("Wrn143"), F("High operating fluid pump temperature")},
            {F("Wrn168"), F("High delay")},
            {F("Wrn801"), F("Braking transistor defective")},
            {F("Wrn806"), F("Brake resistance defective")},
            {F("Wrn807"), F("Calibration requirement")},
            {F("Wrn890"), F("Safety bearing wear too high")},
            {F("Wrn891"), F("High rotor imbalance")},
        };

        const size_t entryCount = sizeof(ErrorCodeDebugMap) / sizeof(ErrorCodeDebugMap[0]);
        // Find the parameter by number
        for (size_t i = 0; i < entryCount; ++i)
        {
            PGM_P entryErrorCode = reinterpret_cast<PGM_P>(ErrorCodeDebugMap[i].errorCode);
            if (strcasecmp_P(errorCode.c_str(), entryErrorCode) == 0)
            {
                return &ErrorCodeDebugMap[i];
            }
        }
        return nullptr; // Not found
    }
#endif // PFEIFFER_VAC_PROTOCOL_INCLUDE_DEBUG

} // namespace PfeifferProtocol

#endif // PFEIFFER_PROTOCOL_CONSTS_H
