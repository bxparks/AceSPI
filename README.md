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
* `SimpleSpiInterface.h`
    * Software SPI using `shiftOut()`
* `SimpleSpiFastInterface.h`
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
    * [SimpleSpiInterface](#SimpleSpiInterface)
    * [SimpleSpiFastInterface](#SimpleSpiFastInterface)
    * [Storing Interface Objects](#StoringInterfaceObjects)
    * [Multiple SPI Buses](#MultipleSpiBuses)
        * [STM32](#MultipleSpiBusesSTM32)
        * [ESP32](#MultipleSpiBusesESP32)
* [Resource Consumption](#ResourceConsumption)
    * [Flash And Static Memory](#FlashAndStaticMemory)
    * [CPU Cycles](#CpuCycles)
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

The "Fast" versions (`HardSpiFastInterface.h`, `SimpleSpiFastInterface.h`)
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
using ace_spi::SimpleSpiInterface;
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
  #include <ace_spi/SimpleSpiFastInterface.h>
  using ace_spi::HardSpiFastInterface;
  using ace_spi::SimpleSpiFastInterface;
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

Notice that the classes in this library do *not* inherit from a common interface
with virtual functions. This saves several hundred bytes of flash memory on
8-bit AVR processors by avoiding the dynamic dispatch, and often allows the
compiler to optimize away the overhead of calling the methods in this library so
that the function call is made directly to the underlying implementation. The
reduction of flash memory consumption is especially large for classes that use
the digitalWriteFast libraries which use compile-time constants for pin numbers.
The disadvantage is that this library is harder to use because these classes
require the downstream classes to be implemented using C++ templates.

<a name="HardSpiInterface"></a>
### HardSpiInterface

The `HardSpiInterface` object is a thin wrapper around the `SPI` object from
`<SPI.h>`. It implements the unified interface described above like this:

```C++
template <
    typename T_SPI,
    uint32_t T_CLOCK_SPEED = 8000000
>
class HardSpiInterface {
  public:
    explicit HardSpiInterface(T_SPI& spi, uint8_t latchPin) :
        mSpi(spi),
        mLatchPin(latchPin)
    {}

    void begin() { ... }
    void end() { ... }

    void send8(uint8_t value) { ... }
    void send16(uint16_t value) { ... }
    void send16(uint8_t msb, uint8_t lsb) { ... }
};
```

The calling code `MyClass` that uses `HardSpiInterface` is configured like this:

```C++
#include <Arduino.h>
#include <SPI.h>
#include <AceSPI.h>
using ace_spi::HardSpiInterface;

template <typename T_SPII>
class MyClass {
  public:
    explicit MyClass(T_SPII& spiInterface)
        : mSpiInterface(spiInterface)
    {...}

    void writeData() {
      // Send 1 byte.
      uint8_t b = ...;
      mSpiInterface.send8(d);

      // Send 2 bytes.
      uint8_t msb = ...;
      uint8_t lsb = ...;
      mSpiInterface.send16(msb, lsb);

      // Send 1 word, msb first.
      uint16_t w = ...;
      mSpiInterface.send16(w);
    }

  private:
    T_SPII mSpiInterface; // copied by value
};

const uint8_t LATCH_PIN = SS;

using SpiInterface = HardSpiInterface<SPIClass>;
SpiInterface spiInterface(spiInstance, LATCH_PIN);
MyClass<SpiInterface> myClass(spiInterface);

void setup() {
  SPI.begin();
  spiInterface.begin();
  ...
}
```

The `using` statement is the C++11 version of a `typedef` that defines
`SpiInterface`. It is not strictly necessary here, but it allows the same
pattern to be used for the more complicated examples below.

The `T_SPII` template parameter contains a `T_` prefix to avoid name collisions
with too many `#define` macros defined in the global namespace on Arduino
platforms. The double `II` contains 2 `Interface`, the first referring to the
SPI protocol, and the second referring to classes in this library.

The latching of the device is attached to the `SS` pin. Other pins can be used.
The latching is performed using the normal `digitalWrite()` function.

The SPI clock speed is defaults to 8000000 (8 MHz), but can be overridden
through one of the template parameters. This class currently supports only
`SPI_MODE0` and `MSBFIRST`. If other SPI configurations are need, it is probably
easiest to just copy the `HardSpiInterface` class and customize it.

<a name="HardSpiFastInterface"></a>
### HardSpiFastInterface

The `HardSpiFastInterface` is identical to `HardSpiInterface` except that it
uses one of the digitalWriteFast libraries listed above, which reduces flash
consumption on AVR processors, and makes the code run faster.

```C++
template <
    typename T_SPI,
    uint8_t T_LATCH_PIN,
    uint32_t T_CLOCK_SPEED = 8000000
>
class HardSpiFastInterface {
  public:
    explicit HardSpiFastInterface(T_SPI& spi) : mSpi(spi) {}

    void begin() { ... }
    void end() { ... }

    void send8(uint8_t value) { ... }
    void send16(uint16_t value) { ... }
    void send16(uint8_t msb, uint8_t lsb) { ... }
};
```

The calling code `MyClass` that uses `HardSpiInterface` is configured like this:

```C++
#include <Arduino.h>
#include <SPI.h>
#include <AceSPI.h>
#if defined(ARDUINO_ARCH_AVR)
  #include <ace_spi/HardSpiFastInterface.h>
  #include <digitalWriteFast.h>
  using ace_spi::HardSpiFastInterface;
#endif

template <typename T_SPII>
class MyClass {
  // Exactly the same as above.
};

const uint8_t LATCH_PIN = SS;

using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
SpiInterface spiInterface(spiInstance);
MyClass<SpiInterface> myClass(spiInterface);

void setup() {
  SPI.begin();
  spiInterface.begin();
  ...
}
```

The latching on the `SS` pin is performed using the `digitalWriteFast()`
function from one of the external "digitalWriteFast" libraries.

<a name="SimpleSpiInterface"></a>
### SimpleSpiInterface

The `SimpleSpiInterface` class is a simple implementation of SPI using the Arduino
built-in `shiftOut()` function, which uses `digitalWrite()` underneath the
covers. Any appropriate GPIO pin can be used for software SPI, instead of being
restricted to the hardware SPI pins.

```C++
class SimpleSpiInterface {
  public:
    explicit SimpleSpiInterface(
        uint8_t latchPin,
        uint8_t dataPin,
        uint8_t clockPin
    ) :
        mLatchPin(latchPin),
        mDataPin(dataPin),
        mClockPin(clockPin)
    {}

    void begin() { ... }
    void end() { ... }

    void send8(uint8_t value) { ... }
    void send16(uint16_t value) { ... }
    void send16(uint8_t msb, uint8_t lsb) { ... }
};
```

We can make our `MyClass` use this interface like this:

```C++
#include <Arduino.h>
#include <AceSPI.h>
using ace_spi::SimpleSpiInterface;

template <typename T_SPII>
class MyClass {
  // Exactly the same as above.
};

const uint8_t DATA_PIN = MOSI;
const uint8_t CLOCK_PIN = SCK;
const uint8_t LATCH_PIN = SS;

using SpiInterface = SimpleSpiInterface;
SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
MyClass<SpiInterface> myClass(spiInterface);

void setup() {
  spiInterface.begin();
  ...
}
```

<a name="SimpleSpiFastInterface"></a>
### SimpleSpiFastInterface

The `SimpleSpiFastInterface` class is the same as `SimpleSpiInterface` except that
it uses the `digitalWriteFast()` and `pinModeFast()` functions provided by one
of the digitalWriteFast libraries mentioned above. The pin numbers need to be
compile-time constants, so they are passed in as template parameters, like this:

```C++
template <uint8_t T_LATCH_PIN, uint8_t T_DATA_PIN, uint8_t T_CLOCK_PIN>
class SimpleSpiFastInterface {
  public:
    explicit SimpleSpiFastInterface() = default;

    void begin() { ... }
    void end() { ... }

    void send8(uint8_t value) { ... }
    void send16(uint16_t value) { ... }
    void send16(uint8_t msb, uint8_t lsb) { ... }
};
```
The code to configure the client code `MyClass` looks very similar:

```C++
#include <Arduino.h>
#include <AceSPI.h>
#if defined(ARDUINO_ARCH_AVR)
  #include <digitalWriteFast.h>
  #include <ace_spi/SimpleSpiFastInterface.h>
  using ace_spi::SimpleSpiFastInterface;
#endif

template <typename T_SPII>
class MyClass {
  // Exactly the same as above.
};

const uint8_t DATA_PIN = MOSI;
const uint8_t CLOCK_PIN = SCK;
const uint8_t LATCH_PIN = SS;

using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
SpiInterface spiInterface;
MyClass<SpiInterface> myClass(spiInterface);

void setup() {
  spiInterface.begin();
  ...
}
```

<a name="StoringInterfaceObjects"></a>
### Storing Interface Objects

In the above examples, the `MyClass` object holds the `T_SPII` interface object
**by value**. In other words, the interface object is copied into the `MyClass`
object. This is efficient because interface objects are very small in size, and
copying them by-value avoids an extra level of indirection when they are used
inside the `MyClass` object. The compiler will generate code that is equivalent
to calling the underlying `SPIClass` methods through an `SPIClass` pointer.

The alternative is to save the `T_SPII` object **by reference** like this:

```C++
template <typename T_SPII>
class MyClass {
  public:
    explicit MyClass(T_SPII& spiInterface)
        : mSpiInterface(spiInterface)
    {...}

    [...]

  private:
    T_SPII& mSpiInterface; // copied by reference
};
```

The internal size of the `HardSpiInterface` object is just a single reference to
the `T_SPII` object and one additional byte for the `latchPin`, so there is
almost difference in the static memory size. However, storing the
`mSpiInterface` as a reference causes an unnecessary extra layer of indirection
every time the `mSpiInterface` object is called. In almost every case, I
recommend storing the `XxxInterface` object **by value** into the `MyClass`
object.

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

template <typename T_SPII>
class MyClass {
  // Exactly the same as above.
};

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

template <typename T_SPII>
class MyClass {
  // Exactly the same as above.
};

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

template <typename T_SPII>
class MyClass {
  // Exactly the same as above.
};

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

template <typename T_SPII>
class MyClass {
  // Exactly the same as above.
};

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

<a name="ResourceConsumption"></a>
## Resource Consumption

<a name="FlashAndStaticMemory"></a>
### Flash And Static Memory

The Memory benchmark numbers can be seen in
[examples/MemoryBenchmark](examples/MemoryBenchmark). Here are 2 samples:

**Arduino Nano**

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |    456/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| SimpleSpiInterface              |    936/   14 |   480/    3 |
| SimpleSpiFastInterface          |    518/   11 |    62/    0 |
| HardSpiInterface                |    978/   16 |   522/    5 |
| HardSpiFastInterface            |    884/   12 |   428/    1 |
+--------------------------------------------------------------+
```

**ESP8266**

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        | 256700/26784 |     0/    0 |
|---------------------------------+--------------+-------------|
| SimpleSpiInterface              | 257384/26800 |   684/   16 |
| HardSpiInterface                | 258456/26816 |  1756/   32 |
+--------------------------------------------------------------+
```

<a name="CpuCycles"></a>
### CPU Cycles

The CPU benchmark numbers can be seen in
[examples/AutoBenchmark](examples/AutoBenchmark). Here are 2 samples:

**Arduino Nano**

```
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| SimpleSpiInterface                      |   860/  891/  956 |     71.8 |
| SimpleSpiFastInterface                  |    76/   76/   84 |    842.1 |
| HardSpiInterface                        |   108/  117/  124 |    547.0 |
| HardSpiFastInterface                    |    28/   30/   36 |   2133.3 |
+-----------------------------------------+-------------------+----------+
```

**ESP8266**

```
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| SimpleSpiInterface                      |   207/  208/  238 |    307.7 |
| HardSpiInterface                        |    69/   73/  133 |    876.7 |
+-----------------------------------------+-------------------+----------+
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
