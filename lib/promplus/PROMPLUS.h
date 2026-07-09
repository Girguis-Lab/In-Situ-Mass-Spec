/*
  ###########################################################
  # This is a fork of the original library.                 #
  # I changed it to read/write 3 times in order to ensure   #
  # data validity, even if power loss has currupted data.   #
  ###########################################################

  EEPROM.h - EEPROM library
  Original Copyright (c) 2006 David A. Mellis.  All right reserved.
  New version by Christopher Andrews 2015.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef PROMPLUS_h
#define PROMPLUS_h

#include <inttypes.h>
#include <avr/eeprom.h>
#include <avr/io.h>

uint8_t PROM_duplicates = 3;

/***
    PRef class.

    This object references an EEPROM cell.
    Its purpose is to mimic a typical byte of RAM, however its storage is the EEPROM.
    This class has an overhead of two bytes, similar to storing a pointer to an EEPROM cell.
***/

struct PRef
{

    PRef(const int index)
        : index(index) {}
    uint8_t duplicates = PROM_duplicates;
    // Access/read members.
    uint8_t operator*() const
    {
        // return eeprom_read_byte( (uint8_t*) index );
        uint8_t
            winner = 0,
            idx = 0;
        uint8_t array[duplicates];

        for (uint8_t i = 0; i < duplicates; i++)
        {
            array[i] = eeprom_read_byte((uint8_t *)((index * duplicates) + i));
        }

        for (uint8_t i = 0; i < duplicates; i++)
        {
            uint8_t count = 1;
            for (uint8_t j = 0; j < duplicates; j++)
            {
                if (array[i] == array[j])
                {
                    count++;
                }
            }
            if (count > winner)
            {
                winner = count;
                idx = i;
            }
        }
        return array[idx];
    }
    operator uint8_t() const { return **this; }

    // Assignment/write members.
    PRef &operator=(const PRef &ref) { return *this = *ref; }
    PRef &operator=(uint8_t in)
    {
        // return eeprom_write_byte( (uint8_t*) index, in ), *this;
        for (int i = 0; i < duplicates; i++)
        {
            eeprom_write_byte((uint8_t *)((index * duplicates) + i), in);
        }
        return *this;
    }
    PRef &operator+=(uint8_t in) { return *this = **this + in; }
    PRef &operator-=(uint8_t in) { return *this = **this - in; }
    PRef &operator*=(uint8_t in) { return *this = **this * in; }
    PRef &operator/=(uint8_t in) { return *this = **this / in; }
    PRef &operator^=(uint8_t in) { return *this = **this ^ in; }
    PRef &operator%=(uint8_t in) { return *this = **this % in; }
    PRef &operator&=(uint8_t in) { return *this = **this & in; }
    PRef &operator|=(uint8_t in) { return *this = **this | in; }
    PRef &operator<<=(uint8_t in) { return *this = **this << in; }
    PRef &operator>>=(uint8_t in) { return *this = **this >> in; }

    PRef &update(uint8_t in) { return in != *this ? *this = in : *this; }

    /** Prefix increment/decrement **/
    PRef &operator++() { return *this += 1; }
    PRef &operator--() { return *this -= 1; }

    /** Postfix increment/decrement **/
    uint8_t operator++(int)
    {
        uint8_t ret = **this;
        return ++(*this), ret;
    }

    uint8_t operator--(int)
    {
        uint8_t ret = **this;
        return --(*this), ret;
    }

    int index; // Index of current EEPROM cell.
};

/***
    PPtr class.

    This object is a bidirectional pointer to EEPROM cells represented by PRef objects.
    Just like a normal pointer type, this can be dereferenced and repositioned using
    increment/decrement operators.
***/

struct PPtr
{

    PPtr(const int index)
        : index(index) {}

    operator int() const { return index; }
    PPtr &operator=(int in) { return index = in, *this; }

    // Iterator functionality.
    bool operator!=(const PPtr &ptr) { return index != ptr.index; }
    PRef operator*() { return index; }

    /** Prefix & Postfix increment/decrement **/
    PPtr &operator++() { return ++index, *this; }
    PPtr &operator--() { return --index, *this; }
    PPtr operator++(int) { return index++; }
    PPtr operator--(int) { return index--; }

    int index; // Index of current EEPROM cell.
};

/***
    EEPROMClass class.

    This object represents the entire EEPROM space.
    It wraps the functionality of PPtr and PRef into a basic interface.
    This class is also 100% backwards compatible with earlier Arduino core releases.
***/

struct PROMClass
{
    // Basic user access methods.
    PRef operator[](const int idx) { return idx; }
    uint8_t read(int idx) { return PRef(idx); }
    void write(int idx, uint8_t val) { (PRef(idx)) = val; }
    void update(int idx, uint8_t val) { PRef(idx).update(val); }

    // STL and C++11 iteration capability.
    PPtr begin() { return 0x00; }
    PPtr end() { return length(); } // Standards requires this to be the item after the last valid entry. The returned pointer is invalid.
    uint16_t length() { return (E2END + 1) / PROM_duplicates; }

    // Functionality to 'get' and 'put' objects to and from EEPROM.
    template <typename T>
    T &get(int idx, T &t, int base = 0)
    {
        PPtr e = base + (idx * sizeof(T));
        uint8_t *ptr = (uint8_t *)&t;
        for (int count = sizeof(T); count; --count, ++e)
            *ptr++ = *e;
        return t;
    }

    template <typename T>
    const T &put(int idx, const T &t, int base = 0)
    {
        PPtr e = base + (idx * sizeof(T));
        const uint8_t *ptr = (const uint8_t *)&t;
        for (int count = sizeof(T); count; --count, ++e)
            (*e).update(*ptr++);
        return t;
    }

    // Configuration methods
    void duplicates(uint8_t value) { PROM_duplicates = value; }
};

static PROMClass PROM;

#endif
