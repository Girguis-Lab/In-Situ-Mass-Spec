#ifndef PFEIFFER_SERIAL_H
#define PFEIFFER_SERIAL_H

#include <Arduino.h>
#include <DebugLog.h> // This library depends on the debug log library.
#include "PfeifferVacProtocolConsts.h"
#include "PfeifferVacProtocolFuncs.h"
#include "PfeifferVacProtocolDatatypes.h"
#include "RS485Serial.h"

// by default this library uses the arduino delay() function, but this can be overwridden with the passed parameter.
// if the delay function returns true, the library will inmediately exit any activities that may be holding up the porcessor at the cost of not reciving a full message.
bool defaultDelayFunc(unsigned long ms)
{
    delay(ms);
    return false;
}

class PfeifferSerialTC80
{
public:
    typedef bool (*DelayFunc)(unsigned long);
    typedef bool (*CheckFunc)();

    explicit PfeifferSerialTC80(RS485Serial_Base &turboSerial, const uint16_t address = 1, Stream &debug = Serial, DelayFunc delayFunc = defaultDelayFunc)
        : _turboSerial(turboSerial), _debug(debug), _delayFunc(delayFunc)
    {
        sprintf(_address, "%03d", address);
    }

    void begin(unsigned long baud = 9600, uint32_t config = SERIAL_8N1)
    {
        _turboSerial.begin(baud, config); // RS485Serial class handles pin setup and initialization
    }

    void setAddress(const uint16_t address)
    {
        snprintf(_address, 4, "%03d", address);
    }

    const char *getAddress() const
    {
        return _address;
    }

    // Send a raw telegram string
    void _sendTelegramRaw(const char *telegram)
    {
        _turboSerial.write(reinterpret_cast<const uint8_t *>(telegram), strlen(telegram));
        _turboSerial.write('\r');
        // The RS485Serial class automatically handles direction switching.
    }

    void printOutgoingTelegram(const char action, const uint16_t parameter, const char *data = nullptr)
    {
        String telegramStr = PfeifferVacProtocol::encodeTelegram(_address, action, parameter, data);
        PfeifferVacProtocol::PfeifferTelegram telegram = PfeifferVacProtocol::decodeTelegram(telegramStr.c_str());
        PfeifferVacProtocol::printTelegramHumanReadable(telegram, true);
        LOG_TRACE("[Raw Telegram: '");
        LOG_TRACE(telegramStr.c_str());
        LOG_TRACE("']\n");
    }

    // Send a telegram using action, parameter, and data
    void _sendTelegram(const char action, const uint16_t parameter, const char *data = nullptr, const bool debugPrint = false)
    {
        if (debugPrint || LOG_GET_LEVEL() >= DebugLogLevel::LVL_DEBUG)
        {
            printOutgoingTelegram(action, parameter, data);
        }

        String telegramStr = PfeifferVacProtocol::encodeTelegram(_address, action, parameter, data);
        _sendTelegramRaw(telegramStr.c_str());
    }

