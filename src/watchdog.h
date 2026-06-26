#ifndef WATCHDOG_H
#define WATCHDOG_H

// ---------------------------------------------------------
// -------------------- Watchdog Timer ---------------------
// ---- Resets the board if it becomes unresponsive --------
// ---------------------------------------------------------

#include <Arduino.h>
#include <stdint.h>
#include <avr/wdt.h>

boolean wdtOn = false;

// The following is needed to capture the watchdog reset reason for later use, it is coppied from avr/wdt.h
uint8_t mcusr_mirror __attribute__((section(".noinit")));
void get_mcusr(void)
    __attribute__((naked))
    __attribute__((section(".init3")));
void get_mcusr(void)
{
    mcusr_mirror = reinterpret_cast<uint8_t>(MCUSR);
    MCUSR = 0;
    wdt_disable();
}

// Enable the watchdog timer with an 8-second timeout
void wdtStart()
{
    wdtOn = true;
    wdt_enable(WDTO_8S);
}

// Disable the watchdog timer
void wdtStop()
{
    wdtOn = false;
    wdt_disable();
}

// Reset the watchdog timer to prevent a system reset
void resetWDT()
{
    if (wdtOn)
    {
        wdt_reset();
    }
}

void wdtPrintStatus(Stream &Serial)
{
    if (mcusr_mirror & (1 << BORF))
    {
        Serial.println(F("| STARTUP: Normal, Brown-out occurred - This is expected on power-on."));
    }
    else if (mcusr_mirror & (1 << WDRF))
    {
        Serial.println(F("| STARTUP: Watchdog timeout occurred"));
    }
    else if (mcusr_mirror & (1 << EXTRF))
    {
        Serial.println(F("| STARTUP: External reset triggered"));
    }
    else if (mcusr_mirror & (1 << PORF))
    {
        Serial.println(F("| STARTUP: Power-on reset occurred"));
    }
    else
    {
        Serial.println(F("| STARTUP: Normal, no reset occurred"));
    }
}
#endif // WATCHDOG_H
