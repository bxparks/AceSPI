# AceSPI

Unified interface for selecting hardware or software SPI implementations on
Arduino platforms. Uses C++ templates to achieve minimal or zero-cost runtime
overhead for the abstraction. In more technical terms, the library provides
compile-time polymorphism instead of runtime polymorphism to avoid the overhead
of the `virtual` keyword.

The code was initially part of the
[AceSegment](https://github.com/bxparks/AceSegment) library, but was extracted
into a separate library so that it can be shared with other projects. It
provides the following implementations:

* `HardSpiInterface.h`
    * Hardware SPI using `digitalWrite()` to control the latch pin.
    * Depends on `<SPI.h>`.
* `HardSpiFastInterface.h`
    * Hardware SPI using `digitalWriteFast()` to control the latch pin.
    * Depends on `<SPI.h>`.
* `SoftSpiInterface.h`
    * Software SPI using `shiftOut()`
* `SoftSpiFastInterface.h`
    * Software SPI using `digitalWriteFast()` on AVR processors

**Version**: 0.1 (2021-06-25)

**Changelog**: [CHANGELOG.md](CHANGELOG.md)

**See Also**:
* https://github.com/bxparks/AceTMI
* https://github.com/bxparks/AceWire

## Table of Contents

* [Installation](#Installation)
    * [Source Code](#SourceCode)
    * [Dependencies](#Dependencies)
* [Documentation](#Documentation)
* [Usage](#Usage)
    * [Include Header and Namespace](#HeaderAndNamespace)
    * [Unified Interface](#UnifiedInterface)
    * [HardSpiInterface](#HardSpiInterface)
    * [HardSpiFastInterface](#HardSpiFastInterface)
    * [SoftSpiInterface](#SoftSpiInterface)
    * [SoftSpiFastInterface](#SoftSpiFastInterface)
    * [Multiple SPI Buses](#MultipleSpiBuses)
        * [STM32](#MultipleSpiBusesSTM32)
        * [ESP32](#MultipleSpiBusesESP32)
* [System Requirements](#SystemRequirements)
    * [Hardware](#Hardware)
    * [Tool Chain](#ToolChain)
    * [Operating System](#OperatingSystem)
* [License](#License)
* [Feedback and Support](#FeedbackAndSupport)
* [Authors](#Authors)

<a name="Installation"></a>
## Installation

The latest stable release will eventually be available in the Arduino IDE
Library Manager. Search for "AceSPI". Click install. (It is not there
yet.)

The development version can be installed by cloning the
[GitHub repository](https://github.com/bxparks/AceSPI), checking out the
`develop` branch, then manually copying over the contents to the `./libraries`
directory used by the Arduino IDE. (The result is a directory named
`./libraries/AceSPI`.)

The `master` branch contains the stable release.

<a name="SourceCode"></a>
### Source Code

The source files are organized as follows:
* `src/AceSPI.h` - main header file
* `src/ace_spi/` - implementation files
* `docs/` - contains the doxygen docs and additional manual docs

<a name="Dependencies"></a>
### Dependencies

The main `AceSPI.h` does not depend any external libraries.

The "Fast" versions (`HardSpiFastInterface.h`, `SoftSpiFastInterface.h`)
depend on one of the digitalWriteFast libraries, for example:

* https://github.com/watterott/Arduino-Libs/tree/master/digitalWriteFast
* https://github.com/NicksonYap/digitalWriteFast

<a name="Documentation"></a>
## Documentation

* this `README.md` file.
* [Doxygen docs](https://bxparks.github.io/AceSPI/html)
    * On Github pages.
* Examples:
    * https://github.com/bxparks/AceSegment/tree/develop/examples/Hc595Demo
    * https://github.com/bxparks/AceSegment/tree/develop/examples/Max7219Demo

<a name="Usage"></a>
## Usage

<a name="HeaderAndNamespace"></a>
### Include Header and Namespace

In many cases, only a single header file `AceSPI.h` is required to use this
library. To prevent name clashes with other libraries that the calling code may
use, all classes are defined in the `ace_spi` namespace. To use the code without
prepending the `ace_spi::` prefix, use the `using` directive:

```C++
#include <Arduino.h>
#include <SPI.h>
#include <AceSPI.h>
using ace_spi::HardSpiInterface;
using ace_spi::SoftSpiInterface;
```

The "Fast" versions are not included automatically by `AceSPI.h` because they
work only on AVR processors and they depend on a `<digitalWriteFast.h>`
library. To use the "Fast" versions, use something like the following:'

```C++
#include <Arduino.h>
#include <SPI.h>
#include <AceSPI.h>

#if defined(ARDUINO_ARCH_AVR)
  #include <digitalWriteFast.h>
  #include <ace_spi/HardSpiFastInterface.h>
  #include <ace_spi/SoftSpiFastInterface.h>
  using ace_spi::HardSpiFastInterface;
  using ace_spi::SoftSpiFastInterface;
#endif
```

<a name="UnifiedInterface"></a>
### Unified Interface

The classes in this library provide the following unified interface for handling
SPI communication. Downstream classes can code against this unified interface
using C++ templates so that different implementations can be selected at
compile-time.

```C++
class XxxInterface {
  public:
    void begin();
    void end();

    void send8(uint8_t value);
    void send16(uint16_t value);
    void send16(uint8_t msb, uint8_t lsb);
};
```

<a name="HardSpiInterface"></a>
### HardSpiInterface

The `HardSpiInterface` object is a thin wrapper around the `SPI` object from
`<SPI.h>`. It is designed to be used with a templatized class that takes
one of the "Interface" classes in this library.

```C++
#include <Arduino.h>
#include <SPI.h>
#include <AceSPI.h>
using ace_spi::HardSpiInterface;

const uint8_t LATCH_PIN = SS;

template <typename T_SPII>
class MyClass {
  public:
    MyClass(T_SPII& spi)
        : mSpi(spi)
    { ... }

  [...]

  private:
    T_SPII mSpi; // reference will also work
};

using SpiInterface = HardSpiInterface<SPIClass>;
SpiInterface spiInterface(spiInstance, LATCH_PIN);
MyClass<SpiInterface> myClass(spiInterface);

void setup() {
  SPI.begin();
  spiInterface.begin();
  ...
}
```

<a name="HardSpiFastInterface"></a>
### HardSpiFastInterface

The `HardSpiFastInterface` is identical to `HardSpiInterface` except that it
uses one of the digitalWriteFast libraries listed above, which reduces flash
consumption on AVR processors.

```C++
#include <Arduino.h>
#include <SPI.h>
#include <AceSPI.h>
#if defined(ARDUINO_ARCH_AVR)
  #include <ace_spi/HardSpiFastInterface.h>
  #include <digitalWriteFast.h>
  using ace_spi::HardSpiFastInterface;
#endif

const uint8_t LATCH_PIN = SS;

template <typename T_SPII>
class MyClass {
  public:
    MyClass(T_SPII& spi)
        : mSpi(spi)
    { ... }

  [...]

  private:
    T_SPII mSpi; // reference will also work
};

using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
SpiInterface spiInterface(spiInstance);
MyClass<SpiInterface> myClass(spiInterface);

void setup() {
  SPI.begin();
  spiInterface.begin();
  ...
}
```

<a name="SoftSpiInterface"></a>
### SoftSpiInterface

The `SoftSpiInterface` class is a simple implementation of SPI using the Arduino
built-in `shiftOut()` function, which uses `digitalWrite()` underneath the
covers. Any appropriate GPIO pin can be used for software SPI, instead of being
restricted to the hardware SPI pins.  We can make our `MyClass` use this
interface like this:

```C++
#include <Arduino.h>
#include <AceSPI.h>
using ace_spi::SoftSpiInterface;

const uint8_t DATA_PIN = MOSI;
const uint8_t CLOCK_PIN = SCK;
const uint8_t LATCH_PIN = SS;

template <typename T_SPII>
class MyClass {
  public:
    MyClass(T_SPII& spi)
        : mSpi(spi)
    { ... }

  [...]

  private:
    T_SPII mSpi; // reference will also work
};

using SpiInterface = SoftSpiInterface;
SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
MyClass<SpiInterface> myClass(spiInterface);

void setup() {
  spiInterface.begin();
  ...
}
```

<a name="SoftSpiFastInterface"></a>
### SoftSpiFastInterface

The `SoftSpiFastInterface` class is the same as `SoftSpiInterface` except that
it uses the `digitalWriteFast()` function provided by one of the
digitalWriteFast libraries mentioned above. The code looks very similar:

```C++
#include <Arduino.h>
#include <AceSPI.h>
#if defined(ARDUINO_ARCH_AVR)
  #include <digitalWriteFast.h>
  #include <ace_spi/SoftSpiFastInterface.h>
  using ace_spi::SoftSpiFastInterface;
#endif

const uint8_t DATA_PIN = MOSI;
const uint8_t CLOCK_PIN = SCK;
const uint8_t LATCH_PIN = SS;

template <typename T_SPII>
class MyClass {
  public:
    MyClass(T_SPII& spi)
        : mSpi(spi)
    { ... }

  [...]

  private:
    T_SPII mSpi; // reference will also work
};

using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
SpiInterface spiInterface;
MyClass<SpiInterface> myClass(spiInterface);

void setup() {
  spiInterface.begin();
  ...
}
```

<a name="MultipleSpiBuses"></a>
### Multiple SPI Buses

Some processors (e.g. STM32, ESP32) have multiple hardware SPI buses. Here are
some notes about how to configure them.

<a name="MultipleSpiBusesSTM32"></a>
#### STM32 (STM32F103)

The STM32F103 "Blue Pill" has 2 SPI buses:

* SPI1
    * SS1 = SS = PA4
    * SCK1 = SCK = PA5
    * MISO1 = MISO = PA6
    * MOSI1 = MOSI = PA7
* SPI2
    * SS2 = PB12
    * SCK2 = PB13
    * MISO2 = PB14
    * MOSI2 = PB15

The primary (default) SPI interface is used like this:

```C++
#include <Arduino.h>
#include <SPI.h>
#include <AceSPI.h>
using ace_spi::HardSpiInterface;

const uint8_t LATCH_PIN = SS;
const uint8_t DATA_PIN = MOSI;
const uint8_t CLOCK_PIN = SCK;

using SpiInterface = HardSpiInterface<SPIClass>;
SpiInterface spiInterface(SPI, LATCH_PIN);
MyClass<SpiInterface> myClass(spiInterface);

void setup() {
  SPI.begin();
  spiInterface.begin();
  ...
}
```

The second SPI interface can be used like this:

```C++
#include <Arduino.h>
#include <SPI.h>
#include <AceSPI.h>
using ace_spi::HardSpiInterface;

const uint8_t LATCH_PIN = PB12;
const uint8_t DATA_PIN = PB15;
const uint8_t CLOCK_PIN = PB13;

SPIClass spiSecondary(DATA_PIN, PB14 /*miso*/, CLOCK_PIN);
using SpiInterface = HardSpiInterface<SPIClass>;
SpiInterface spiInterface(spiSecondary, LATCH_PIN);
MyClass<SpiInterface> myClass(spiInterface);

void setupAceSegment() {
  spiSecondary.begin();
  spiInterface.begin();
  ...
}
```

<a name="MultipleSpiBusesESP32"></a>
#### ESP32

The ESP32 has 4 SPI buses, of which 2 are available for general purposes. The
default GPIO pin mappings are:

* SPI2 (aka HSPI)
    * MOSI = 13
    * MISO = 12
    * SS = 15
    * SCK = 14
* SPI3 (aka VSPI, default)
    * MOSI = 23
    * MISO = 19
    * SS = 5
    * SCK = 18

(My understanding is that the ESP32 has some sort of GPIO pin remapping
matrix that can reroute these pins to other pins, but my knowledge of this
capability is limited.)

The primary (default) `SPI` instance uses the `VSPI` bus and is used like this:

```C++
#include <Arduino.h>
#include <SPI.h>
#include <AceSPI.h>
using ace_spi::HardSpiInterface;

const uint8_t LATCH_PIN = SS;
const uint8_t DATA_PIN = MOSI;
const uint8_t CLOCK_PIN = SCK;

using SpiInterface = HardSpiInterface<SPIClass>;
SpiInterface spiInterface(SPI, LATCH_PIN);
MyClass<SpiInterface> myClass(spiInterface);

void setupAceSegment() {
  SPI.begin();
  spiInterface.begin();
  ...
}
```

The secondary `HSPI` bus can be used like this:

```C++
#include <Arduino.h>
#include <SPI.h>
#include <AceSPI.h>
using ace_spi::HardSpiInterface;

const uint8_t LATCH_PIN = 15;
const uint8_t DATA_PIN = 13;
const uint8_t CLOCK_PIN = 14;

SPIClass spiSecondary(HSPI);
using SpiInterface = HardSpiInterface<SPIClass>;
SpiInterface spiInterface(spiSecondary, LATCH_PIN);
MyClass<SpiInterface> myClass(spiInterface);

void setupAceSegment() {
  spiSecondary.begin();
  spiInterface.begin();
  ...
}
```

<a name="SystemRequirements"></a>
## System Requirements

<a name="Hardware"></a>
### Hardware

This library has Tier 1 support on the following boards:

* Arduino Nano (16 MHz ATmega328P)
* SparkFun Pro Micro (16 MHz ATmega32U4)
* SAMD21 M0 Mini (48 MHz ARM Cortex-M0+)
* STM32 Blue Pill (STM32F103C8, 72 MHz ARM Cortex-M3)
* NodeMCU 1.0 (ESP-12E module, 80MHz ESP8266)
* WeMos D1 Mini (ESP-12E module, 80 MHz ESP8266)
* ESP32 dev board (ESP-WROOM-32 module, 240 MHz dual core Tensilica LX6)
* Teensy 3.2 (72 MHz ARM Cortex-M4)

Tier 2 support can be expected on the following boards, mostly because I don't
test these as often:

* ATtiny85 (8 MHz ATtiny85)
* Arduino Pro Mini (16 MHz ATmega328P)
* Teensy LC (48 MHz ARM Cortex-M0+)
* Mini Mega 2560 (Arduino Mega 2560 compatible, 16 MHz ATmega2560)

The following boards are **not** supported:

* Any platform using the ArduinoCore-API
  (https://github.com/arduino/ArduinoCore-api). For example:
    * Nano Every
    * MKRZero
    * Raspberry Pi Pico RP2040

<a name="ToolChain"></a>
### Tool Chain

* [Arduino IDE 1.8.13](https://www.arduino.cc/en/Main/Software)
* [Arduino CLI 0.14.0](https://arduino.github.io/arduino-cli)
* [SpenceKonde ATTinyCore 1.5.2](https://github.com/SpenceKonde/ATTinyCore)
* [Arduino AVR Boards 1.8.3](https://github.com/arduino/ArduinoCore-avr)
* [Arduino SAMD Boards 1.8.9](https://github.com/arduino/ArduinoCore-samd)
* [SparkFun AVR Boards 1.1.13](https://github.com/sparkfun/Arduino_Boards)
* [SparkFun SAMD Boards 1.8.3](https://github.com/sparkfun/Arduino_Boards)
* [STM32duino 2.0.0](https://github.com/stm32duino/Arduino_Core_STM32)
* [ESP8266 Arduino 2.7.4](https://github.com/esp8266/Arduino)
* [ESP32 Arduino 1.0.6](https://github.com/espressif/arduino-esp32)
* [Teensyduino 1.53](https://www.pjrc.com/teensy/td_download.html)

<a name="OperatingSystem"></a>
### Operating System

I use Ubuntu 20.04 for the vast majority of my development. I expect that the
library will work fine under MacOS and Windows, but I have not explicitly tested
them.

<a name="License"></a>
## License

[MIT License](https://opensource.org/licenses/MIT)

<a name="FeedbackAndSupport"></a>
## Feedback and Support

If you have any questions, comments and other support questions about how to
use this library, use the
[GitHub Discussions](https://github.com/bxparks/AceSPI/discussions)
for this project. If you have bug reports or feature requests, file a ticket in
[GitHub Issues](https://github.com/bxparks/AceSPI/issues). I'd love to hear
about how this software and its documentation can be improved. I can't promise
that I will incorporate everything, but I will give your ideas serious
consideration.

Please refrain from emailing me directly unless the content is sensitive. The
problem with email is that I cannot reference the email conversation when other
people ask similar questions later.

<a name="Authors"></a>
## Authors

Created by Brian T. Park (brian@xparks.net).
