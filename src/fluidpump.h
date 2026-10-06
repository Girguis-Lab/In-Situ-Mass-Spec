// Fluid pump control for the ISMS sample pump.
//
// Drives a Micropump EagleDrive gear pump through three signals -- a power
// relay (a powerPin), an analog 0-5 V speed reference, and a direction line --
// and presents them as one signed percentage, so the FLUIDPUMP and
// FLUIDPUMP_RATE serial commands and the automatic startup sequence can set
// flow in a single call. Header-only; the single global instance lives in
// includes.h.
//
// Pump manual:
// https://micropump-nova.s3.us-west-2.amazonaws.com/687/Micropump-EagleDrive-v1_5-Install-Operation-Warranty-Manual.pdf

#ifndef FLUIDPUMP_H
#define FLUIDPUMP_H

// Milliseconds the speed line must be held near full scale after power-on to
// put the pump into PWM control mode. Applies only to the PWM control path,
// which loopHandler() currently leaves disabled.
#define FLUIDPUMP_PWM_STARTUP_DURRATION 2500

// Value setSpeed() returns when init() has not been called yet. Deliberately
// outside the valid -100 to 100 range so callers can tell it apart from a real
// speed.
#define FLUID_PUMP_UNINITIALIZED -999

#include <Arduino.h>

// Controls the fluid pump's power, speed, and direction.
//
// The pump is driven from the analog speed reference, which needs no enable
// handshake: the power relay is closed and a 0-5 V level sets the flow rate.
// The manufacturer's alternative PWM control mode instead requires the speed
// line to be held high for the first FLUIDPUMP_PWM_STARTUP_DURRATION
// milliseconds of every power-on, and that sequencing is kept -- commented
// out -- in loopHandler() in case the wiring moves back to PWM.
class FluidPump
{
public:
    // Binds the pump to the power relay `pumpPower` and to the `pwmPin` speed
    // output and `reverseDirectionPin` direction output.
    //
    // Records the pins and starts stopped; no hardware is touched until init()
    // runs, so this is safe to use for a global instance.
    FluidPump(powerPin *pumpPower, uint8_t pwmPin, uint8_t reverseDirectionPin)
        : pumpPower(pumpPower), pwmPin(pwmPin), reverseDirectionPin(reverseDirectionPin), initialized(false), pumpEnableComplete(false), timeWhenEnabled(0), currentSpeedPercent(0) {}

    // Prepares the pump's pins and leaves the pump stopped.
    //
    // Configures the speed and direction pins as outputs, forces the speed to
    // 0 (which also opens the power relay, so a reboot mid-run does not leave
    // the pump spinning), and marks the pump usable by setSpeed(). Call once
    // from setup(), before any other method.
    void init()
    {
        pumpPower->begin(LOW);
        pinMode(pwmPin, OUTPUT);
        pinMode(reverseDirectionPin, OUTPUT);

        _setSpeed(0); // Initialize pump pwm to fully off
        initialized = true;
    }

    // Placeholder for the per-loop PWM enable sequencing; currently does
    // nothing and is not called.
    //
    // While the pump was driven over PWM this had to run every few
    // milliseconds: on each power-on it held the speed line at full scale for
    // FLUIDPUMP_PWM_STARTUP_DURRATION to enable PWM control mode, then applied
    // the requested speed. The analog speed reference in use today needs no
    // such handshake, so the body is retained only as a reference for anyone
    // rewiring the pump back to PWM.
    void loopHandler()
    {
        // loop is currently disabled because we are using the analog instead of pwm control
        // -----------------------------------------------------------
        // if (!initialized)
        //     return; // Not initialized or not enabled
        // if (currentSpeedPercent == 0)
        // {
        //     timeWhenEnabled = 0;
        //     _setSpeed(0);
        //     return;
        // }
        // else if (timeWhenEnabled == 0)
        // {
        //     // Startup procedure calls for a near 100% duty cycle for a set durration on the PWM line to enable pwm control
        //     _setSpeed(100);
        //     // record when startup begins
        //     timeWhenEnabled = millis();
        //     pumpEnableComplete = false;
        //     return;
        // }
        // else if (timeWhenEnabled > FLUIDPUMP_PWM_STARTUP_DURRATION && !pumpEnableComplete)
        // {
        //     // when the pump enable time is finished, actually set the pump to the desired speed
        //     _setSpeed(abs(currentSpeedPercent));
        //     pumpEnableComplete = true;
        // }
    }

    // Returns the speed last commanded through setSpeed(), as a percentage of
    // full speed from -100 to 100, where a negative value means the pump is
    // running in reverse and 0 means it is stopped and unpowered.
    int getSpeed() const
    {
        return currentSpeedPercent;
    }

    // Sets the pump's direction and speed from a single signed percentage.
    //
    // `percent` is a fraction of full speed from -100 to 100; negative values
    // run the pump in reverse and 0 stops it and opens its power relay. Values
    // above 100 are clamped. Returns the speed actually applied, or
    // FLUID_PUMP_UNINITIALIZED if init() has not been called yet, in which
    // case nothing is driven.
    int setSpeed(int percent)
    {
        if (!initialized)
            return FLUID_PUMP_UNINITIALIZED; // Not initialized
        if (percent < -100)
            percent = -100;
        if (percent > 100)
            percent = 100;
        if (percent < 0)
        {
            runInReverseDir();
        }
        else
        {
            runInForwardDir();
        }
        currentSpeedPercent = percent;
        // loopHandler();
        _setSpeed(currentSpeedPercent);
        return currentSpeedPercent;
    }

    // Selects reverse rotation by grounding the direction line, as the pump
    // manual specifies. Takes effect on the next speed change.
    void runInReverseDir()
    {
        /// ground the pin per micropump documentation
        pinMode(reverseDirectionPin, OUTPUT);
        digitalWrite(reverseDirectionPin, LOW);
    }

    // Selects forward rotation by floating the direction line -- switching the
    // pin to an input rather than driving it high -- as the pump manual
    // specifies.
    void runInForwardDir()
    {
        /// float the pin per micropump documentation
        pinMode(reverseDirectionPin, INPUT);
    }

    // Applies the magnitude of `percent` to the speed line and switches pump
    // power to match. Internal; setSpeed() is the entry point callers use.
    //
    // The absolute value of `percent` (0 to 100) is mapped onto the 0-255
    // analog output. The power relay is opened at 0 and closed otherwise, so
    // the pump is never left powered but commanded to a standstill. Direction
    // is not touched here.
    void _setSpeed(int percent)
    {
        uint8_t pwmValue = map(abs(percent), 0, 100, 0, 255);
        if (percent == 0)
            pumpPower->turnOff();
        else
            pumpPower->turnOn();
        analogWrite(pwmPin, pwmValue);
    }

private:
    powerPin *pumpPower;           // Relay supplying the pump; never null.
    uint8_t pwmPin;                // Analog speed reference output.
    uint8_t reverseDirectionPin;   // Direction line: grounded reverses.
    bool initialized;              // True once init() has configured the pins.
    bool pumpEnableComplete;       // PWM mode only: enable hold has finished.
    unsigned long timeWhenEnabled; // PWM mode only: millis() at power-on.
    int currentSpeedPercent;       // Last commanded speed, -100 to 100.
};

#endif // FLUIDPUMP_H
