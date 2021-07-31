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

/*
 * A sketch that generates the min/avg/max duration (in microsecondes) of the
 * rendering logic of various SPI implementations. See the generated README.md
 * for more information.
 */

#include <Arduino.h>
#include <SPI.h> // SPIClass
#include <AceCommon.h> // TimingStats
#include <AceSPI.h>

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
#include <digitalWriteFast.h>
#include <ace_spi/SimpleSpiFastInterface.h>
#include <ace_spi/HardSpiFastInterface.h>
#endif

using namespace ace_spi;
using ace_common::TimingStats;

#if ! defined(SERIAL_PORT_MONITOR)
#define SERIAL_PORT_MONITOR Serial
#endif

//------------------------------------------------------------------
// Setup for SPI parameters.
//------------------------------------------------------------------

const uint8_t LATCH_PIN = SS;
const uint8_t DATA_PIN = MOSI;
const uint8_t CLOCK_PIN = SCK;

//------------------------------------------------------------------
// Run benchmarks.
//------------------------------------------------------------------

/** Print the result for each LedMatrix algorithm. */
static void printStats(
    const __FlashStringHelper* name,
    const TimingStats& stats,
    uint16_t numSamples) {
  SERIAL_PORT_MONITOR.print(name);
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(stats.getMin());
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(stats.getAvg());
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(stats.getMax());
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.println(numSamples);
}

TimingStats timingStats;

template <typename T_SPII>
void runBenchmark(const __FlashStringHelper* name, T_SPII& spiInterface) {
  // Sample 20 times.
  uint16_t numSamples = 20;
  timingStats.reset();
  for (uint16_t i = 0; i < numSamples; i++) {
    // Send 8 bytes, emulating an LED module with 8 digits.
    uint16_t startMicros = micros();
    spiInterface.send8(0x11);
    spiInterface.send8(0x22);
    spiInterface.send8(0x33);
    spiInterface.send8(0x44);
    spiInterface.send8(0x55);
    spiInterface.send8(0x66);
    spiInterface.send8(0x77);
    spiInterface.send8(0x88);
    uint16_t endMicros = micros();
    timingStats.update(endMicros - startMicros);
    yield();
  }

  printStats(name, timingStats, numSamples);
}

//-----------------------------------------------------------------------------
// Specific SPI implementations
//-----------------------------------------------------------------------------

void runSimpleSpi() {
  using SpiInterface = SimpleSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);

  spiInterface.begin();
  runBenchmark(F("SimpleSpiInterface"), spiInterface);
  spiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runSimpleSpiFast() {
  using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;

  spiInterface.begin();
  runBenchmark(F("SimpleSpiFastInterface"), spiInterface);
  spiInterface.end();
}
#endif

void runHardSpi() {
  using SpiInterface = HardSpiInterface<SPIClass>;
  SpiInterface spiInterface(SPI, LATCH_PIN);

  SPI.begin();
  spiInterface.begin();
  runBenchmark(F("HardSpiInterface"), spiInterface);
  spiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runHardSpiFast() {
  using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
  SpiInterface spiInterface(SPI);

  SPI.begin();
  spiInterface.begin();
  runBenchmark(F("HardSpiFastInterface"), spiInterface);
  spiInterface.end();
}
#endif

//-----------------------------------------------------------------------------
// runBenchmarks()
//-----------------------------------------------------------------------------

void runBenchmarks() {
  runHardSpi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runHardSpiFast();
#endif

  runSimpleSpi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runSimpleSpiFast();
#endif
}

//-----------------------------------------------------------------------------
// sizeof()
//-----------------------------------------------------------------------------

void printSizeOf() {
  SERIAL_PORT_MONITOR.print(F("sizeof(HardSpiInterface): "));
  SERIAL_PORT_MONITOR.println(sizeof(HardSpiInterface<SPIClass>));

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.print(F("sizeof(HardSpiFastInterface): "));
  SERIAL_PORT_MONITOR.println(sizeof(HardSpiFastInterface<SPIClass, 11>));
#endif

  SERIAL_PORT_MONITOR.print(F("sizeof(SimpleSpiInterface): "));
  SERIAL_PORT_MONITOR.println(sizeof(SimpleSpiInterface));

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.print(F("sizeof(SimpleSpiFastInterface<11, 12, 13>): "));
  SERIAL_PORT_MONITOR.println(sizeof(SimpleSpiFastInterface<11, 12, 13>));
#endif
}

//-----------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait for Leonardo/Micro

  SERIAL_PORT_MONITOR.println(F("SIZEOF"));
  printSizeOf();

  SERIAL_PORT_MONITOR.println(F("BENCHMARKS"));
  runBenchmarks();

  SERIAL_PORT_MONITOR.println(F("END"));

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {}
