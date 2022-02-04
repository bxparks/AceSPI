# AutoBenchmark

This program determines the speed of various SPI implementations.

**Version**: AceSPI v0.4

**DO NOT EDIT**: This file was auto-generated using `make README.md`.

## Dependencies

This program depends on the following libraries:

* [AceCommon](https://github.com/bxparks/AceCommon)
* [AceSPI](https://github.com/bxparks/AceSPI)

On AVR processors, one of the following libraries is required to run the
`digitalWriteFast()` versions of the low-level drivers:

* https://github.com/watterott/Arduino-Libs/tree/master/digitalWriteFast
* https://github.com/NicksonYap/digitalWriteFast

## How to Generate

This requires the [AUniter](https://github.com/bxparks/AUniter) script
to execute the Arduino IDE programmatically.

The `Makefile` has rules to generate the `*.txt` results file for several
microcontrollers that I usually support, but the `$ make benchmarks` command
does not work very well because the USB port of the microcontroller is a
dynamically changing parameter. I created a semi-automated way of collect the
`*.txt` files:

1. Connect the microcontroller to the serial port. I usually do this through a
USB hub with individually controlled switch.
2. Type `$ auniter ports` to determine its `/dev/ttyXXX` port number (e.g.
`/dev/ttyUSB0` or `/dev/ttyACM0`).
3. If the port is `USB0` or `ACM0`, type `$ make nano.txt`, etc.
4. Switch off the old microontroller.
5. Go to Step 1 and repeat for each microcontroller.

The `generate_table.awk` program reads one of `*.txt` files and prints out an
ASCII table that can be directly embedded into this README.md file. For example
the following command produces the table in the Nano section below:

```
$ ./generate_table.awk < nano.txt
```

Fortunately, we no longer need to run `generate_table.awk` for each `*.txt`
file. The process has been automated using the `generate_readme.py` script which
will be invoked by the following command:
```
$ make README.md
```

The CPU times below are given in microseconds. The "samples" column is the
number of `TimingStats::update()` calls that were made.

## CPU Time Changes

**v0.2:**
* Initial benchmarks.

## Results

The following tables show the number of microseconds taken by various SPI
implementations:

* `SimpleSpiInterface`
* `SimpleSpiFastInterface`
* `HardSpiInterface`
* `HardSpiFastInterface`

The number of bytes transferred is 8 bytes. The hardware SPI from `<SPI.h>` is
configurd for 8 MHz during these benchmarks. The "eff kbps" is the observed
transfer speed in bits per second. It is usually far lower than the requested
frequency in the `SPISettings` due to the overhead of the `beginTransaction()`,
`endTransaction()` and the `digitalWrite()` used to latch the CS pin.

On AVR processors, the "fast" options are available using one of the
digitalWriteFast libraries whose `digitalWriteFast()` functions can be up to 50X
faster if the `pin` number and `value` parameters are compile-time constants. In
addition, the `digitalWriteFast` functions reduce flash memory consumption by
600-700 bytes compared to their non-fast equivalents.

### Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* Arduino AVR Boards 1.8.4
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(HardSpiInterface): 3
sizeof(HardSpiFastInterface): 2
sizeof(SimpleSpiInterface): 3
sizeof(SimpleSpiFastInterface<11, 12, 13>): 1

CPU:
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| HardSpiInterface                        |   112/  117/  124 |    547.0 |
| HardSpiFastInterface                    |    28/   32/   36 |   2000.0 |
| SimpleSpiInterface                      |   860/  890/  960 |     71.9 |
| SimpleSpiFastInterface                  |    76/   78/   84 |    820.5 |
+-----------------------------------------+-------------------+----------+

```

### SparkFun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* SparkFun AVR Boards 1.1.13
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(HardSpiInterface): 3
sizeof(HardSpiFastInterface): 2
sizeof(SimpleSpiInterface): 3
sizeof(SimpleSpiFastInterface<11, 12, 13>): 1

CPU:
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| HardSpiInterface                        |    88/   92/  100 |    695.7 |
| HardSpiFastInterface                    |    28/   28/   32 |   2285.7 |
| SimpleSpiInterface                      |   832/  839/  844 |     76.3 |
| SimpleSpiFastInterface                  |    68/   69/   80 |    927.5 |
+-----------------------------------------+-------------------+----------+

```

### STM32

* STM32 "Blue Pill", STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* STM32duino 2.2.0

```
Sizes of Objects:
sizeof(HardSpiInterface): 8
sizeof(SimpleSpiInterface): 3

CPU:
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| HardSpiInterface                        |   319/  322/  344 |    198.8 |
| SimpleSpiInterface                      |   316/  318/  321 |    201.3 |
+-----------------------------------------+-------------------+----------+

```

### ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* ESP8266 Boards 3.0.2

```
Sizes of Objects:
sizeof(HardSpiInterface): 8
sizeof(SimpleSpiInterface): 3

CPU:
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| HardSpiInterface                        |    78/   81/  138 |    790.1 |
| SimpleSpiInterface                      |   340/  341/  371 |    187.7 |
+-----------------------------------------+-------------------+----------+

```

### ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* ESP32 Boards 2.0.2

```
Sizes of Objects:
sizeof(HardSpiInterface): 8
sizeof(SimpleSpiInterface): 3

CPU:
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| HardSpiInterface                        |    68/   70/   95 |    914.3 |
| SimpleSpiInterface                      |    28/   28/   35 |   2285.7 |
+-----------------------------------------+-------------------+----------+

```

### Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.19, Arduino CLI 0.20.2
* Teensyduino 1.56
* Compiler options: "Faster"

```
Sizes of Objects:
sizeof(HardSpiInterface): 8
sizeof(SimpleSpiInterface): 3

CPU:
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| HardSpiInterface                        |    18/   18/   21 |   3555.6 |
| SimpleSpiInterface                      |    66/   66/   68 |    969.7 |
+-----------------------------------------+-------------------+----------+

```

