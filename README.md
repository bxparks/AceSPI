# AceSPI

[![Validation](https://github.com/bxparks/AceSPI/actions/workflows/validation.yml/badge.svg)](https://github.com/bxparks/AceSPI/actions/workflows/validation.yml)

Unified interface for selecting hardware or software SPI implementations on
Arduino platforms. The code was initially part of the
[AceSegment](https://github.com/bxparks/AceSegment) library, but was extracted
into a separate library so that it can be shared with other projects. It
provides the following implementations:

* `HardSpiInterface`
    * Hardware SPI using `digitalWrite()` to control the latch pin.
    * Depends on `<SPI.h>`.
* `HardSpiFastInterface`
    * Hardware SPI using `digitalWriteFast()` to control the latch pin.
    * Depends on `<SPI.h>`.
* `SimpleSpiInterface`
    * Software SPI using `shiftOut()`
* `SimpleSpiFastInterface`
    * Software SPI using `digitalWriteFast()` on AVR processors
    * Consumes only 9X less flash memory compared to `HardSpiInterface` (62
      bytes of flash compared to 520 bytes).
    * Faster than `HardSpiInterface` (840 kbps versus 550 kbps).

Currently, this library supports writing from master to slave devices. It does
not support reading from slave devices.

This library uses C++ templates to achieve minimal runtime overhead for the
abstraction. In more technical terms, the library provides compile-time
polymorphism instead of runtime polymorphism to avoid the overhead of the
`virtual` keyword.

**Version**: 0.4 (2022-02-04)

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

The latest stable release is available in the Arduino IDE Library Manager.
Search for "AceSPI". Click install.

The development version can be installed by cloning the
[GitHub repository](https://github.com/bxparks/AceSPI), checking out the
default `develop` branch, then manually copying over to or symlinking from the
`./libraries` directory used by the Arduino IDE. (The result is a directory
or link named `./libraries/AceSPI`.)

The `master` branch contains the stable releases.

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
    void begin() const;
    void end() const;
    void beginTransaction() const;
    void endTransaction() const;
    void transfer(uint8_t value) const;
    void transfer16(uint16_t value) const;

    void send8(uint8_t value) const;
    void send16(uint16_t value) const;
    void send16(uint8_t msb, uint8_t lsb) const;
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

The `beginTransaction()` takes possession of the bus, and latches the CS/SS pin
`LOW` to enable the slave device. The `transfer(uint8_t)` and
`transfer16(uint16_t)` correspond to the matching methods in the `SPIClass`
which send the actual bits to the bus. The `endTransaction()` latches the CS/SS
pin `HIGH` to mark the end of the data transfer, and releases the bus.

The `send8(uint8_t)`, `send16(uint16_t)`, and `send16(uint8_t, uint8_t)` are
convenience methods that wrap the following 3 common operations:

* `beginTransaction()` which pulls the CS/SS pin LOW,
* `transfer()` or `transfer16()` to transfer the data,
* `endTransaction()` which pulls the CS/SS pin HIGH.

These can help reduce the repetitive calls to `beginTransaction()` and
`endTransaction()`.

<a name="HardSpiInterface"></a>
### HardSpiInterface

The `HardSpiInterface` object is a thin wrapper around the `SPI` object from
`<SPI.h>`. It implements the unified interface described above like this:

```C++
namespace ace_spi {

template <
    typename T_SPI,
    uint32_t T_CLOCK_SPEED = 8000000
>
class HardSpiInterface {
  public:
    explicit HardSpiInterface(T_SPI& spi, uint8_t latchPin);

    void begin() const;
    void end() const;
    void beginTransaction() const;
    void endTransaction() const;
    void transfer(uint8_t value) const;
    void transfer16(uint16_t value) const;

    void send8(uint8_t value) const;
    void send16(uint16_t value) const;
    void send16(uint8_t msb, uint8_t lsb) const;
};

}
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
    explicit MyClass(const T_SPII& spiInterface)
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
    const T_SPII mSpiInterface; // copied by value
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
namespace ace_spi {

template <
    typename T_SPI,
    uint8_t T_LATCH_PIN,
    uint32_t T_CLOCK_SPEED = 8000000
>
class HardSpiFastInterface {
  public:
    explicit HardSpiFastInterface(T_SPI& spi);

    void begin() const;
    void end() const;
    void beginTransaction() const;
    void endTransaction() const;
    void transfer(uint8_t value) const;
    void transfer16(uint16_t value) const;

    void send8(uint8_t value) const;
    void send16(uint16_t value) const;
    void send16(uint8_t msb, uint8_t lsb) const;
};

}
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
function from one of the external "digitalWriteFast" libraries. According to
[MemoryBenchmark](examples/MemoryBenchmark), `HardSpiFastInterface` saves
about 100 bytes of flash memory compared to `HardSpiInterface`.

<a name="SimpleSpiInterface"></a>
### SimpleSpiInterface

The `SimpleSpiInterface` class is a software "bitbanging" implementation of SPI
using the Arduino built-in `shiftOut()` function, which uses `digitalWrite()`
underneath the covers. Any appropriate GPIO pin can be used for software SPI,
instead of being restricted to the hardware SPI pins.

```C++
namespace ace_spi {

class SimpleSpiInterface {
  public:
    explicit SimpleSpiInterface(
        uint8_t latchPin,
        uint8_t dataPin,
        uint8_t clockPin
    );

    void begin() const;
    void end() const;
    void beginTransaction() const;
    void endTransaction() const;
    void transfer(uint8_t value) const;
    void transfer16(uint16_t value) const;

    void send8(uint8_t value) const;
    void send16(uint16_t value) const;
    void send16(uint8_t msb, uint8_t lsb) const;
};

}
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

The amount of flash memory used by `SimpleSpiInterface` is similar to
`HardSpiInterface`, so the only compelling reason for using `SimpleSpiInterface`
is the ability to use any GPIO pin for the `MOSI`, `SCK` and `CS/SS` pins. The
big advantage of `SimpleSpiInterface` comes into play when the digitalWriteFast
library is used instead, as described below.

<a name="SimpleSpiFastInterface"></a>
### SimpleSpiFastInterface

The `SimpleSpiFastInterface` class is the same as `SimpleSpiInterface` except
that it uses the `digitalWriteFast()` and `pinModeFast()` functions provided by
one of the digitalWriteFast libraries mentioned above. The pin numbers need to
be compile-time constants, so they are passed in as template parameters, like
this:

```C++
namespace ace_spi {

template <uint8_t T_LATCH_PIN, uint8_t T_DATA_PIN, uint8_t T_CLOCK_PIN>
class SimpleSpiFastInterface {
  public:
    explicit SimpleSpiFastInterface();

    void begin() const;
    void end() const;
    void beginTransaction() const;
    void endTransaction() const;
    void transfer(uint8_t value) const;
    void transfer16(uint16_t value) const;

    void send8(uint8_t value) const;
    void send16(uint16_t value) const;
    void send16(uint8_t msb, uint8_t lsb) const;
};

}
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

According to [MemoryBenchmark](examples/MemoryBenchmark), the use of a
digitialWriteFast library eliminates the pin-to-port mapping arrays, and reduces
the flash memory consumption by about 450 bytes on AVR processors. Only a mere
72 bytes of flash is consumed by this implementation on an AVR. And
[AutoBenchmark](examples/AutoBenchmark) shows that `SimpleSpiFastInterface` can
be almost as fast as the hardware `<SPI.h>` library. On resource constrained
applications on AVR processors, the `SimpleSpiFastInterface` is a worthy
alternative.

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
    explicit MyClass(const T_SPII& spiInterface)
        : mSpiInterface(spiInterface)
    {...}

    [...]

  private:
    const T_SPII& mSpiInterface; // copied by reference
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
| HardSpiInterface                |    978/   16 |   522/    5 |
| HardSpiFastInterface            |    884/   12 |   428/    1 |
| SimpleSpiInterface              |    936/   14 |   480/    3 |
| SimpleSpiFastInterface          |    518/   11 |    62/    0 |
+--------------------------------------------------------------+
```

**ESP8266**

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        | 256700/26784 |     0/    0 |
|---------------------------------+--------------+-------------|
| HardSpiInterface                | 258456/26816 |  1756/   32 |
| SimpleSpiInterface              | 257384/26800 |   684/   16 |
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
| HardSpiInterface                        |   108/  117/  124 |    547.0 |
| HardSpiFastInterface                    |    28/   30/   36 |   2133.3 |
| SimpleSpiInterface                      |   860/  891/  956 |     71.8 |
| SimpleSpiFastInterface                  |    76/   76/   84 |    842.1 |
+-----------------------------------------+-------------------+----------+
```

**ESP8266**

```
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| HardSpiInterface                        |    69/   73/  133 |    876.7 |
| SimpleSpiInterface                      |   207/  208/  238 |    307.7 |
+-----------------------------------------+-------------------+----------+
```

<a name="SystemRequirements"></a>
## System Requirements

<a name="Hardware"></a>
### Hardware

**Tier 1: Fully Supported**

These boards are tested on each release:

* Arduino Nano (16 MHz ATmega328P)
* SparkFun Pro Micro (16 MHz ATmega32U4)
* STM32 Blue Pill (STM32F103C8, 72 MHz ARM Cortex-M3)
* NodeMCU 1.0 (ESP-12E module, 80MHz ESP8266)
* WeMos D1 Mini (ESP-12E module, 80 MHz ESP8266)
* ESP32 dev board (ESP-WROOM-32 module, 240 MHz dual core Tensilica LX6)
* Teensy 3.2 (72 MHz ARM Cortex-M4)

**Tier 2: Should work**

These boards should work but I don't test them as often:

* ATtiny85 (8 MHz ATtiny85)
* Arduino Pro Mini (16 MHz ATmega328P)
* Teensy LC (48 MHz ARM Cortex-M0+)
* Mini Mega 2560 (Arduino Mega 2560 compatible, 16 MHz ATmega2560)

**Tier 3: May work, but not supported**

* SAMD21 M0 Mini (48 MHz ARM Cortex-M0+)
    * Arduino-branded SAMD21 boards use the ArduinoCore-API, so are explicitly
      blacklisted. See below.
    * Other 3rd party SAMD21 boards *may* work using the SparkFun SAMD core.
    * However, as of SparkFun SAMD Core v1.8.6 and Arduino IDE 1.8.19, I can no
      longer upload binaries to these 3rd party boards due to errors.
    * Therefore, third party SAMD21 boards are now in this new Tier 3 category.
    * This library may work on these boards, but I can no longer support them.

**Tier Blacklisted**

The following boards are *not* supported and are explicitly blacklisted to allow
the compiler to print useful error messages instead of hundreds of lines of
compiler errors:

* Any platform using the ArduinoCore-API
  (https://github.com/arduino/ArduinoCore-api). For example:
    * Nano Every
    * MKRZero
    * Raspberry Pi Pico RP2040

<a name="ToolChain"></a>
### Tool Chain

* [Arduino IDE 1.8.19](https://www.arduino.cc/en/Main/Software)
* [Arduino CLI 0.20.2](https://arduino.github.io/arduino-cli)
* [SpenceKonde ATTinyCore 1.5.2](https://github.com/SpenceKonde/ATTinyCore)
* [Arduino AVR Boards 1.8.4](https://github.com/arduino/ArduinoCore-avr)
* [Arduino SAMD Boards 1.8.9](https://github.com/arduino/ArduinoCore-samd)
* [SparkFun AVR Boards 1.1.13](https://github.com/sparkfun/Arduino_Boards)
* [SparkFun SAMD Boards 1.8.6](https://github.com/sparkfun/Arduino_Boards)
* [STM32duino 2.2.0](https://github.com/stm32duino/Arduino_Core_STM32)
* [ESP8266 Arduino 3.0.2](https://github.com/esp8266/Arduino)
* [ESP32 Arduino 2.0.2](https://github.com/espressif/arduino-esp32)
* [Teensyduino 1.56](https://www.pjrc.com/teensy/td_download.html)

This library is *not* compatible with:

* Any platform using the
  [ArduinoCore-API](https://github.com/arduino/ArduinoCore-api), for example:
    * [Arduino megaAVR](https://github.com/arduino/ArduinoCore-megaavr/)
    * [MegaCoreX](https://github.com/MCUdude/MegaCoreX)
    * [Arduino SAMD Boards >=1.8.10](https://github.com/arduino/ArduinoCore-samd)

It should work with [PlatformIO](https://platformio.org/) but I have
not tested it.

The library can be compiled under Linux or MacOS (using both g++ and clang++
compilers) using the EpoxyDuino (https://github.com/bxparks/EpoxyDuino)
emulation layer.

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

If you have any questions, comments, or feature requests for this library,
please use the [GitHub
Discussions](https://github.com/bxparks/AceSPI/discussions) for this project. If
you have bug reports, please file a ticket in [GitHub
Issues](https://github.com/bxparks/AceSPI/issues). Feature requests should go
into Discussions first because they often have alternative solutions which are
useful to remain visible, instead of disappearing from the default view of the
Issue tracker after the ticket is closed.

Please refrain from emailing me directly unless the content is sensitive. The
problem with email is that I cannot reference the email conversation when other
people ask similar questions later.

<a name="Authors"></a>
## Authors

Created by Brian T. Park (brian@xparks.net).
