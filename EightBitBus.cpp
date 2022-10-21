#include "EightBitBus.h"
#include "Pins.h"

EightBitBus::EightBitBus(int lowBusPin)
{
    _lowBusPin = lowBusPin;
}

void EightBitBus::set(byte value)
{
    for (byte offset = 0; offset < 8; offset++)
    {
        pinMode(_lowBusPin + offset, OUTPUT);
        digitalWrite(_lowBusPin + offset, (value & ((byte)1 << offset)) ? HIGH : LOW);
    }
}

void EightBitBus::detach()
{
    for (int offset = 0; offset < 8; offset++)
    {
        digitalWrite(_lowBusPin + offset, LOW);
        pinMode(_lowBusPin + offset, INPUT);
    }
}

byte EightBitBus::read()
{
    byte result = 0;
    detach();

    for (int offset = 0; offset < 8; offset++)
    {
        if (digitalRead(_lowBusPin + offset) == HIGH)
        {
            result |= (1 << offset);
        }
    }

    return result;
}
