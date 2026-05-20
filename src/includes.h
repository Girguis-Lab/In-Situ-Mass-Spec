#pragma once

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
#include "savedSettings.h"

// --- Forward function declarations ----
void nonBlockDelay(unsigned long ms);

// -- PINS --
#define PIN_LED1 A9
#define PIN_LED2 A8
#define PIN_PWR_PH 49                // PICO_ON
#define PIN_PWR_ROUGHING 51          // MVP_ON
#define PIN_PWR_FLUIDPUMP 53         // CFP_ON
#define PIN_ANALOG_FLUIDPUMP_SPEED 8 // Fluid pump speed control (0-5V pwm signal)
#define PIN_FLUIDPUMP_REVERSE 15     // Fluid pump reverse signal (Physically exposed as bare header J11, pin 6 - the pin furthest from the capacitor)
// Reference: https://docs.arduino.cc/retired/hacking/hardware/PinMapping2560/

// -- CONSTSANTS --
#define FIRMWARE_VERSION "3.5"
#define OK "OK "
#define ERROR "ERROR"
#define COMMS_BAUDRATE 9600
#define COMMS Serial1
#define DEBUG Serial1
LazySerial::LazySerial<128> lazy(COMMS);

// -- TURBO PUMP CONTROLLER CONFIG --
#define PIN_TC80_RS485_DISABLE_RECEIVE 22 // RE (INVERTED: PIN HIGH = Disabled)
#define PIN_TC80_RS485_ENABLE_SEND 23     // DE (REGULAR: PIN HIGH = Enabled)
#define TC80_SERIAL_SPEED 9600
#define TC80_SERIAL_CONFIG SERIAL_8N1
#define TC80_RESPONSE_TIMEOUT 1000 // milliseconds to wait for a response from the TC80
RS485Serial<HardwareSerial, 128, 128> turboSerialRS485(Serial, PIN_TC80_RS485_ENABLE_SEND, PIN_TC80_RS485_DISABLE_RECEIVE, nonBlockDelay);

// A non-blocking delay function that allows only the turbo serial connection and watchdog resets to keep working while paused in various places.
void nonBlockDelayTurbo(unsigned long ms)
{
    unsigned long start = micros();
    do
    {
        turboSerialRS485.task();
        resetWDT();
    } while (micros() - start < ms * 1000); // Non-blocking delay, handles rollover
}
PfeifferSerialTC80 turboTC80(turboSerialRS485, 1, COMMS, nonBlockDelayTurbo);              // Turbo pump controller object (address 1, using HardwareSerial1)
FluidPump fluidPump(PIN_ANALOG_FLUIDPUMP_SPEED, PIN_PWR_FLUIDPUMP, PIN_FLUIDPUMP_REVERSE); // Fluid pump control object
