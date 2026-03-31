#ifndef PFEIFFER_SERIAL_H
#define PFEIFFER_SERIAL_H

#include <Arduino.h>
#include <ArduinoLog.h> // This library depebds on arduino log.
#include "PfeifferVacProtocolConsts.h"
#include "PfeifferVacProtocolFuncs.h"
#include "PfeifferVacProtocolDatatypes.h"

class PfeifferSerialTC80
{
public:
    typedef void (*DelayFunc)(unsigned long);
    typedef bool (*CheckFunc)();

    PfeifferSerialTC80(HardwareSerial &turboSerial, const uint16_t address = 1, Stream &debug = Serial, DelayFunc delayFunc = ::delay, uint8_t pin485DriverEnable = 0, uint8_t pin485ReciverDisable = 0)
        : _turboSerial(turboSerial), _debug(debug), _delayFunc(delayFunc), _pin485SendEnable(pin485DriverEnable), _pin485ReceiveDisable(pin485ReciverDisable)
    {
        sprintf(_address, "%03d", address);
        _checkFunc = nullptr;
    }

    void begin(unsigned long baud = 9600, uint32_t config = SERIAL_8N1, CheckFunc checkFunc = nullptr)
    {
        _checkFunc = checkFunc;
        if (_pin485SendEnable != 0)
            pinMode(_pin485SendEnable, OUTPUT);
        if (_pin485ReceiveDisable != 0)
            pinMode(_pin485ReceiveDisable, OUTPUT);
        _enable485Receiver();
        _turboSerial.begin(baud, config); // default: 9600 baud, 8 data bits, no parity, 1 stop bit
    }

    void setAddress(const uint16_t address)
    {
        snprintf(_address, 4, "%03d", address);
    }

    const char *getAddress() const
    {
        return _address;
    }

    void _enable485Receiver()
    {
        if (_pin485SendEnable != 0)
            digitalWrite(_pin485SendEnable, LOW);

        if (_pin485ReceiveDisable != 0)
            digitalWrite(_pin485ReceiveDisable, LOW);
    }

    void _enable485Sender()
    {
        if (_pin485SendEnable != 0)
            digitalWrite(_pin485SendEnable, HIGH);

        if (_pin485ReceiveDisable != 0)
            digitalWrite(_pin485ReceiveDisable, HIGH);
    }

    // Send a raw telegram string
    void _sendTelegramRaw(const char *telegram)
    {
        _enable485Sender();
        _turboSerial.print(telegram);
        _turboSerial.print('\r');
        _turboSerial.flush();
        _enable485Receiver();
    }

    // Send a telegram using action, parameter, and data
    void _sendTelegram(const char action, const uint16_t parameter, const char *data = nullptr, const bool debugPrint = false)
    {
        if (debugPrint)
        {
            String telegramStr = PfeifferVacProtocol::encodeTelegram(_address, action, parameter, data);
            Log.trace("Raw: ");
            Log.traceln(telegramStr.c_str());
            PfeifferVacProtocol::PfeifferTelegram telegram = PfeifferVacProtocol::decodeTelegram(telegramStr.c_str());
            PfeifferVacProtocol::printTelegramHumanReadable(telegram, _debug);
        }

        // Check function is designed to allow the user to back out if they actually don't want to send the command
        // if (_checkFunc && _checkFunc() == false)
        //     return;

        String telegramStr = PfeifferVacProtocol::encodeTelegram(_address, action, parameter, data);
        _sendTelegramRaw(telegramStr.c_str());
    }

    // Receive a raw telegram (non-blocking, returns String)
    String _receiveTelegramRaw(unsigned long timeout = 1000)
    {
        _enable485Receiver();
        String frame = "";
        unsigned long start = millis();
        while (millis() - start < timeout)
        {
            _delayFunc(1); // Small delay to avoid busy wait
            if (_turboSerial.available())
            {
                char c = _turboSerial.read();
                if (c == '\r')
                    break;
                frame += c;
            }
        }
        return frame;
    }

