// -- Arduino Standard Includes --
#include <Arduino.h>
#include <Wire.h>
#include <avr/wdt.h>
#include <EEPROM.h>

// -- Bundled 3rd Party Library Includes --
#include <ArduinoLog.h>        // A logging library for Arduino that allows for log levels and printing
#include <DoEEP.h>             // A eeprom wrapper library that provides a simple Key-Value store
#include <LazySerial.h>        // A simple command line interface library for Arduino
#include <PfeifferTurboTC80.h> // In-house library to control Pfeiffer Turbo TC80 turbo pump over RS485

// -- Project includes --
#include "util.h"
#include "watchdog.h"
#include "fluidpump.h"

// --- Forward function declarations ----
void nonBlockDelay(unsigned long ms);

// -- PINS --
#define PIN_LED1 A9
#define PIN_LED2 A8
#define PIN_PWR_PH 49                // PICO_ON
#define PIN_PWR_ROUGHING 51          // MVP_ON
#define PIN_PWR_FLUIDPUMP 53         // CFP_ON
#define PIN_ANALOG_FLUIDPUMP_SPEED 8 // Fluid pump speed control (0-5V pwm signal)

// -- CONSTSANTS --
#define FIRMWARE_VERSION "3.5"
#define OK "OK "
#define ERROR "ERROR"
#define COMMS_SPEED 9600
#define COMMS Serial1
#define DEBUG Serial1
LazySerial::LazySerial<128> lazy(COMMS);

// -- SAVED SETTINGS IN EEPROM --
#define AUTOSTART_DELAY_EEP_KEY "autostart_delay"
#define AUTOSTART_DELAY_DEFAULT 5000UL // milliseconds to wait before autostarting if autostart is enabled
#define AUTOSTART_ENABLED_EEP_KEY "autostart_enabled"
#define AUTOSTART_ENABLED_DEFAULT false
#define LOG_LEVEL_EEP_KEY "log_level"
#define LOG_LEVEL_DEFAULT LOG_LEVEL_INFO

// -- TURBO PUMP CONTROLLER CONFIG --
#define PIN_TC80_RS485_DISABLE_RECEIVE 22 // RE (INVERTED: PIN HIGH = Disabled)
#define PIN_TC80_RS485_ENABLE_SEND 23     // DE (REGULAR: PIN HIGH = Enabled)
#define TC80_SERIAL_SPEED 9600
#define TC80_SERIAL_CONFIG SERIAL_8N1
#define TC80_RESPONSE_TIMEOUT 1000                                                                                         // milliseconds to wait for a response from the TC80
PfeifferSerialTC80 turboTC80(Serial, 1, DEBUG, nonBlockDelay, PIN_TC80_RS485_ENABLE_SEND, PIN_TC80_RS485_DISABLE_RECEIVE); // Turbo pump controller object (address 1, using HardwareSerial1)

FluidPump fluidPump(PIN_ANALOG_FLUIDPUMP_SPEED, PIN_PWR_FLUIDPUMP); // Fluid pump control object
