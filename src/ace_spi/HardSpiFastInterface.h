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

#ifndef ACE_SPI_HARD_SPI_FAST_INTERFACE_H
#define ACE_SPI_HARD_SPI_FAST_INTERFACE_H

#include <stdint.h>
#include <Arduino.h>
#include <SPI.h>

namespace ace_spi {

/**
 * This class is functionally identical to HardSpiInterface except that the GPIO
 * pins are controlled by digitalWriteFast() and pinModeFast() methods. This
 * decreases flash memory consumption by 70 bytes (AceSegment/HybridModule) to
 * 250 (AceSegment/Max7219Module, AceSegment/Hc595Module) bytes.
 *
 * If more than 2-3 or LED modules are used, each using different LatchPins, the
 * HardSpiInterface class might consume less flash memory than this one because
 * HardSpiInterface avoids generating different template instantiations of the
 * HybridModule, Max7219Module, or Hc595Module classes. If flash memory size is
 * a problem, users should experiment with both and compare the difference.
 *
 * @tparam T_SPI the class of the hardware SPI instance, usually SPIClass
 * @tparam T_LATCH_PIN the CS/SS pin that controls the SPI peripheral
 * @tparam T_CLOCK_SPEED the SPI clock speed, default 8000000 (8 MHz)
 */
template <
    typename T_SPI,
    uint8_t T_LATCH_PIN,
    uint32_t T_CLOCK_SPEED = 8000000
>
class HardSpiFastInterface {
  private:
    // Some of the following constants are defined in <SPI.h> so it is not
    // possible to avoid the dependency on <SPI.h>

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
     */
    explicit HardSpiFastInterface(T_SPI& spi) : mSpi(spi) {}

    /**
     * Initialize the HardSpiFastInterface. The hardware SPI object must be
     * initialized using `SPI.begin()` as well.
     */
    void begin() const {
      // To use Hardware SPI on ESP8266, we must set the SCK and MOSI pins to
      // 'SPECIAL' instead of 'OUTPUT'. This is performed by calling
      // SPI.begin(). Also, unlike other Arduino platforms, the SPIClass on
      // the ESP8266 defaults to controlling the SS/CS pin itself, instead of
      // letting the application code control it. The setHwCs(false) lets
      // HardSpiFastInterface control the CS/SS pin.
      // https://www.esp8266.com/wiki/doku.php?id=esp8266_gpio_pin_allocations
      #if defined(ESP8266)
        mSpi.setHwCs(false);
      #endif

      pinModeFast(T_LATCH_PIN, OUTPUT);
    }

    /** Clean up the object. */
    void end() const {
      pinModeFast(T_LATCH_PIN, INPUT);
    }

    /** Begin SPI transaction. Pull latch LOW. */
    void beginTransaction() const {
      mSpi.beginTransaction(SPISettings(T_CLOCK_SPEED, kBitOrder, kSpiMode));
      digitalWriteFast(T_LATCH_PIN, LOW);
    }

    /** End SPI transaction. Pull latch HIGH. */
    void endTransaction() const {
      digitalWriteFast(T_LATCH_PIN, HIGH);
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
    HardSpiFastInterface(const HardSpiFastInterface&) = default;
    HardSpiFastInterface& operator=(const HardSpiFastInterface&) = default;

  private:
    T_SPI& mSpi;
};

} // ace_spi

#endif
