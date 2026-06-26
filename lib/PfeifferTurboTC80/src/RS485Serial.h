#pragma once
#include <Arduino.h>
#include "RingBufCPP.h"

// Base interface so users can swap implementations if they want.
// You can derive from this class and keep the same method interface.
class RS485Serial_Base
{
public:
    RS485Serial_Base() {};

    // Setup / teardown
    virtual void begin(unsigned long baud = 9600, uint32_t config = SERIAL_8N1);
    virtual void end();

    // Direction control (explicit override if needed)
    virtual void enterReceiveMode();
    virtual void enterSendMode();

    // --- Cooperative task function ---
    // Call this frequently (e.g. each loop()) to:
    virtual void task();

    // --- RX API ---
    virtual int available();
    virtual int availableForWrite();
    virtual int read();

    // --- Write API ---
    // Write is blocking
    virtual size_t write(const uint8_t *buffer, size_t size);
    virtual size_t write(uint8_t b);

    // --- Print API ---
    // Make templated print more efficient by reusing underlying Print::print where possible
    template <typename T>
    size_t print(const T &value);
    template <typename T>
    size_t println(const T &value);
};

// Concrete RS485 implementation that wraps an underlying HardwareSerial Object
// and manages RS485 direction pins + RX/TX buffering.
class RS485HardwareSerial : public RS485Serial_Base
{
public:
    RS485HardwareSerial(
        HardwareSerial &serial,
        uint8_t pin485SendEnable,
        uint8_t pin485ReceiveDisable,
        unsigned long readTimeoutMs = 6)
        : serial(serial),
          _pin485SendEnable(pin485SendEnable),
          _pin485ReceiveDisable(pin485ReceiveDisable),
          _readTimeoutMs(readTimeoutMs)
    {
        _sendMode = false;
    }

    // Setup / teardown
    void begin(unsigned long baud = 9600, uint32_t config = SERIAL_8N1) override
    {
        if (_pin485SendEnable != 0)
            pinMode(_pin485SendEnable, OUTPUT);
        if (_pin485ReceiveDisable != 0)
            pinMode(_pin485ReceiveDisable, OUTPUT);
        enterReceiveMode();         // start in receive mode
        serial.begin(baud, config); // hardware serial with config
    }

    void end() override
    {
        serial.end();
    };

    // === Data Flow Direction Control ==== (allows explicit override if needed)

    void enterReceiveMode() override
    {
        // make sure outgoing messages have sent fully.
        serial.flush();

        // Disable sending mode
        _sendMode = false;
        if (_pin485SendEnable != 0)
            digitalWrite(_pin485SendEnable, LOW);
        if (_pin485ReceiveDisable != 0)
            digitalWrite(_pin485ReceiveDisable, LOW);
    }

    void enterSendMode() override
    {
        // Drain any in-flight incoming bytes into RX buffer
        _injestRxBytes();

        // Enable sending mode
        _sendMode = true;
        if (_pin485SendEnable != 0)
            digitalWrite(_pin485SendEnable, HIGH);
        if (_pin485ReceiveDisable != 0)
            digitalWrite(_pin485ReceiveDisable, HIGH);
    }

    // === Cooperative task function ==
    // Call this frequently (e.g. each loop()) to:
    // - pull incoming bytes into RX buffer
    void task() override
    {
        // Drain arduino serial into RX buffer when in receive mode
        if (!_sendMode)
            _injestRxBytes();
    }

    // === RX Methods ===

    int available() override
    {
        return rxBuffer.numElements() + serial.available();
    }

    int availableForWrite() override
    {
        return serial.availableForWrite();
    }

    // read and return a byte from the RX buffer.
    int read() override
    {
        _injestRxBytes();
        if (available() == 0)
            return -1;
        // fetch the last byte from the buffer.
        uint8_t readByte = -1;
        rxBuffer.pull(&readByte);
        return static_cast<int>(readByte);
    }

    // === TX Methods ====

    // Multiple byte write (blocking)
    size_t write(const uint8_t *bytes, size_t size) override
    {
        enterSendMode();
        size_t written = serial.write(bytes, size);
        enterReceiveMode();
        return written;
    }

    // Single byte write (blocking)
    size_t write(uint8_t b) override
    {
        enterSendMode();
        size_t written = serial.write(b);
        enterReceiveMode();
        return written;
    }

    // Reusing underlying serial.print through templating.
    template <typename T>
    size_t print(const T &value)
    {
        enterSendMode();
        size_t written = serial.print(value);
        enterReceiveMode();
        return written;
    }

    // Reusing underlying serial.println through templating.
    template <typename T>
    size_t println(const T &value)
    {
        enterSendMode();
        size_t written = serial.println(value);
        enterReceiveMode();
        return written;
    }

    // Reads all incoming bytes from the arduino serial into the RxBuffer
    // Includes a timeout in case of serial flooding.
    size_t _injestRxBytes()
    {
        unsigned long startTime = millis();
        size_t bytesRead = 0;
        while (serial.available())
        {
            int c = serial.read();
            rxBuffer.add(static_cast<uint8_t>(c));
            bytesRead++;
            if (millis() - startTime > _readTimeoutMs)
                break;
        }
        return bytesRead;
    }

    HardwareSerial &serial;

private:
    uint8_t _pin485SendEnable;
    uint8_t _pin485ReceiveDisable;
    unsigned long _readTimeoutMs;
    RingBufCPP<uint8_t, 600> rxBuffer;

    // RS485 state
    bool _sendMode;
};
