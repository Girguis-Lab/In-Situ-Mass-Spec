#pragma once

// -- Arduino Standard Includes --
#include <Arduino.h>
#include <Wire.h>
#include <avr/wdt.h>

// -- Bundled 3rd Party Library Includes --
#define DEBUGLOG_DEFAULT_LOG_LEVEL_TRACE // compile time flag, higher level messages will get compiled out.
#define LOG_PREAMBLE ""
#include <DebugLog.h>   // A logging library for Arduino that allows for log levels and printing
#include <LazySerial.h> // A simple command line interface library for Arduino
// #define PFEIFFER_VAC_PROTOCOL_INCLUDE_DEBUG 0
#include <PfeifferTurboTC80.h> // In-house library to control Pfeiffer Turbo TC80 turbo pump over RS485

// -- Project includes --
#include "util.h"
#include "watchdog.h"
#include "fluidpump.h"
#include "savedSettings.h"

// --- Forward function declarations ----
bool nonBlockDelay(unsigned long ms);

// -- PINS --
#define PIN_LED1 A9
#define PIN_LED2 A8
#define PIN_PWR_PH 49
#define PIN_PWR_ROUGHING 51
#define PIN_PWR_FLUIDPUMP 53

powerPin LED1_PWR(A9);               // LED 1 is a staus indicator
powerPin LED2_PWR(A8);               // LED 2 is a warning/error indicator
powerPin ACCESSORY_PWR(49);          // PICO_ON usually the ph probe or cryo pump (if equipped)
powerPin ROUGHING_PWR(51);           // MVP_ON
powerPin FLUIDPUMP_PWR(53);          // CFP_ON
#define PIN_ANALOG_FLUIDPUMP_SPEED 8 // Fluid pump speed control (0-5V pwm signal)
#define PIN_FLUIDPUMP_REVERSE 15     // Fluid pump reverse signal (Physically exposed as bare header J11, pin 6 - the pin furthest from the capacitor)
// Reference: https://docs.arduino.cc/retired/hacking/hardware/PinMapping2560/

// -- CONSTSANTS --
#define FIRMWARE_VERSION "3.6"
#define OK "| OK "
#define ERROR "| ERROR"
#define COMMS_BAUDRATE 9600
#define ACCESSORY_NAME "Cryo" // [MODEL CHANGE] Typically "pH" if the probe is attached or "Cryo" if the cryopump is attached.
// #define INCLUDE_OUT_OF_NORMAL_RANGE_MARKS true // comment out to disable the exclamation marks (!) in the stats logging output for values that fall outside the expected operating range

// --- LOGGING CONSTANTS ---- Maps the Arduinolog library levels to the 3 levels used in this code.
#define DEBUG_LOG_LEVEL_OFF DebugLogLevel::LVL_INFO
#define DEBUG_LOG_LEVEL_LOW DebugLogLevel::LVL_DEBUG
#define DEBUG_LOG_LEVEL_HIGH DebugLogLevel::LVL_TRACE

// comment out the following to use with pre-2026 ISMS versions
// #define COMMS Serial1 // [MODEL CHANGE]
// #define TURBO_SERIAL Serial // [MODEL CHANGE]

// uncomment the following to use with pre-2026 ISMS versions
#define COMMS Serial         // [MODEL CHANGE]
#define TURBO_SERIAL Serial1 // [MODEL CHANGE]

// globals
bool beatActive = false; // used to know if the "BEAT" command was sent indicating any autostart routines should not run this time around.
LazySerial::LazySerial<128> lazy(COMMS);

// -- TURBO PUMP CONTROLLER CONFIG --
#define PIN_TC80_RS485_DISABLE_RECEIVE 22 // RE (INVERTED: PIN HIGH = Disabled)
#define PIN_TC80_RS485_ENABLE_SEND 23     // DE (REGULAR: PIN HIGH = Enabled)
#define TC80_SERIAL_SPEED 9600
#define TC80_SERIAL_CONFIG SERIAL_8N1
#define TC80_RESPONSE_TIMEOUT 1000 // milliseconds to wait for a response from the TC80
#define TC80_TURBO_LOW_SPEED_WARNING_RPM 70000
RS485HardwareSerial turboSerialRS485(TURBO_SERIAL, PIN_TC80_RS485_ENABLE_SEND, PIN_TC80_RS485_DISABLE_RECEIVE, 10);
PfeifferSerialTC80 turboTC80(turboSerialRS485, 1, COMMS, nonBlockDelay); // Turbo pump controller object (address 1, using HardwareSerial1)

FluidPump fluidPump(&FLUIDPUMP_PWR, PIN_ANALOG_FLUIDPUMP_SPEED, PIN_FLUIDPUMP_REVERSE); // Fluid pump control object
