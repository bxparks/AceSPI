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

#ifndef ACE_SPI_HARD_SPI_INTERFACE_H
#define ACE_SPI_HARD_SPI_INTERFACE_H

#include <stdint.h>
#include <Arduino.h> // digitalWrite()
#include <SPI.h>

namespace ace_spi {

/**
 * Hardware SPI interface to talk to SPI peripherals. It was initially created
 * to communicate with the 74HC595 Shift Register chip, then verified to work
 * with the MAX7219 LED controller chip. This is currently not meant to be
 * general-purpose SPI interface. In particular, it supports only SPI MODE0, and
 * MSBFIRST configurations.
 *
 * For different SPI configurations, it is probably easiest to just copy this
 * file, make the necessary changes, then substitute the new class in places
 * where this class is used. The maximum speed of MAX7219 is 16MHz so this class
 * sets the default SPI speed to 8MHz.
 *
 * The ESP32 has 2 user-accessible SPI buses (HSPI and VSPI), and so does the
 * STM32F1 (SPI1 and SPI2). Usually, the predefined SPI instance is used, but
 * a user-defined secondary SPI instance can be passed into the constructor.
 *
 * @tparam T_SPI the class of the hardware SPI instance, usually SPIClass
 * @tparam T_CLOCK_SPEED the SPI clock speed, default 8000000 (8 MHz)
 */
template <
    typename T_SPI,
    uint32_t T_CLOCK_SPEED = 8000000
>
class HardSpiInterface {
  private:
    // Some of the following constants are defined in <SPI.h> so unfortunately,
    // it is not possible to avoid pulling in the global SPI instance into
    // applications which don't use SPI.

    /** MSB first or LSB first */
  #if defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_ARCH_SAMD)
    static const BitOrder kBitOrder = MSBFIRST;
  #else
    static const uint8_t kBitOrder = MSBFIRST;
  #endif

    /** SPI mode */
    static const uint8_t kSpiMode = SPI_MODE0;

  public:
    /**
     * Constructor.
     *
     * @param spi instance of the `T_SPI` class. If the pre-installed `<SPI.h>`
     *    is used, `T_SPI` is `SPIClass` and `spi` will be the pre-defined `SPI`
     *    object.
     * @param latchPin the pin that controls the CS/SS pin of the slave device
     */
    explicit HardSpiInterface(T_SPI& spi, uint8_t latchPin) :
        mSpi(spi),
        mLatchPin(latchPin)
    {}

    /**
     * Initialize the HardSpiInterface. The hardware SPI object must be
     * initialized using `SPI.begin()` as well.
     */
    void begin() const {
      // To use Hardware SPI on ESP8266, we must set the SCK and MOSI pins to
      // 'SPECIAL' instead of 'OUTPUT'. This is performed by calling
      // SPI.begin(). Also, unlike other Arduino platforms, the SPIClass on
      // the ESP8266 defaults to controlling the SS/CS pin itself, instead of
      // letting the application code control it. The setHwCs(false) let's
      // HardSpiInterface control the CS/SS pin.
      // https://www.esp8266.com/wiki/doku.php?id=esp8266_gpio_pin_allocations
      #if defined(ESP8266)
        mSpi.setHwCs(false);
      #endif

      pinMode(mLatchPin, OUTPUT);
    }

    /** Clean up the object. */
    void end() const {
      pinMode(mLatchPin, INPUT);
    }

    /** Begin SPI transaction. Pull latch LOW. */
    void beginTransaction() const {
      mSpi.beginTransaction(SPISettings(T_CLOCK_SPEED, kBitOrder, kSpiMode));
      digitalWrite(mLatchPin, LOW);
    }

    /** End SPI transaction. Pull latch HIGH. */
    void endTransaction() const {
      digitalWrite(mLatchPin, HIGH);
      mSpi.endTransaction();
    }

    /** Transfer 8 bits. */
    void transfer(uint8_t value) const {
      mSpi.transfer(value);
    }

    /** Transfer 16 bits. */
    void transfer16(uint16_t value) const {
      mSpi.transfer16(value);
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
      uint16_t value = ((uint16_t) msb) << 8 | (uint16_t) lsb;
      transfer16(value);
      endTransaction();
    }

    // Use default copy constructor and assignment operator.
    HardSpiInterface(const HardSpiInterface&) = default;
    HardSpiInterface& operator=(const HardSpiInterface&) = default;

  private:
    T_SPI& mSpi;
    uint8_t const mLatchPin;
};

} // ace_spi

#endif
