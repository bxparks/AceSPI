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

#ifndef ACE_SPI_SIMPLE_SPI_FAST_INTERFACE_H
#define ACE_SPI_SIMPLE_SPI_FAST_INTERFACE_H

#include <stdint.h>
#include <Arduino.h> // OUTPUT, INPUT

namespace ace_spi {

/**
 * Software SPI using pinModeFast(), digitalWriteFast() and shiftOutFast()
 * from https://github.com/NicksonYap/digitalWriteFast.
 *
 * @tparam T_LATCH_PIN the latch pin (CS)
 * @tparam T_DATA_PIN the data pin (MOSI)
 * @tparam T_CLOCK_PIN the clock pin (CLK)
 */
template <uint8_t T_LATCH_PIN, uint8_t T_DATA_PIN, uint8_t T_CLOCK_PIN>
class SimpleSpiFastInterface {
  public:
    /** Constructor. */
    explicit SimpleSpiFastInterface() = default;

    /** Initialize the various pins. */
    void begin() const {
      pinModeFast(T_LATCH_PIN, OUTPUT);
      pinModeFast(T_DATA_PIN, OUTPUT);
      pinModeFast(T_CLOCK_PIN, OUTPUT);
    }

    /** Reset the various pins. */
    void end() const {
      pinModeFast(T_LATCH_PIN, INPUT);
      pinModeFast(T_DATA_PIN, INPUT);
      pinModeFast(T_CLOCK_PIN, INPUT);
    }

    /** Begin SPI transaction. Pull latch LOW. */
    void beginTransaction() const {
      digitalWriteFast(T_LATCH_PIN, LOW);
    }

    /** End SPI transaction. Pull latch HIGH. */
    void endTransaction() const {
      digitalWriteFast(T_LATCH_PIN, HIGH);
    }

    /** Transfer 8 bits. */
    void transfer(uint8_t value) const {
      shiftOutFast(value);
    }

    /** Transfer 16 bits. */
    void transfer16(uint16_t value) const {
      uint8_t msb = (value & 0xff00) >> 8;
      uint8_t lsb = (value & 0xff);
      shiftOutFast(msb);
      shiftOutFast(lsb);
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
      shiftOutFast(msb);
      shiftOutFast(lsb);
      endTransaction();
    }

    // Use default copy constructor and assignment operator.
    SimpleSpiFastInterface(const SimpleSpiFastInterface&) = default;
    SimpleSpiFastInterface& operator=(const SimpleSpiFastInterface&) = default;

  private:
    static void shiftOutFast(uint8_t output) {
      uint8_t mask = 0x80; // start with the MSB
      for (uint8_t i = 0; i < 8; i++)  {
        digitalWriteFast(T_CLOCK_PIN, LOW);
        if (output & mask) {
          digitalWriteFast(T_DATA_PIN, HIGH);
        } else {
          digitalWriteFast(T_DATA_PIN, LOW);
        }
        digitalWriteFast(T_CLOCK_PIN, HIGH);
        mask >>= 1;
      }
    }
};

} // ace_spi

#endif
