# Memory Benchmark

The `MemoryBenchmark.ino` collects the amount of flash and static memory
consumed by different implementations in the AceSPI library.

It compiles various code snippets which are controlled by the `FEATURE` macro
flag. The `collect.sh` edits this `FEATURE` flag programmatically, then runs the
Arduino IDE compiler on the program, and extracts the flash and static memory
usage into a text file (e.g. `nano.txt`).

The numbers shown below should be considered to be rough estimates. It is often
difficult to separate out the code size of the library from the overhead imposed
by the runtime environment of the processor. For example, it often seems like
the ESP8266 allocates flash memory in blocks of a certain quantity, so the
calculated flash size can jump around in unexpected ways.

**Version**: AceSPI v0.3

**DO NOT EDIT**: This file was auto-generated using `make README.md`.

## How to Generate

This requires the [AUniter](https://github.com/bxparks/AUniter) script
to execute the Arduino IDE programmatically.

The `Makefile` has rules for several microcontrollers:

```
$ make benchmarks
```
produces the following files:

```
attiny.txt
nano.txt
micro.txt
samd.txt
stm32.txt
esp8266.txt
esp32.txt
teensy32.txt
```

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

## Library Size Changes

**v0.3**

* Initial MemoryBenchmark using the old v0.3 implementation from 2018,
before substantional refactoring in 2021.

## Results

The following shows the flash and static memory sizes of the `MemoryBenchmark`
program for various `LedModule` configurations and various Writer classes.

* `SimpleSpiInterface`
* `SimpleSpiFastInterface`
* `HardSpiInterface`
* `HardSpiFastInterface`

### ATtiny85

* 8MHz ATtiny85
* Arduino IDE 1.8.13
* SpenceKonde/ATTinyCore 1.5.2

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |    260/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| HardSpiInterface                |   1060/   20 |   800/    9 |
| HardSpiFastInterface            |    746/   16 |   486/    5 |
| SimpleSpiInterface              |    726/   14 |   466/    3 |
| SimpleSpiFastInterface          |    314/   11 |    54/    0 |
+--------------------------------------------------------------+

```

### Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3

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

### Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |   3472/  151 |     0/    0 |
|---------------------------------+--------------+-------------|
| HardSpiInterface                |   4070/  156 |   598/    5 |
| HardSpiFastInterface            |   4000/  152 |   528/    1 |
| SimpleSpiInterface              |   4028/  154 |   556/    3 |
| SimpleSpiFastInterface          |   3534/  151 |    62/    0 |
+--------------------------------------------------------------+

```

### SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Sparkfun SAMD Core 1.8.3

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |   9940/    0 |     0/    0 |
|---------------------------------+--------------+-------------|
| HardSpiInterface                |  10716/    0 |   776/    0 |
| SimpleSpiInterface              |  10292/    0 |   352/    0 |
+--------------------------------------------------------------+

```

### STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 2.0.0

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |  21420/ 3536 |     0/    0 |
|---------------------------------+--------------+-------------|
| HardSpiInterface                |  25716/ 3724 |  4296/  188 |
| SimpleSpiInterface              |  23864/ 3720 |  2444/  184 |
+--------------------------------------------------------------+

```

### ESP8266

* NodeMCU 1.0, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

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

### ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.6

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        | 197748/13084 |     0/    0 |
|---------------------------------+--------------+-------------|
| HardSpiInterface                | 201800/13328 |  4052/  244 |
| SimpleSpiInterface              | 199532/13272 |  1784/  188 |
+--------------------------------------------------------------+

```

### Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53
* Compiler options: "Faster"

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |  10880/ 4152 |     0/    0 |
|---------------------------------+--------------+-------------|
| HardSpiInterface                |  12940/ 4216 |  2060/   64 |
| SimpleSpiInterface              |  11616/ 4156 |   736/    4 |
+--------------------------------------------------------------+

```

