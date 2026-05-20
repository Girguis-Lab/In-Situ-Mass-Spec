#ifndef PFEIFFER_VAC_PROTOCOL_FUNCS_H
#define PFEIFFER_VAC_PROTOCOL_FUNCS_H

#include <Arduino.h>
#include "PfeifferVacProtocolConsts.h"
#include "PfeifferVacProtocolDatatypes.h"

namespace PfeifferVacProtocol
{

    struct PfeifferTelegram
    {
        String address;
        char action;
        String parameter;
        String data;
        TelegramError error;

        PfeifferTelegram() : address(""), action('\0'), parameter(""), data(""), error(TelegramError::None)
        {
        }
    };

    // Helper: Encode a telegram from action, parameter, and data
    String encodeTelegram(const char *address, const char action, const uint16_t parameter, const char *data = nullptr)
    {
        // Telegrams starts with the 3-digit address
        String frame = String(address);
        // Then the action character either a 1 for set or a 0 for query
        frame += action;
        frame += '0'; // 0 character as per protocol
        char paramStr[4];
        snprintf(paramStr, sizeof(paramStr), "%03d", parameter);
        frame += paramStr;
        if (data != nullptr)
        {
            char dataLength[3];
            snprintf(dataLength, sizeof(dataLength), "%02d", (int)strlen(data));
            frame += dataLength;
            frame += data;
        }
        else
        {
            frame += "00"; // no data
        }
        // Calculate checksum
        uint16_t checksum = 0;

        for (size_t i = 0; i < frame.length(); ++i)
            checksum += (uint16_t)frame[i];
        checksum = checksum % 256;
        char checksumStr[4];
        snprintf(checksumStr, sizeof(checksumStr), "%03d", checksum);
        frame += checksumStr;
        return frame;
    }

    // Helper: Decode a telegram into its fields (returns PfeifferResponse)
    PfeifferTelegram decodeTelegram(const char *telegram)
    {
        PfeifferTelegram resp;
        String telegramStr = String(telegram);
        size_t len = strlen(telegram);
        if (len < 8)
        {
            resp.error = TelegramError::InvalidChecksum;
            return resp; // too short
        }
        resp.address = telegramStr.substring(0, 3);
        resp.action = telegram[3];
        // there is always a '0' at index 4
        resp.parameter = telegramStr.substring(5, 8);
        // Data is between param and checksum
        int dataLen = atoi(telegramStr.substring(8, 10).c_str());
        resp.data = telegramStr.substring(10, 10 + dataLen);

        // handle special error cases based on data content
        if (resp.data == "NO_DEF")
        {
            resp.error = TelegramError::InvalidParameter;
        }
        else if (resp.data == "_RANGE")
        {
            resp.error = TelegramError::OutOfRange;
        }
        else if (resp.data == "_LOGIC")
        {
            resp.error = TelegramError::LogicError;
        }

        uint8_t telegramChecksum = atoi(telegramStr.substring(len - 3, len).c_str());
        // Calculate checksum from all chars except the last 4 (checksum)
        uint16_t calculatedChecksum = 0;
        for (size_t i = 0; i < len - 3; ++i)
            calculatedChecksum += (uint16_t)telegram[i];
        calculatedChecksum = calculatedChecksum % 256;
        if (telegramChecksum != calculatedChecksum)
        {
            resp.error = TelegramError::InvalidChecksum;
        }
        return resp;
    }

    // Print a human-readable version of a PfeifferTelegram
    void printTelegramHumanReadable(const PfeifferTelegram &telegram, bool outgoing = false)
    {
        // print details
        if (outgoing)
        {
            Log.info("-> Turbo ");
        }
        else
        {
            Log.info("<- Turbo ");
        }
        // return early for empty telegram
        if (telegram.address.length() == 0 || telegram.parameter.length() == 0)
        {
            Log.info("Nothing ");
            return;
        }
        int paramNum = telegram.parameter.toInt();
        // Find the command entry by parameter number
        const ParameterDebugEntry *entry = PfeifferVacProtocol::getParameterDebugEntry(paramNum);

        Log.verbose("(Addr:%s) ", telegram.address.c_str());
        if (outgoing)
        {
            if (telegram.action == (char)PfeifferVacProtocol::Action::Query)
                Log.info("Query ");
            else if (telegram.action == (char)PfeifferVacProtocol::Action::Command)
                Log.info("Command set ");
            else
                Log.warning("Unknown action '%s' ", telegram.action);
        }

        if (entry)
        {
            Log.info(entry->description);
        }
        Log.trace(" (p%s)", telegram.parameter.c_str());

        if (telegram.error == TelegramError::InvalidChecksum)
        {
            Log.info(" checksum error!\n");
            return; // For errors return early
        }
        else if (telegram.error == TelegramError::InvalidParameter)
        {
            Log.info(" invalid parameter!\n");
            return; // For errors return early
        }
        if (telegram.error == TelegramError::LogicError)
        {
            Log.info(" turbo pump logic error!\n");
            return; // For errors return early
        }
        if (telegram.error == TelegramError::OutOfRange)
        {
            Log.info(" value out of range!\n");
            return; // For errors return early
        }

        if (outgoing && telegram.action == (char)PfeifferVacProtocol::Action::Query)
        {
            Log.info("\n");
            return; // For outgoing queries, we don't have data to interpret, so we can return early after printing the parameter description
        }
        if (entry)
        {
            if (telegram.data.length() > 0)
            {
                if (outgoing && telegram.action == (char)PfeifferVacProtocol::Action::Command)
                    Log.info(" to ");
                else if (!outgoing)
                    Log.info(" is ");
                const char *ascii = telegram.data.c_str();
                switch (entry->datatype)
                {
                case DataType::BooleanOld:
                    Log.info(String(entry->lookupFuncFunction(BooleanOld(ascii).decode() ? 1 : 0)).c_str());
                    Log.verbose(" (BooleanOld %s)", ascii);
                    break;
                case DataType::BooleanNew:
                    Log.info(String(entry->lookupFuncFunction(BooleanNew(ascii).decode() ? 1 : 0)).c_str());
                    Log.verbose(" (BooleanNew %s)", ascii);
                    break;
                case DataType::UShortInt:
                    Log.info(String(entry->lookupFuncFunction(UShortInt(ascii).decode())).c_str());
                    Log.verbose(" (UShortInt %s)", ascii);
                    break;
                case DataType::UInteger:
                    Log.info("%l", UInteger(ascii).decode());
                    Log.verbose(" (UInteger %s)", ascii);
                    break;
                case DataType::UReal:
                    Log.info("%F", (double)UReal(ascii).decode());
                    Log.verbose(" (UReal %s)", ascii);
                    break;
                case DataType::String6:
                    Log.info(ascii);
                    break;
                case DataType::String16:
                    Log.info(ascii);
                    break;
                case DataType::String8:
                    Log.info(ascii);
                    break;
                default:
                    Log.info(ascii);
                    Log.trace(" (Unknown Datatype %d)", (int)entry->datatype);
                    break;
                }
            }
            else
            {
                Log.info("(no data)");
            }
        }
        else
        {
            Log.info(" Unknown Param %s", telegram.parameter.c_str());
            Log.trace("RawData: %s", telegram.data.length() > 0 ? telegram.data.c_str() : "(no data)");
        }
    }
}

#endif // PFEIFFER_VAC_PROTOCOL_FUNCS_H
