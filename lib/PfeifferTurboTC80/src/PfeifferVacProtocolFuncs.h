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
        bool checksumValid;

        PfeifferTelegram() : address(""), action('\0'), parameter(""), data(""), checksumValid(true) {}
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
            resp.checksumValid = false;
            return resp; // too short
        }
        resp.address = telegramStr.substring(0, 3);
        resp.action = telegram[3];
        // there is always a '0' at index 4
        resp.parameter = telegramStr.substring(5, 8);
        // Data is between param and checksum
        int dataLen = atoi(telegramStr.substring(8, 10).c_str());
        resp.data = telegramStr.substring(10, 10 + dataLen);

        uint8_t telegramChecksum = atoi(telegramStr.substring(len - 3, len).c_str());
        // Calculate checksum from all chars except the last 4 (checksum)
        uint16_t calculatedChecksum = 0;
        for (size_t i = 0; i < len - 3; ++i)
            calculatedChecksum += (uint16_t)telegram[i];
        calculatedChecksum = calculatedChecksum % 256;
        resp.checksumValid = (telegramChecksum == calculatedChecksum);
        return resp;
    }

    // Print a human-readable version of a PfeifferTelegram
    void printTelegramHumanReadable(const PfeifferTelegram &telegram, Stream &stream)
    {
        if (telegram.address.length() == 0 || telegram.parameter.length() == 0)
        {
            stream.print("Nothing");
            return;
        }
        int paramNum = telegram.parameter.toInt();
        // Find the command entry by parameter number
        const ParameterDebugEntry *entry = PfeifferVacProtocol::getParameterDebugEntry(paramNum);
        // print details
        stream.print("Turbo Pump ");
        stream.print(telegram.address);
        if (telegram.action == (char)PfeifferVacProtocol::Action::Query)
            stream.print(" Query");
        else if (telegram.action == (char)PfeifferVacProtocol::Action::Command)
            stream.print(" Command");
        else
            stream.print(String(" Unknown action (") + telegram.action + ")");

        stream.print(String(" Parameter ") + telegram.parameter + " = ");
        if (entry)
        {
            stream.print(String(entry->description).c_str());
            stream.print(" is ");
            if (telegram.data.length() > 0)
            {
                const char *ascii = telegram.data.c_str();
                switch (entry->datatype)
                {
                case DataType::BooleanOld:
                    stream.print("(BooleanOld)");
                    stream.print(ascii);
                    stream.print(" = ");
                    stream.print(String(entry->lookupFuncFunction(BooleanOld(ascii).decode() ? 1 : 0)).c_str());
                    break;
                case DataType::BooleanNew:
                    stream.print("(BooleanNew) ");
                    stream.print(ascii);
                    stream.print(" = ");
                    stream.print(String(entry->lookupFuncFunction(BooleanNew(ascii).decode() ? 1 : 0)).c_str());
                    break;
                case DataType::UShortInt:
                    stream.print("(UShortInt) ");
                    stream.print(ascii);
                    stream.print(" = ");
                    stream.print(String(entry->lookupFuncFunction(UShortInt(ascii).decode())).c_str());
                    break;
                case DataType::UInteger:
                    stream.print("(UInteger) ");
                    stream.print(ascii);
                    stream.print(" = ");
                    stream.print(UInteger(ascii).decode());
                    break;
                case DataType::UReal:
                    stream.print("(UReal) ");
                    stream.print(ascii);
                    stream.print(" = ");
                    stream.print(UReal(ascii).decode(), 2);
                    break;
                case DataType::String6:
                    stream.print(ascii);
                    break;
                case DataType::String16:
                    stream.print(ascii);
                    break;
                case DataType::String8:
                    stream.print(ascii);
                    break;
                default:
                    stream.print(String("Unknown Datatype (") + ((int)entry->datatype) + ")");
                    stream.print(" Raw data is ");
                    stream.print(ascii);
                    break;
                }
            }
            else
            {
                stream.print("(no data)");
            }
        }
        else
        {
            stream.print(String(" Unknown ") + telegram.parameter);
            stream.print(telegram.data.length() > 0 ? (String(" Data is ") + telegram.data) : "(no data)");
        }
        if (!telegram.checksumValid)
        {
            stream.print(" | WARNING: Invalid checksum!");
        }
        stream.println();
    }
}

#endif // PFEIFFER_VAC_PROTOCOL_FUNCS_H