    // Receive a raw telegram (non-blocking, returns String)
    String _receiveTelegramRaw(unsigned long timeout = 20)
    {
        String frame = "";
        unsigned long start = millis();
        while (millis() - start < timeout)
        {
            _turboSerial.task(); // Call task() to handle buffering and direction control
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

    // Receive and decode a telegram
    PfeifferVacProtocol::PfeifferTelegram receiveTelegram(bool debugPrint = false, unsigned long timeout = 5000)
    {
        String raw = _receiveTelegramRaw(timeout);
        PfeifferVacProtocol::PfeifferTelegram telegram = PfeifferVacProtocol::decodeTelegram(raw.c_str());
        if (debugPrint || LOG_GET_LEVEL() >= DebugLogLevel::LVL_DEBUG)
        {
            PfeifferVacProtocol::printTelegramHumanReadable(telegram, false);
            LOG_TRACE("\n[RAW Received Telegram is '");
            LOG_TRACE(raw.c_str());
            LOG_TRACE("'] ");
            LOG_INFO("\n");
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

    // 0: boolean_old
    void sendCommand(const uint16_t parameter, const PfeifferVacProtocol::BooleanOld data, const bool debugPrint = false)
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
        // A standard successful response has the same source address and no Errors
        return strcmp(telegram.address.c_str(), _address) == 0 &&
               telegram.error == PfeifferVacProtocol::TelegramError::None && (uint16_t)telegram.parameter.toInt() == expectedParameter;
    }

    // A utility function to output debug messages (assuming _debug is available)
    inline void _printInvalidResponseDebug(const PfeifferVacProtocol::PfeifferTelegram telegram, uint16_t expectedParameter)
    {
        // Assumes LOG_DEBUG is available or a similar logging mechanism
        if (telegram.address.length() == 0 && telegram.parameter.length() == 0)
        {
            return; // no message was recived, ignore.
        }
        else if (strcmp(telegram.address.c_str(), _address) != 0)
        {
            LOG_DEBUG(F("!WARN: Address mismatch in Turbo response. Expected: "), _address, ", Received: ", telegram.address.c_str(), "\n");
        }
        else if (telegram.error == PfeifferVacProtocol::TelegramError::LogicError)
        {
            LOG_DEBUG(F("!WARN: Turbo pump internal logic error.\n"));
        }
        else if (telegram.error == PfeifferVacProtocol::TelegramError::OutOfRange)
        {
            LOG_DEBUG(F("!WARN: Given command value was out of range.\n"));
        }
        else if (telegram.error == PfeifferVacProtocol::TelegramError::InvalidParameter)
        {
            LOG_DEBUG(F("!WARN: Parameter "));
            LOG_DEBUG(telegram.parameter.c_str());
            LOG_DEBUG(F(" is not a valid parameter.\n"));
        }
        else if (telegram.error == PfeifferVacProtocol::TelegramError::InvalidChecksum)
        {
            LOG_DEBUG("!WARN: Checksum invalid or partial Turbo response.\n");
        }
        else if ((uint8_t)telegram.parameter.toInt() != expectedParameter)
        {
            LOG_DEBUG("!WARN: Unexpected parameter in Turbo response: ");
            LOG_DEBUG(telegram.parameter.c_str());
            LOG_DEBUG(" ≠ ");
            LOG_DEBUG(expectedParameter, "\n");
        }
        else
        {
            LOG_DEBUG("!WARN: Data length mismatch or unexpected content in Turbo response.\n");
        }
    }

    // 0: boolean_old (Native Type: boolean)
    boolean receiveBooleanOld(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        unsigned long startTime = millis();
        while (millis() - startTime < timeout)
        {
            const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout / 4);
            if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 6)
            {
                isValid = true;
                return PfeifferVacProtocol::BooleanOld(telegram.data.c_str()).decode();
            }
            else
            {
                isValid = false;

                _printInvalidResponseDebug(telegram, expectedParameter);
                // Small delay before retrying:
                bool exitEarly = _delayFunc(50);
                // break & return if something happened durring the delay which requires exit, such as a received user command
                if (exitEarly)
                    break;
            }
        }
        return false; // Default/invalid return
    }

    // 1: u_integer (Native Type: unsigned long)
    unsigned long receiveUInteger(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        unsigned long startTime = millis();
        while (millis() - startTime < timeout)
        {
            const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout / 4);
            if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 6)
            {
                isValid = true;
                return PfeifferVacProtocol::UInteger(telegram.data.c_str()).decode();
            }
            else
            {
                isValid = false;
                _printInvalidResponseDebug(telegram, expectedParameter);
                // Small delay before retrying:
                bool exitEarly = _delayFunc(50);
                // break & return if something happened durring the delay which requires exit, such as a received user command
                if (exitEarly)
                    break;
            }
        }
        return 0; // Default/invalid return
    }

    // 2: u_real (Native Type: float)
    float receiveUReal(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        unsigned long startTime = millis();
        while (millis() - startTime < timeout)
        {
            const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout / 4);
            if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 6)
            {
                isValid = true;
                return PfeifferVacProtocol::UReal(telegram.data.c_str()).decode();
            }
            else
            {
                isValid = false;
                _printInvalidResponseDebug(telegram, expectedParameter);
                // Small delay before retrying:
                bool exitEarly = _delayFunc(50);
                // break & return if something happened durring the delay which requires exit, such as a received user command
                if (exitEarly)
                    break;
            }
        }
        return 0.0f; // Default/invalid return
    }

    // 6: boolean_new (Native Type: boolean)
    boolean receiveBooleanNew(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        unsigned long startTime = millis();
        while (millis() - startTime < timeout)
        {
            const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout / 4);
            if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 1)
            {
                isValid = true;
                return PfeifferVacProtocol::BooleanNew(telegram.data.c_str()).decode();
            }
            else
            {
                isValid = false;
                _printInvalidResponseDebug(telegram, expectedParameter);
                // Small delay before retrying:
                bool exitEarly = _delayFunc(50);
                // break & return if something happened durring the delay which requires exit, such as a received user command
                if (exitEarly)
                    break;
            }
        }
        return false; // Default/invalid return
    }

    // 7: u_short_int (Native Type: uint16_t)
    uint16_t receiveUShortInt(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        unsigned long startTime = millis();
        while (millis() - startTime < timeout)
        {
            const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout / 4);
            if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 3)
            {
                isValid = true;
                return PfeifferVacProtocol::UShortInt(telegram.data.c_str()).decode();
            }
            else
            {
                isValid = false;
                _printInvalidResponseDebug(telegram, expectedParameter);
                // Small delay before retrying:
                bool exitEarly = _delayFunc(50);
                // break & return if something happened durring the delay which requires exit, such as a received user command
                if (exitEarly)
                    break;
            }
        }
        return 0; // Default/invalid return
    }

    // 10: u_expo_new (Native Type: float)
    float receiveUExpoNew(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        unsigned long startTime = millis();
        while (millis() - startTime < timeout)
        {
            const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout / 4);
            if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 6)
            {
                isValid = true;
                return PfeifferVacProtocol::UExpoNew(telegram.data.c_str()).decode();
            }
            else
            {
                isValid = false;
                _printInvalidResponseDebug(telegram, expectedParameter);
                // Small delay before retrying:
                bool exitEarly = _delayFunc(50);
                // break & return if something happened durring the delay which requires exit, such as a received user command
                if (exitEarly)
                    break;
            }
        }
        return 0.0f; // Default/invalid return
    }

    // ----------------------------------------
    // ------ String Receive Functions --------
    // ----------------------------------------

    // 4: string (6 chars) (Native Type: String)
    String receiveString6(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        unsigned long startTime = millis();
        while (millis() - startTime < timeout)
        {
            const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout / 4);
            if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 6)
            {
                isValid = true;
                // Decode returns const char*, which is implicitly convertible to Arduino String
                return String(PfeifferVacProtocol::String6(telegram.data.c_str()).decode());
            }
            else
            {
                isValid = false;
                _printInvalidResponseDebug(telegram, expectedParameter);
                // Small delay before retrying:
                bool exitEarly = _delayFunc(50);
                // break & return if something happened durring the delay which requires exit, such as a received user command
                if (exitEarly)
                    break;
            }
        }
        return ""; // Return empty String on invalid response
    }

    // 11: string16 (Native Type: String)
    String receiveString16(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        unsigned long startTime = millis();
        while (millis() - startTime < timeout)
        {
            const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout / 4);
            if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 16)
            {
                isValid = true;
                return String(PfeifferVacProtocol::String16(telegram.data.c_str()).decode());
            }
            else
            {
                isValid = false;
                _printInvalidResponseDebug(telegram, expectedParameter);
                // Small delay before retrying:
                bool exitEarly = _delayFunc(50);
                // break & return if something happened durring the delay which requires exit, such as a received user command
                if (exitEarly)
                    break;
            }
        }
        return "";
    }

    // 12: string8 (Native Type: String)
    String receiveString8(uint16_t expectedParameter, bool &isValid, bool debugPrint = true, unsigned long timeout = 1000)
    {
        unsigned long startTime = millis();
        while (millis() - startTime < timeout)
        {
            const PfeifferVacProtocol::PfeifferTelegram telegram = receiveTelegram(debugPrint, timeout / 4);
            if (isValidResponse(telegram, expectedParameter) && telegram.data.length() == 8)
            {
                isValid = true;
                return String(PfeifferVacProtocol::String8(telegram.data.c_str()).decode());
            }
            else
            {
                isValid = false;
                _printInvalidResponseDebug(telegram, expectedParameter);
                // Small delay before retrying:
                bool exitEarly = _delayFunc(50);
                // break & return if something happened durring the delay which requires exit, such as a received user command
                if (exitEarly)
                    break;
            }
        }
        return "";
    }

    // Queries the log of accumulated errors on the turbo pump and returns them as a comma separated human readable list
    const String queryLatestError(bool &isValid, bool debugPrint = false, unsigned long timeout = 1000)
    {
        sendQuery(PfeifferVacProtocol::StatusRequest::ErrorCode, debugPrint);
        delay(1); // delay to avoid backtalk
        String errMsg = receiveString6(PfeifferVacProtocol::StatusRequest::ErrorCode, isValid, debugPrint, timeout / 4);
        const PfeifferVacProtocol::ErrorCodeDebugEntry *ErrorDebugEntry = PfeifferVacProtocol::getHumanReadableErrorMessage(errMsg);
        if (ErrorDebugEntry != nullptr)
            return String(ErrorDebugEntry->problem);
        else
            return String("");
    }

    // Queries the log of accumulated errors on the turbo pump and returns them as a comma separated human readable list
    const String queryErrorHistory(bool &isValid, bool debugPrint = false, unsigned long timeout = 1000)
    {
        String errorMessages = "";
        for (int p = PfeifferVacProtocol::StatusRequest::ErrHist1; p <= PfeifferVacProtocol::StatusRequest::ErrHist10; p++)
        {
            sendQuery(p, debugPrint);
            delay(1); // delay to avoid backtalk
            String errMsg = receiveString6(p, isValid, debugPrint, timeout / 4);
            if (!isValid)
            {
                errorMessages += "failed to query turbo error history,";
                break;
            }
            else if (errMsg == String("000000"))
            {
                break; // we've reached the end of the error/warning history
            }
            else
            {
                errorMessages += errMsg;
#if PFEIFFER_VAC_PROTOCOL_INCLUDE_DEBUG
                const PfeifferVacProtocol::ErrorCodeDebugEntry *ErrorDebugEntry = PfeifferVacProtocol::getHumanReadableErrorMessage(errMsg);
                if (ErrorDebugEntry != nullptr)
                {
                    errorMessages += String(" ") + String(ErrorDebugEntry->problem);
                }
#endif // PFEIFFER_VAC_PROTOCOL_INCLUDE_DEBUG
                errorMessages += ",";
            }
        }
        return errorMessages;
    }

private:
    RS485Serial_Base &_turboSerial;
    Stream &_debug;
    DelayFunc _delayFunc;
    char _address[4]; // 3 digits + null terminator address of the turbo pump
};

#endif // PFEIFFER_SERIAL_H
