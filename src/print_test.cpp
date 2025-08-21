#include <Arduino.h>
#include "print_test.h"

extern HardwareSerial Serial;

namespace Test {

    void print()
    {
        Serial.println("Test from print_test.cpp");
        return;
    }

}