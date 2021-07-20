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
 * decreases flash memory consumption by 70 bytes (HybridModule) to 250
 * (Max7219Module, Hc595Module) bytes. However, if multiple LED modules are
 * used, using different LatchPins, the HardSpiInterface might ultimately
 * consume less flash memory because it avoids generating different template
 * instantiations of the HybridModule, Max7219Module, or Hc595Module classes.
 * Users are advised to try both and compare the difference.
 *
 * @tparam T_SPI the class of the hardware SPI instance, usually SPIClass
 * @tparam T_LATCH_PIN the CS/SS pin that controls the SPI peripheral
 */
template <typename T_SPI, uint8_t T_LATCH_PIN>
class HardSpiFastInterface {
  private:
    // Some of the following constants are defined in <SPI.h> so unfortunately,
    // it is not possible to avoid pulling in the global SPI instance into
    // applications which don't use SPI.

    /**
     * MAX7219 has a maximum clock of 16 MHz, so set this to 8 MHz.
     * TODO: Make this a configurable parameter?
     */
    static const uint32_t kClockSpeed = 8000000;

    /** MSB first or LSB first */
  #if defined(ARDUINO_ARCH_STM32)
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

    /** Send 8 bits, including latching LOW and HIGH. */
    void send8(uint8_t value) const {
      mSpi.beginTransaction(SPISettings(kClockSpeed, kBitOrder, kSpiMode));
      digitalWriteFast(T_LATCH_PIN, LOW);
      mSpi.transfer(value);
      digitalWriteFast(T_LATCH_PIN, HIGH);
      mSpi.endTransaction();
    }

    /** Send 16 bits, including latching LOW and HIGH. */
    void send16(uint16_t value) const {
      mSpi.beginTransaction(SPISettings(kClockSpeed, kBitOrder, kSpiMode));
      digitalWriteFast(T_LATCH_PIN, LOW);
      mSpi.transfer16(value);
      digitalWriteFast(T_LATCH_PIN, HIGH);
      mSpi.endTransaction();
    }

    /** Send 2 bytes as 16-bit stream, including latching LOW and HIGH. */
    void send16(uint8_t msb, uint8_t lsb) const {
      uint16_t value = ((uint16_t) msb) << 8 | (uint16_t) lsb;
      send16(value);
    }

    // Use default copy constructor and assignment operator.
    HardSpiFastInterface(const HardSpiFastInterface&) = default;
    HardSpiFastInterface& operator=(const HardSpiFastInterface&) = default;

  private:
    T_SPI& mSpi;
};

} // ace_spi

#endif
