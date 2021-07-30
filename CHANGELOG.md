# Changelog

* Unreleased
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
