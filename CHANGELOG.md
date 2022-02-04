# Changelog

* Unreleased
* 0.4 (2020-02-04)
    * Upgrade tool chain and regenerate `MemoryBenchmark` and `AutoBenchmark`.
        * Arduino IDE from 1.8.13 to 1.8.19
        * Arduino CLI from 0.14.0 to 0.20.2
        * Arduino AVR Core from 1.8.3 to 1.8.4
        * STM32duino from 2.0.0 to 2.2.0
        * ESP8266 Core from 2.7.4 to 3.0.2
        * ESP32 Core from 1.0.6 to 2.0.2
        * Teensyduino from 1.53 to 1.56
    * Downgrade SAMD21 to new "Tier 3: May work but unsupported".
    * No functional change. This is a maintenance release.
* 0.3 (2021-08-17)
    * Add `beginTransaction()`, `endTransaction()`, `transfer()`,
     and `transfer16()` for compatibility with the `SPIClass` from `<SPI.h>`.
        * Retain `send8()` and `send16()` as convenience methods that wrap
          the repetitive calls to the above methods.
    * First public release.
* 0.2 (2021-07-30)
    * Add `T_CLOCK_SPEED` template parameter to `HardSpiInterface` and
      `HardSpiFastInterface`, defaulting to 8000000 (8 MHz).
    * Add `examples/MemoryBenchmark` and `examples/AutoBenchmark` to
      calculate memory and CPU consumptions for various implementations.
    * Add GitHub workflows validation of `examples/*`.
    * Rename `SoftSpi*Interface` to `SimpleSpi*Interface`.
* 0.1 (2021-06-25)
    * First GitHub release.
* 2021-06-24
    * Initial extraction from
      [AceSegment](https://github.com/bxparks/AceSegment) library.
