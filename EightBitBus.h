#ifndef _EIGHT_BIT_BUS_DOT_H_
#define _EIGHT_BIT_BUS_DOT_H_

#include <Arduino.h>

class EightBitBus
{
private:
    int _lowBusPin;

public:
    EightBitBus(int lowBusPin);
    void set(byte value);
    void detach();
    byte read();
};

#endif