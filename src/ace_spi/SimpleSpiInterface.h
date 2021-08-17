/*
MIT License

Copyright (c) 2021 Brian T. Park

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef ACE_SPI_SIMPLE_SPI_INTERFACE_H
#define ACE_SPI_SIMPLE_SPI_INTERFACE_H

#include <stdint.h>
#include <Arduino.h>

namespace ace_spi {

/** Software SPI using shiftOut(). */
class SimpleSpiInterface {
  public:
    /**
     * Constructor.
     *
     * @param latchPin the latch pin (CS)
     * @param dataPin the data pin (MOSI)
     * @param clockPin the clock pin (CLK)
     */
    explicit SimpleSpiInterface(
        uint8_t latchPin,
        uint8_t dataPin,
        uint8_t clockPin
    ) :
        mLatchPin(latchPin),
        mDataPin(dataPin),
        mClockPin(clockPin)
    {}

    /** Initialize the various pins. */
    void begin() const {
      pinMode(mLatchPin, OUTPUT);
      pinMode(mDataPin, OUTPUT);
      pinMode(mClockPin, OUTPUT);
    }

    /** Reset the various pins. */
    void end() const {
      pinMode(mLatchPin, INPUT);
      pinMode(mDataPin, INPUT);
      pinMode(mClockPin, INPUT);
    }

    /** Begin SPI transaction. Pull latch LOW. */
    void beginTransaction() const {
      digitalWrite(mLatchPin, LOW);
    }

    /** End SPI transaction. Pull latch HIGH. */
    void endTransaction() const {
      digitalWrite(mLatchPin, HIGH);
    }

    /** Transfer 8 bits. */
    void transfer(uint8_t value) const {
      shiftOut(mDataPin, mClockPin, MSBFIRST, value);
    }

    /** Transfer 16 bits. */
    void transfer16(uint16_t value) const {
      uint8_t msb = (value & 0xff00) >> 8;
      uint8_t lsb = (value & 0xff);
      shiftOut(mDataPin, mClockPin, MSBFIRST, msb);
      shiftOut(mDataPin, mClockPin, MSBFIRST, lsb);
    }

    /** Convenience method to send 8 bits a single transaction. */
    void send8(uint8_t value) const {
      beginTransaction();
      transfer(value);
      endTransaction();
    }

    /** Convenience method to send 16 bits a single transaction. */
    void send16(uint16_t value) const {
      beginTransaction();
      transfer16(value);
      endTransaction();
    }

    /** Convenience method to send 16 bits a single transaction. */
    void send16(uint8_t msb, uint8_t lsb) const {
      beginTransaction();
      shiftOut(mDataPin, mClockPin, MSBFIRST, msb);
      shiftOut(mDataPin, mClockPin, MSBFIRST, lsb);
      endTransaction();
    }

    // Use default copy constructor and assignment operator.
    SimpleSpiInterface(const SimpleSpiInterface&) = default;
    SimpleSpiInterface& operator=(const SimpleSpiInterface&) = default;

  private:
    uint8_t const mLatchPin;
    uint8_t const mDataPin;
    uint8_t const mClockPin;
};

} // ace_spi

#endif
