// Hardware watchdog timer support for the ISMS controller.
//
// The ATmega2560's watchdog resets the board if nothing pets it for 8 seconds,
// which recovers the instrument from a hang or a lockup on the turbo pump's
// RS485 bus with nobody on deck to power-cycle it. Because a watchdog reset
// looks like any other reboot from the outside, this file also captures MCUSR
// at boot -- before the C runtime clears it -- so setup() can report why the
// board last restarted. Header-only, and defines objects, so it is included
// exactly once (from includes.h).

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <Arduino.h>
#include <stdint.h>
#include <avr/wdt.h>

// True while the watchdog is enabled. Gates resetWDT() so that petting a
// disabled watchdog is harmless.
boolean wdtOn = false;

// Copy of MCUSR taken at boot, holding the flags that say what caused the last
// reset. Placed in .noinit so it is not cleared by the C runtime's startup
// code, which would otherwise wipe it before setup() could read it.
uint8_t mcusr_mirror __attribute__((section(".noinit")));

// Mirrors MCUSR into mcusr_mirror and disables the watchdog before main().
//
// Runs from .init3 and is declared naked so it executes ahead of the Arduino
// core and of any runtime setup that would clear the reset flags; disabling
// the watchdog here also keeps a short timeout from re-triggering during a
// slow startup. wdtPrintStatus() reads the mirrored value later. Registered
// via its section attributes -- never call it directly. Pattern taken from the
// example in avr/wdt.h.
void get_mcusr(void)
    __attribute__((naked))
    __attribute__((section(".init3")));
void get_mcusr(void)
{
    mcusr_mirror = reinterpret_cast<uint8_t>(MCUSR);
    MCUSR = 0;
    wdt_disable();
}

// Enables the watchdog with an 8 second timeout.
//
// Once this returns, resetWDT() must be called at least that often or the
// board reboots. Every waiting routine in this firmware goes through
// nonBlockDelay(), which pets the watchdog via taskTick(), so the requirement
// is met as long as new code waits the same way.
void wdtStart()
{
    wdtOn = true;
    wdt_enable(WDTO_8S);
}

// Disables the watchdog, so a hung loop will no longer reset the board.
// Intended for debugging a hang, not for normal operation.
void wdtStop()
{
    wdtOn = false;
    wdt_disable();
}

// Pets the watchdog, restarting its 8 second countdown. Does nothing if the
// watchdog is not currently enabled.
void resetWDT()
{
    if (wdtOn)
    {
        wdt_reset();
    }
}

// Prints a one-line explanation of why the board last reset to `serial`.
//
// Decodes the boot-time copy of MCUSR captured by get_mcusr(). A brown-out
// flag is expected on power-on, whereas a watchdog flag means the previous run
// stopped petting the timer and was reset -- worth investigating in a
// deployment log. Called once from setup(), which passes the operator serial
// port.
//
// Every set flag is reported, not just the first one found. MCUSR's flags are
// cumulative until something clears them, and more than one is set routinely:
// a watchdog reset during a supply sag sets both WDRF and BORF. Reporting only
// the first match used to hide the watchdog behind the expected brown-out
// message, suppressing the single most diagnostically useful fact there is --
// that the firmware hung.
void wdtPrintStatus(Stream &serial)
{
    serial.print(F("| STARTUP: last reset flags:"));
    if (mcusr_mirror & (1 << WDRF))
        serial.print(F(" WATCHDOG-TIMEOUT(firmware hung - investigate)"));
    if (mcusr_mirror & (1 << BORF))
        serial.print(F(" Brown-out(expected on power-on)"));
    if (mcusr_mirror & (1 << EXTRF))
        serial.print(F(" External-reset"));
    if (mcusr_mirror & (1 << PORF))
        serial.print(F(" Power-on"));
    if ((mcusr_mirror & ((1 << WDRF) | (1 << BORF) | (1 << EXTRF) | (1 << PORF))) == 0)
        serial.print(F(" none recorded"));
    serial.println();
}
#endif // WATCHDOG_H