    // Receive and decode a telegram (non-blocking)
    PfeifferVacProtocol::PfeifferTelegram receiveTelegram(bool debugPrint = false, unsigned long timeout = 1000)
    {
        String raw = _receiveTelegramRaw(timeout);
        PfeifferVacProtocol::PfeifferTelegram telegram = PfeifferVacProtocol::decodeTelegram(raw.c_str());
        if (debugPrint)
        {
            _debug.print("Turbo Received: (RAW=");
            _debug.print(raw);
            _debug.print(") ");
            PfeifferVacProtocol::printTelegramHumanReadable(telegram, _debug);
        }
        return telegram;
    }

    void sendQuery(const uint16_t parameter, const bool debugPrint = false)
    {
        _sendTelegram('0', parameter, "=?", debugPrint);
    }

    void sendCommand(const uint16_t parameter, const char *data, const bool debugPrint = false)
    {
        _sendTelegram('1', parameter, data, debugPrint);
    }

    //
    void sendCommand(const uint16_t parameter, const uint8_t data, const bool debugPrint = false)
    {
        _sendTelegram('1', parameter, String(data).c_str());
    }

    // 0: boolean_old
    void sendCommand(const uint16_t parameter, const PfeifferVacProtocol::BooleanOld data, const bool debugPrint = false, bool receive = true)
    {
        _sendTelegram('1', parameter, data.encode().c_str(), debugPrint);
    }

    // 1: u_integer
    void sendCommand(const uint16_t parameter, const PfeifferVacProtocol::UInteger data, const bool debugPrint = false)
    {
        _sendTelegram('1', parameter, data.encode().c_str(), debugPrint);
    }

    // 2: u_real
    void sendCommand(const uint16_t parameter, const PfeifferVacProtocol::UReal data, const bool debugPrint = false)
    {
        _sendTelegram('1', parameter, data.encode().c_str(), debugPrint);
    }

    // 4: string (6 chars)
    void sendCommand(const uint16_t parameter, const PfeifferVacProtocol::String6 data, const bool debugPrint = false)
    {
        _sendTelegram('1', parameter, data.encode().c_str(), debugPrint);
    }

    // 6: boolean_new
    void sendCommand(const uint16_t parameter, const PfeifferVacProtocol::BooleanNew data, const bool debugPrint = false)
    {
        _sendTelegram('1', parameter, data.encode().c_str(), debugPrint);
    }

    // 7: u_short_int
    void sendCommand(const uint16_t parameter, const PfeifferVacProtocol::UShortInt data, const bool debugPrint = false)
    {
        _sendTelegram('1', parameter, data.encode().c_str(), debugPrint);
    }

    // 11: string16
    void sendCommand(const uint16_t parameter, const PfeifferVacProtocol::String16 data, const bool debugPrint = false)
    {
        _sendTelegram('1', parameter, data.encode().c_str(), debugPrint);
    }

    // 12: string8
    void sendCommand(const uint16_t parameter, const PfeifferVacProtocol::String8 data, const bool debugPrint = false)
    {
        _sendTelegram('1', parameter, data.encode().c_str(), debugPrint);
    }

    // 10: u_expo_new
    void sendCommand(const uint16_t parameter, const PfeifferVacProtocol::UExpoNew data, const bool debugPrint = false)
    {
        _sendTelegram('1', parameter, data.encode().c_str(), debugPrint);
    }

    // =====================================================================
    // receiving functions for the datatypes
    // =====================================================================
    // Helper to check if a response is complete & valid
    inline bool isValidResponse(const PfeifferVacProtocol::PfeifferTelegram &telegram, uint16_t expectedParameter)
    {
        // A standard successful response has the same source address and a valid checksum
        return telegram.address == _address &&
               telegram.checksumValid && (uint16_t)telegram.parameter.toInt() == expectedParameter;
    }

    // A utility function to output debug messages (assuming _debug is available)
    inline void _printInvalidResponseDebug(bool debugPrint, const PfeifferVacProtocol::PfeifferTelegram telegram, uint16_t expectedParameter)
    {
        if (debugPrint)
        {
            // Assuming _debug.print is available or a similar logging mechanism
            _debug.print("INVALID Response Received: ");
            if (strcmp(telegram.address.c_str(), _address) == 0)
            {
                _debug.print("Address mismatch.");
            }
            else if (!telegram.checksumValid)
            {
                _debug.print("Checksum invalid.");
            }
            else if ((uint8_t)telegram.parameter.toInt() != expectedParameter)
            {
                _debug.print(String("Expected parameter ") + expectedParameter + " got " + telegram.parameter);
            }
            else
            {
                _debug.print("Data length mismatch or unexpected content.");
            }
        }
    }

