/*
 * A program which compiles various implementations of SPI to determine the
 * flash and static memory sizes from the output of the compiler. See the
 * generated README.md for details.
 */

#include <Arduino.h>

// DO NOT MODIFY THIS LINE. This will be overwritten by collect.sh on each
// iteration, incrementing from 0 to N. The Arduino IDE will compile the
// program, then the script will extract the flash and static memory usage
// numbers printed out by the Arduino compiler. The numbers will be printed on
// the STDOUT, which then can be saved to a file specific for a particular
// hardware platform, e.g. "nano.txt" or "esp8266.txt".
#define FEATURE 0

// List of features of AceSPI that we want to gather memory usage numbers.
#define FEATURE_BASELINE 0
#define FEATURE_HARD_SPI 1
#define FEATURE_HARD_SPI_FAST 2
#define FEATURE_SIMPLE_SPI 3
#define FEATURE_SIMPLE_SPI_FAST 4

// A volatile integer to prevent the compiler from optimizing away the entire
// program.
volatile int disableCompilerOptimization = 0;

#if FEATURE > FEATURE_BASELINE
  #include <AceSPI.h>
  #if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
    #include <digitalWriteFast.h>
    #include <ace_spi/SimpleSpiFastInterface.h>
    #include <ace_spi/HardSpiFastInterface.h>
  #endif
  using namespace ace_spi;

  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

  #if FEATURE == FEATURE_HARD_SPI
    using SpiInterface = HardSpiInterface<SPIClass>;
    SpiInterface spiInterface(SPI, LATCH_PIN);

  #elif FEATURE == FEATURE_HARD_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
    SpiInterface spiInterface(SPI);

  #elif FEATURE == FEATURE_SIMPLE_SPI
    using SpiInterface = SimpleSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);

  #elif FEATURE == FEATURE_SIMPLE_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;

  #else
    #error Unknown FEATURE

  #endif
#endif

// TeensyDuino seems to pull in malloc() and free() when a class with virtual
// functions is used polymorphically. This causes the memory consumption of
// FEATURE_BASELINE (which normally has no classes defined, so does not include
// malloc() and free()) to be artificially small which throws off the memory
// consumption calculations for all subsequent features. Let's define a
// throw-away class and call its method for all FEATURES, including BASELINE.
#if defined(TEENSYDUINO)
  class FooClass {
    public:
      virtual void doit() {
        disableCompilerOptimization = 0;
      }
  };

  FooClass* foo;
#endif

void setup() {
#if defined(TEENSYDUINO)
  foo = new FooClass();
#endif

  disableCompilerOptimization = 3;

#if FEATURE == FEATURE_HARD_SPI
  SPI.begin();
  spiInterface.begin();

#elif FEATURE == FEATURE_HARD_SPI_FAST
  SPI.begin();
  spiInterface.begin();

#elif FEATURE == FEATURE_SIMPLE_SPI
  spiInterface.begin();

#elif FEATURE == FEATURE_SIMPLE_SPI_FAST
  spiInterface.begin();

#else
  // No setup() needed for Writers.

#endif
}

void loop() {
#if defined(TEENSYDUINO)
  foo->doit();
#endif

#if FEATURE == FEATURE_BASELINE
  // do nothing

#elif FEATURE == FEATURE_SIMPLE_SPI \
    || FEATURE == FEATURE_SIMPLE_SPI_FAST \
    || FEATURE == FEATURE_HARD_SPI \
    || FEATURE == FEATURE_HARD_SPI_FAST
  // Send 4 bytes, emulating a 4-digit LED module.
  spiInterface.send8(0x11);
  spiInterface.send8(0x33);
  spiInterface.send8(0x55);
  spiInterface.send8(0x77);

#else
  #error Unknown FEATURE

#endif
}
