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

#endif // UTIL_H