    // 0: boolean_old (Native Type: boolean)
    boolean receiveBooleanOld(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout);
        if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 6)
        {
            isValid = true;
            return PfeifferVacProtocol::BooleanOld(telegram.data.c_str()).decode();
        }
        else
        {
            isValid = false;
            _printInvalidResponseDebug(debugPrint, telegram, expectedParameter);
            return false; // Default/invalid return
        }
    }

    // 1: u_integer (Native Type: uint32_t)
    uint32_t receiveUInteger(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout);
        if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 6)
        {
            isValid = true;
            return PfeifferVacProtocol::UInteger(telegram.data.c_str()).decode();
        }
        else
        {
            isValid = false;
            _printInvalidResponseDebug(debugPrint, telegram, expectedParameter);
            return 0; // Default/invalid return
        }
    }

    // 2: u_real (Native Type: float)
    float receiveUReal(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout);
        if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 6)
        {
            isValid = true;
            return PfeifferVacProtocol::UReal(telegram.data.c_str()).decode();
        }
        else
        {
            isValid = false;
            _printInvalidResponseDebug(debugPrint, telegram, expectedParameter);
            return 0.0f; // Default/invalid return
        }
    }

    // 6: boolean_new (Native Type: boolean)
    boolean receiveBooleanNew(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout);
        if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 1)
        {
            isValid = true;
            return PfeifferVacProtocol::BooleanNew(telegram.data.c_str()).decode();
        }
        else
        {
            isValid = false;
            _printInvalidResponseDebug(debugPrint, telegram, expectedParameter);
            return false; // Default/invalid return
        }
    }

    // 7: u_short_int (Native Type: uint16_t)
    uint16_t receiveUShortInt(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout);
        if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 3)
        {
            isValid = true;
            return PfeifferVacProtocol::UShortInt(telegram.data.c_str()).decode();
        }
        else
        {
            isValid = false;
            _printInvalidResponseDebug(debugPrint, telegram, expectedParameter);
            return 0; // Default/invalid return
        }
    }

    // 10: u_expo_new (Native Type: float)
    float receiveUExpoNew(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout);
        if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 6)
        {
            isValid = true;
            return PfeifferVacProtocol::UExpoNew(telegram.data.c_str()).decode();
        }
        else
        {
            isValid = false;
            _printInvalidResponseDebug(debugPrint, telegram, expectedParameter);
            return 0.0f; // Default/invalid return
        }
    }

    // ----------------------------------------
    // ------ String Receive Functions --------
    // ----------------------------------------

    // 4: string (6 chars) (Native Type: String)
    String receiveString6(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout);
        if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 6)
        {
            isValid = true;
            // Decode returns const char*, which is implicitly convertible to Arduino String
            return String(PfeifferVacProtocol::String6(telegram.data.c_str()).decode());
        }
        else
        {
            isValid = false;
            _printInvalidResponseDebug(debugPrint, telegram, expectedParameter);
            return ""; // Return empty String on invalid response
        }
    }

    // 11: string16 (Native Type: String)
    String receiveString16(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout);
        if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 16)
        {
            isValid = true;
            return String(PfeifferVacProtocol::String16(telegram.data.c_str()).decode());
        }
        else
        {
            isValid = false;
            _printInvalidResponseDebug(debugPrint, telegram, expectedParameter);
            return "";
        }
    }

    // 12: string8 (Native Type: String)
    String receiveString8(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout);
        if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 8)
        {
            isValid = true;
            return String(PfeifferVacProtocol::String8(telegram.data.c_str()).decode());
        }
        else
        {
            isValid = false;
            _printInvalidResponseDebug(debugPrint, telegram, expectedParameter);
            return "";
        }
    }

private:
    HardwareSerial &_turboSerial;
    uint8_t _pin485SendEnable;
    uint8_t _pin485ReceiveDisable;
    char _address[4]; // 3 digits + null terminator address of the turbo pump
    Stream &_debug;
    DelayFunc _delayFunc;
    CheckFunc _checkFunc;
};

#endif // PFEIFFER_SERIAL_H
