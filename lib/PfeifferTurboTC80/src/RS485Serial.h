#pragma once
#include <Arduino.h>

// Base interface so users can swap implementations if they want.
// You can derive from this class and keep the same method interface.
/// Concrete implementation that wraps an underlying Serial-like object
/// and manages RS485 direction pins + RX/TX buffering.
/// TSerial must be a Print + Stream-like class (e.g. HardwareSerial).
template <
    typename TSerial,
    size_t RX_BUF_SIZE = 128,
    size_t TX_BUF_SIZE = 128>
class RS485Serial
{
public:
    RS485Serial(
        TSerial &serial,
        uint8_t pin485SendEnable,
        uint8_t pin485ReceiveDisable,
        void (*delayFunc)(unsigned long) = delay)
        : _serial(serial),
          _pin485SendEnable(pin485SendEnable),
          _pin485ReceiveDisable(pin485ReceiveDisable),
          _delayFunc(delayFunc)
    {
        _rxHead = _rxTail = 0;
        _txHead = _txTail = 0;
        _sendMode = false;
        _lastActivityMs = 0;
    }

    // Setup / teardown
    virtual void begin(unsigned long baud = 9600, uint32_t config = SERIAL_8N1)
    {
        if (_pin485SendEnable != 0)
            pinMode(_pin485SendEnable, OUTPUT);
        if (_pin485ReceiveDisable != 0)
            pinMode(_pin485ReceiveDisable, OUTPUT);
        enterReceiveMode();          // start in receive mode
        _serial.begin(baud, config); // hardware serial with config
    }

    virtual void end()
    {
        _serial.end();
    };

    // Direction control (explicit override if needed)

    virtual void enterReceiveMode()
    {
        // Flush outgoing data first
        _serial.flush();

        if (_pin485SendEnable != 0)
            digitalWrite(_pin485SendEnable, LOW);

        if (_pin485ReceiveDisable != 0)
            digitalWrite(_pin485ReceiveDisable, LOW);

        _sendMode = false;
    }

    virtual void enterSendMode()
    {
        // Drain any in-flight incoming bytes into RX buffer
        unsigned long startTime = millis();
        const unsigned long timeoutMs = 2; // small debounce; tune if needed

        while (_serial.available())
        {
            int c = _serial.read();
            if (c >= 0)
                pushRx(static_cast<uint8_t>(c));
            _delayFunc(1);
            if (millis() - startTime > timeoutMs)
                break;
        }

        if (_pin485SendEnable != 0)
            digitalWrite(_pin485SendEnable, HIGH);

        if (_pin485ReceiveDisable != 0)
            digitalWrite(_pin485ReceiveDisable, HIGH);

        _sendMode = true;
    }

    // --- Cooperative task function ---
    //
    // Call this frequently (e.g. each loop()) to:
    //  - pull incoming bytes into RX buffer
    //  - send bytes from TX buffer when possible
    //  - automatically toggle direction to TX when there is data to send,
    //    and back to RX when done
    virtual void task()
    {
        unsigned long now = millis();

        // 1) Always read from hardware into RX buffer when in receive mode
        if (!_sendMode)
        {
            while (_serial.available() > 0)
            {
                int c = _serial.read();
                if (c < 0)
                    break;
                pushRx(static_cast<uint8_t>(c));
                _lastActivityMs = now;
            }
        }

        // 2) If we have data to transmit and we're not in send mode, switch
        if (!_sendMode && txAvailable() > 0)
        {
            enterSendMode();
        }

        // 3) If in send mode, send out bytes from TX buffer
        if (_sendMode)
        {
            while (txAvailable() > 0 && _serial.availableForWrite() > 0)
            {
                uint8_t b = popTx();
                _serial.write(b);
                _lastActivityMs = now;
            }

            // If we've emptied the TX buffer, flush underlying and go back to RX
            if (txAvailable() == 0)
            {
                _serial.flush();
                enterReceiveMode();
            }
        }
    }

    // --- RX API ---

    virtual int available()
    {
        return static_cast<int>(rxAvailable());
    }

    virtual int availableForWrite()
    {
        return static_cast<int>(TX_BUF_SIZE - txAvailable() - 1);
    }

    virtual int read()
    {
        if (rxAvailable() == 0)
            return -1;
        return static_cast<int>(popRx());
    }

    virtual int peek()
    {
        if (rxAvailable() == 0)
            return -1;
        return static_cast<int>(_rxBuffer[_rxTail]);
    }

    // --- Write / Print API ---

    virtual size_t write(const uint8_t *buffer, size_t size)
    {
        size_t written = 0;
        for (size_t i = 0; i < size; ++i)
        {
            if (!pushTx(buffer[i]))
                break;
            ++written;
        }
        return written;
    }

    size_t write(uint8_t b)
    {
        return pushTx(b) ? 1 : 0;
    }

    // Make templated print more efficient by reusing underlying Print::print where possible
    template <typename T>
    size_t print(const T &value)
    {
        // Use the base RS485Serial::print which calls writeImpl
        return RS485Serial::print(value);
    }

    template <typename T>
    size_t println(const T &value)
    {
        return RS485Serial::println(value);
    }

protected:
    // Allow derived classes to override how templated print is implemented
    template <typename T>
    size_t writeImpl(const T &value)
    {
        // Default implementation converts via underlying Print-like behavior
        String s(value);
        return write(reinterpret_cast<const uint8_t *>(s.c_str()), s.length());
    }

private:
    TSerial &_serial;
    uint8_t _pin485SendEnable;
    uint8_t _pin485ReceiveDisable;
    void (*_delayFunc)(unsigned long);

    // RS485 state
    bool _sendMode;
    unsigned long _lastActivityMs;

    // RX ring buffer
    uint8_t _rxBuffer[RX_BUF_SIZE];
    volatile size_t _rxHead;
    volatile size_t _rxTail;

    // TX ring buffer
    uint8_t _txBuffer[TX_BUF_SIZE];
    volatile size_t _txHead;
    volatile size_t _txTail;

    // --- RX buffer helpers ---

    size_t rxAvailable() const
    {
        return (_rxHead + RX_BUF_SIZE - _rxTail) % RX_BUF_SIZE;
    }

    bool pushRx(uint8_t b)
    {
        size_t next = (_rxHead + 1) % RX_BUF_SIZE;
        if (next == _rxTail)
        {
            // Buffer full; drop byte
            return false;
        }
        _rxBuffer[_rxHead] = b;
        _rxHead = next;
        return true;
    }

    uint8_t popRx()
    {
        if (_rxTail == _rxHead)
            return 0;
        uint8_t b = _rxBuffer[_rxTail];
        _rxTail = (_rxTail + 1) % RX_BUF_SIZE;
        return b;
    }

    // --- TX buffer helpers ---

    size_t txAvailable() const
    {
        return (_txHead + TX_BUF_SIZE - _txTail) % TX_BUF_SIZE;
    }

    bool pushTx(uint8_t b)
    {
        size_t next = (_txHead + 1) % TX_BUF_SIZE;
        if (next == _txTail)
        {
            // TX buffer full
            return false;
        }
        _txBuffer[_txHead] = b;
        _txHead = next;
        return true;
    }

    uint8_t popTx()
    {
        if (_txTail == _txHead)
            return 0;
        uint8_t b = _txBuffer[_txTail];
        _txTail = (_txTail + 1) % TX_BUF_SIZE;
        return b;
    }
};
