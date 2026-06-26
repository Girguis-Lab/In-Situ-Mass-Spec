#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>

// Returns the available RAM in bytes on Arduino Mega
inline int getAvailableRAM()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

// Returns the total RAM in bytes on Arduino Mega
inline int getTotalRam()
{
    return RAMEND - RAMSTART + 1;
}

/**
 * Print a string padded with a specific character to a specific width.
 */
void print_padded(Stream &s, const __FlashStringHelper *str, int length, char pad_char = ' ')
{
    s.print(str);
    int str_len = strlen_P(reinterpret_cast<PGM_P>(str));
    for (int i = 0; i < length - str_len; ++i)
    {
        s.print(pad_char);
    }
}

String logLevelToString(int level)
{
    switch (level)
    {
    case LOG_LEVEL_SILENT:
        return "SILENT(THIS SHOULD NOT BE POSSIBLE)";
    case LOG_LEVEL_FATAL:
        return "FATAL(THIS SHOULD NOT BE POSSIBLE)";
    case LOG_LEVEL_ERROR:
        return "ERROR(THIS SHOULD NOT BE POSSIBLE)";
    case LOG_LEVEL_WARNING:
        return "WARNING(THIS SHOULD NOT BE POSSIBLE)";
    case LOG_LEVEL_INFO: // DEBUG_LOG_LEVEL_OFF
        return "DEBUG_OFF";
    case LOG_LEVEL_TRACE: // DEBUG_LOG_LEVEL_LOW
        return "DEBUG_LOW";
    case LOG_LEVEL_VERBOSE: // DEBUG_LOG_LEVEL_HIGH
        return "DEBUG_HIGH";
    default:
        return "UNKNOWN(THIS SHOULD NOT BE POSSIBLE)";
    }
}

class powerPin
{
public:
    explicit powerPin(const uint8_t pinNum) : pinNum(pinNum)
    {
        state = false;
        initilized = false;
    }

    void begin(const uint8_t defaultState)
    {
        pinMode(pinNum, OUTPUT);
        if (defaultState == HIGH)
            turnOn();
        else
            turnOff();
    }

    void turnOn()
    {
        digitalWrite(pinNum, HIGH);
        state = HIGH;
    }

    void turnOff()
    {
        digitalWrite(pinNum, LOW);
        state = LOW;
    }

    uint8_t getState()
    {
        return state;
    }

private:
    uint8_t state;
    bool initilized;
    uint8_t pinNum;
};

#endif // UTIL_H
