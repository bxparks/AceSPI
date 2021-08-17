# Changelog

* Unreleased
* 0.3 (2021-08-17)
    * Add `beginTransaction()`, `endTransaction()`, `transfer()`,
     and `transfer16()` for compatibility with the `SPIClass` from `<SPI.h>`.
        * Retain `send8()` and `send16()` as convenience methods that wrap
          the repetitive calls to the above methods.
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
