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

int logLevelToInt(const char *levelStr)
{
    if (strncasecmp(levelStr, "SILENT", 6) == 0)
        return LOG_LEVEL_SILENT;
    else if (strncasecmp(levelStr, "FATAL", 5) == 0)
        return LOG_LEVEL_FATAL;
    else if (strncasecmp(levelStr, "ERROR", 5) == 0)
        return LOG_LEVEL_ERROR;
    else if (strncasecmp(levelStr, "WARNING", 7) == 0)
        return LOG_LEVEL_WARNING;
    else if (strncasecmp(levelStr, "INFO", 4) == 0)
        return LOG_LEVEL_INFO;
    else if (strncasecmp(levelStr, "NOTICE", 6) == 0)
        return LOG_LEVEL_NOTICE; // NOTICE is an alias for INFO
    else if (strncasecmp(levelStr, "TRACE", 5) == 0)
        return LOG_LEVEL_TRACE;
    else if (strncasecmp(levelStr, "DEBUG", 5) == 0)
        return LOG_LEVEL_TRACE; // DEBUG is an alias for TRACE
    else if (strncasecmp(levelStr, "VERBOSE", 7) == 0)
        return LOG_LEVEL_VERBOSE;
    else
        return -1; // Invalid log level
}

String logLevelToString(int level)
{
    switch (level)
    {
    case LOG_LEVEL_SILENT:
        return "SILENT";
    case LOG_LEVEL_FATAL:
        return "FATAL";
    case LOG_LEVEL_ERROR:
        return "ERROR";
    case LOG_LEVEL_WARNING:
        return "WARNING";
    case LOG_LEVEL_INFO:
        return "INFO";
    case LOG_LEVEL_TRACE:
        return "TRACE";
    case LOG_LEVEL_VERBOSE:
        return "VERBOSE";
    default:
        return "UNKNOWN";
    }
}

#endif // UTIL_H
