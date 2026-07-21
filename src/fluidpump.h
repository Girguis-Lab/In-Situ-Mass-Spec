#ifndef FLUIDPUMP_H
#define FLUIDPUMP_H

#define FLUIDPUMP_PWM_STARTUP_DURRATION 2500
#define FLUID_PUMP_UNINITIALIZED -999

#include <Arduino.h>

// https://micropump-nova.s3.us-west-2.amazonaws.com/687/Micropump-EagleDrive-v1_5-Install-Operation-Warranty-Manual.pdf
// Note that the pump startup requires the PWM wire to be high for first 2.5 seconds of power on in order to enable PWM control mode
class FluidPump
{
public:
    FluidPump(powerPin *pumpPower, uint8_t pwmPin, uint8_t reverseDirectionPin)
        : pumpPower(pumpPower), pwmPin(pwmPin), reverseDirectionPin(reverseDirectionPin), initialized(false), pumpEnableComplete(false), timeWhenEnabled(0), currentSpeedPercent(0) {}

    /* Init sets up the pump for operation, it should be called early on in your code before any other methods are called */
    void init()
    {
        pumpPower->begin(LOW);
        pinMode(pwmPin, OUTPUT);
        pinMode(reverseDirectionPin, OUTPUT);

        _setSpeed(0); // Initialize pump pwm to fully off
        initialized = true;
    }

    /*
    Loop handler should be called in your main program loop fairly frequently - on the order of ms.
    It's job is to ensure that the electrical startup conditions for the fluid pump are met whenever the fluid pump is turned on.
    */
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

    // Returns the currently set speed of the pump as a percentage of full speed
    int getSpeed() const
    {
        return currentSpeedPercent;
    }

    // Set pump speed as a percentage of full speed (-100 - 100)
    int setSpeed(int percent)
    {
        if (!initialized)
            return FLUID_PUMP_UNINITIALIZED; // Not initialized
        if (percent < -100)
            percent = 100;
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

    void runInReverseDir()
    {
        /// ground the pin per micropump documentation
        pinMode(reverseDirectionPin, OUTPUT);
        digitalWrite(reverseDirectionPin, LOW);
    }

    void runInForwardDir()
    {
        /// float the pin per micropump documentation
        pinMode(reverseDirectionPin, INPUT);
    }

    // Set raw PWM speed as percentage, enabling or disabling pump power as necessary (0-100)
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
    powerPin *pumpPower;
    uint8_t pwmPin;
    uint8_t reverseDirectionPin;
    bool initialized;
    bool pumpEnableComplete;
    unsigned long timeWhenEnabled; // in miliseconds
    int currentSpeedPercent;
};

#endif // FLUIDPUMP_H
