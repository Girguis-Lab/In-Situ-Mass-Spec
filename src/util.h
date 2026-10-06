// Small utilities shared across the ISMS firmware.
//
// Collects the pieces that have no better home: SRAM accounting for the
// low-memory warning in the stats log, EEPROM inspection helpers used while
// debugging, compile-time array helpers, padded printing for the serial help
// tables, and the powerPin class that every switched load on the board is
// driven through. Header-only, and defines functions and objects rather than
// only declaring them, so it is included exactly once (from includes.h).

#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>
#include <EEPROM.h>

// Returns the number of bytes of SRAM currently unused on the ATmega2560.
//
// Measures the gap between the current stack pointer (approximated by the
// address of a local) and the top of the heap, so the result shrinks as either
// the stack or dynamic allocation grows. Intended for the low-memory warning
// in the stats telegram, not for precise accounting.
inline int getAvailableRAM()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

// Returns the total size of the ATmega2560's SRAM in bytes, for use as the
// denominator when reporting how much free memory is left.
inline int getTotalRam()
{
    return RAMEND - RAMSTART + 1;
}

// Zeroes the first 100 physical bytes of EEPROM.
//
// Debug helper for starting from a blank slate. Note that this does NOT clear
// the saved settings: eeprom_settings_slot is an array index scaled by
// sizeof(saved_settings_t) and by PROMPLUS's 3 redundant copies per cell, so
// the struct actually lives around physical byte 2400 -- see savedSettings.h.
// Use the RESET_SETTINGS command to restore the defaults. Not reachable from
// any serial command.
void eraseEEPROM()
{
    for (int i = 0; i < 100; i++)
    {
        EEPROM.write(i, 0);
    }
}

// Number of EEPROM bytes dumpEEPROM() prints per output row.
#define bytesPerLine 16

// Prints a comma-separated hex dump of the first `lines` rows of EEPROM to `serial`, each
// row holding bytesPerLine bytes and prefixed with its start address.
//
// Debug helper for confirming what the settings struct actually wrote to.
void dumpEEPROM(int lines, Stream &_serial)
{
    char buff[100];
    for (int i = 0; i < lines; i++)
    {
        sprintf(buff, "0x0%02X0: ", i);
        _serial.print(buff);
        for (int j = 0; j < bytesPerLine; j++)
        {
            sprintf(buff, "%02X", EEPROM.read((i * bytesPerLine) + j));
            _serial.print(buff);
            if (j != bytesPerLine - 1)
            {
                _serial.print(",");
            }
        }
        _serial.print("\n");
    }
}

// Source - https://stackoverflow.com/a/1598827
// Posted by Michael Burr, modified by community. License - CC BY-SA 2.5
// Gives the number of elements in an array in a type-safe way.
//
// Unlike a plain sizeof division, this fails to compile if handed a pointer
// instead of an array, which would otherwise silently yield a wrong count.
#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

// Concatinates the arrays `a` and `b` into `out`, copying `a` first.
//
// All three lengths are template parameters, so `out` is required by the
// signature to be exactly the combined length and the sizes must be known at
// compile time. Callers must supply the destination -- nothing is allocated.
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

// Prints `str` to `s`, then pads with `pad_char` until `length` characters
// have been written in total.
//
// Used to line up the columns of the serial help tables. `str` must live in
// flash -- pass an F() string -- and no padding is added if it is already
// `length` characters or longer.
void print_padded(Stream &s, const __FlashStringHelper *str, int length, char pad_char = ' ')
{
    s.print(str);
    int str_len = strlen_P(reinterpret_cast<PGM_P>(str));
    for (int i = 0; i < length - str_len; ++i)
    {
        s.print(pad_char);
    }
}

// Returns a human-readable name for `level`, using the three-level vocabulary
// the ISMS exposes to operators through the DEBUG_LOGGING command.
//
// Only LVL_INFO, LVL_DEBUG and LVL_TRACE are ever set by this firmware, since
// anything below LVL_INFO would also suppress the stats telegram. The lower
// levels are mapped to strings that mark them as states that should not occur.
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

// A single digital output driving a switched load: an indicator LED or the
// power relay for a pump, the accessory port, or another rail.
//
// Wraps pinMode() and digitalWrite() and caches the last commanded level, so
// the stats telegram can report what each rail was told to do without reading
// the pin back. Instances are created at global scope in includes.h before the
// Arduino core is running, so begin() should be called from setup() before the
// pin is driven. turnOn() and turnOff() apply pinMode(OUTPUT) themselves if it
// has not happened yet, so a pin driven before begin() still drives rather
// than silently enabling its pull-up -- but begin() is what gives a rail a
// defined state at boot, so keep calling it.
class powerPin
{
public:
    // Records `pinNum` as the pin to drive and starts in the off state. Does
    // not touch the hardware -- the Arduino core may not be running yet -- so
    // the pin is configured by whichever of begin(), turnOn() or turnOff()
    // runs first.
    explicit powerPin(const uint8_t pinNum) : pinNum(pinNum)
    {
        state = false;
        initilized = false;
    }

    // Configures the pin as an output and drives it to `defaultState`, either
    // HIGH or LOW, leaving the cached state consistent with the pin.
    void begin(const uint8_t defaultState)
    {
        initilized = true;
        pinMode(pinNum, OUTPUT);
        if (defaultState == HIGH)
            turnOn();
        else
            turnOff();
    }

    // Drives the pin HIGH, energizing the attached load.
    void turnOn()
    {
        if (!initilized)
        {
            initilized = true;
            pinMode(pinNum, OUTPUT);
        }
        digitalWrite(pinNum, HIGH);
        state = HIGH;
    }

    // Drives the pin LOW, de-energizing the attached load.
    void turnOff()
    {
        if (!initilized)
        {
            initilized = true;
            pinMode(pinNum, OUTPUT);
        }
        digitalWrite(pinNum, LOW);
        state = LOW;
    }

    // Returns the level last commanded on this pin, HIGH or LOW. This is the
    // cached value, not a fresh read of the hardware.
    uint8_t getState()
    {
        return state;
    }

private:
    uint8_t state;   // Level last commanded on the pin, HIGH or LOW.
    bool initilized; // True once pinMode(OUTPUT) has been applied, by begin() or by the first turnOn()/turnOff().
    uint8_t pinNum;  // Arduino pin number this instance drives.
};

#endif // UTIL_H
