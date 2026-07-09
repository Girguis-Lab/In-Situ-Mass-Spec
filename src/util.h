#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>
#include <EEPROM.h>

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

// make sure there is nothing in EEPROM before example executes
void eraseEEPROM()
{
    for (int i = 0; i < 100; i++)
    {
        EEPROM.write(i, 0);
    }
}

// print a hex dump of the EEPROM memory
#define bytesPerLine 16
void dumpEEPROM(int lines)
{
    char buff[100];
    for (int i = 0; i < lines; i++)
    {
        sprintf(buff, "0x0%02X0: ", i);
        Serial.print(buff);
        for (int j = 0; j < bytesPerLine; j++)
        {
            sprintf(buff, "%02X", EEPROM.read((i * bytesPerLine) + j));
            Serial.print(buff);
            if (j != bytesPerLine - 1)
            {
                Serial.print(",");
            }
        }
        Serial.print("\n");
    }
}

// Source - https://stackoverflow.com/a/1598827
// Posted by Michael Burr, modified by community. See post 'Timeline' for change history
// Retrieved 2026-06-29, License - CC BY-SA 2.5
/** Gives the size of an array in a type-safe way */
#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

/**
 * Concatinates the two arrays &a and &b and puts the result in &out
 * The size of each array must be size must be known at compile time.
 */
template <typename T, size_t N, size_t M>
void concatArrays(const T (&a)[N], const T (&b)[M], T (&out)[N + M])
{
    for (size_t i = 0; i < N; ++i)
    {
        out[i] = a[i];
    }
    for (size_t i = 0; i < M; ++i)
    {
        out[N + i] = b[i];
    }
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

String logLevelToString(DebugLogLevel level)
{
    switch (level)
    {
    case DebugLogLevel::LVL_NONE:
        return "SILENT(THIS SHOULD NOT BE POSSIBLE)";
    case DebugLogLevel::LVL_ERROR:
        return "ERROR(THIS SHOULD NOT BE POSSIBLE)";
    case DebugLogLevel::LVL_WARN:
        return "WARNING(THIS SHOULD NOT BE POSSIBLE)";
    case DebugLogLevel::LVL_INFO: // DEBUG_LOG_LEVEL_OFF
        return "DEBUG_OFF";
    case DebugLogLevel::LVL_DEBUG: // DEBUG_LOG_LEVEL_LOW
        return "DEBUG_LOW";
    case DebugLogLevel::LVL_TRACE: // DEBUG_LOG_LEVEL_HIGH
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
