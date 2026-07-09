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
            LOG_INFO("-> Turbo ");
        }
        else
        {
            LOG_INFO("<- Turbo ");
        }
        // return early for empty telegram
        if (telegram.address.length() == 0 || telegram.parameter.length() == 0)
        {
            LOG_INFO("Nothing ");
            return;
        }

        // Find the command entry by parameter number
        ParameterDebugEntry debugEntry = {};
        int paramNum = telegram.parameter.toInt();
        const bool debugEntryFound = PfeifferVacProtocol::getParameterDebugEntry(paramNum, &debugEntry);
        LOG_TRACE("(Addr: ", telegram.address.c_str(), ")");
        if (outgoing)
        {
            if (telegram.action == (char)PfeifferVacProtocol::Action::Query)
                LOG_INFO("Query ");
            else if (telegram.action == (char)PfeifferVacProtocol::Action::Command)
                LOG_INFO("Command set ");
            else
                LOG_WARN("Unknown action '", telegram.action, "' ");
        }

        if (debugEntryFound)
        {
            LOG_INFO(debugEntry.description);
        }
        LOG_DEBUG(" (p", telegram.parameter.c_str(), ")");

        if (telegram.error == TelegramError::InvalidChecksum)
        {
            LOG_INFO(" checksum error!\n");
            return; // For errors return early
        }
        else if (telegram.error == TelegramError::InvalidParameter)
        {
            LOG_INFO(" invalid parameter!\n");
            return; // For errors return early
        }
        if (telegram.error == TelegramError::LogicError)
        {
            LOG_INFO(" turbo pump logic error!\n");
            return; // For errors return early
        }
        if (telegram.error == TelegramError::OutOfRange)
        {
            LOG_INFO(" value out of range!\n");
            return; // For errors return early
        }

        if (outgoing && telegram.action == (char)PfeifferVacProtocol::Action::Query)
        {
            LOG_INFO("\n");
            return; // For outgoing queries, we don't have data to interpret, so we can return early after printing the parameter description
        }
        if (debugEntryFound)
        {
            if (telegram.data.length() > 0)
            {
                if (outgoing && telegram.action == (char)PfeifferVacProtocol::Action::Command)
                    LOG_INFO(" to ");
                else if (!outgoing)
                    LOG_INFO(" is ");
                const char *ascii = telegram.data.c_str();
                switch (debugEntry.datatype)
                {
                case DataType::BooleanOld:
                    LOG_INFO((debugEntry.lookupFuncFunction ? String(debugEntry.lookupFuncFunction(BooleanOld(ascii).decode() ? 1 : 0)) : String(BooleanOld(ascii).decode())).c_str());
                    LOG_TRACE(" (BooleanOld ", ascii, ")");
                    break;
                case DataType::BooleanNew:
                    LOG_INFO((debugEntry.lookupFuncFunction ? String(debugEntry.lookupFuncFunction(BooleanNew(ascii).decode() ? 1 : 0)) : String(BooleanNew(ascii).decode())).c_str());
                    LOG_TRACE(" (BooleanNew ", ascii, ")");
                    break;
                case DataType::UShortInt:
                    LOG_INFO((debugEntry.lookupFuncFunction ? String(debugEntry.lookupFuncFunction(UShortInt(ascii).decode())) : String(UShortInt(ascii).decode())).c_str());
                    LOG_TRACE(" (UShortInt ", ascii, ")");
                    break;
                case DataType::UInteger:
                    LOG_INFO(UInteger(ascii).decode());
                    LOG_TRACE(" (UInteger ", ascii, ")");
                    break;
                case DataType::UReal:
                    LOG_INFO(UReal(ascii).decode());
                    LOG_TRACE(" (UReal ", ascii, ")");
                    break;
                case DataType::String6:
                    LOG_INFO(ascii);
                    break;
                case DataType::String16:
                    LOG_INFO(ascii);
                    break;
                case DataType::String8:
                    LOG_INFO(ascii);
                    break;
                default:
                    LOG_INFO(ascii);
                    LOG_DEBUG(" (Unknown Datatype");
                    LOG_DEBUG(static_cast<int>(debugEntry.datatype));
                    LOG_DEBUG(")");
                    break;
                }
            }
            else
            {
                LOG_INFO("(no data)");
            }
        }
        else
        {
            LOG_INFO(" Unknown Param ", telegram.parameter.c_str());
            LOG_DEBUG("RawData: ", telegram.data.length() > 0 ? telegram.data.c_str() : "(no data)");
        }
        LOG_INFO("\n");
    }
}

#endif // PFEIFFER_VAC_PROTOCOL_FUNCS_H
