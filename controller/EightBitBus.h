#ifndef _EIGHT_BIT_BUS_DOT_H_
#define _EIGHT_BIT_BUS_DOT_H_

#include <Arduino.h>

class EightBitBus
{
private:
    int _lowBusPin;

public:
    // Create 8 bus lines starting a lowBusPin
    EightBitBus(int lowBusPin);

    // Set bus pins to write mode and set based on the bits of value
    void set(byte value);

    // Set bus pins to read (so high impedance detached from the bus)
    void detach();

    // Set bus pins to read and grab the current value
    byte read();
};

#endif