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

#ifndef ACE_SEGMENT_GPIO_INTERFACE_H
#define ACE_SEGMENT_GPIO_INTERFACE_H

#include <stdint.h>
#include <Arduino.h>

namespace ace_segment {

/**
 * A utility lass that provides a thin layer of indirection to various GPIO
 * functions. This allows injection of a different GpioInterface for testing
 * purposes. For normal code, the compiler will optimize away the extra function
 * call.
 */
class GpioInterface {
  public:
    /** Write value to pin. */
    static void digitalWrite(uint8_t pin, uint8_t value) {
    #if defined(ARDUINO_API_VERSION)
      arduino::digitalWrite(pin, value);
    #else
      ::digitalWrite(pin, value);
    #endif
    }

    /** Set pin mode. */
    static void pinMode(uint8_t pin, uint8_t mode) {
    #if defined(ARDUINO_API_VERSION)
      arduino::pinMode(pin, mode);
    #else
      ::pinMode(pin, mode);
    #endif
    }
};

}

#endif
