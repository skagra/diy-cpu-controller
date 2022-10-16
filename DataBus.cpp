#include "DataBus.h"
#include "Pins.h"

void DataBus::set(byte value)
{
    for (byte offset = 0; offset < 8; offset++)
    {
        pinMode(BUS_PIN_LOW + offset, OUTPUT);
        digitalWrite(BUS_PIN_LOW + offset, (value & ((byte)1 << offset)) ? HIGH : LOW);
    }
}

void DataBus::detach()
{
    for (int offset = 0; offset < 8; offset++)
    {
        digitalWrite(BUS_PIN_LOW + offset, LOW);
        pinMode(BUS_PIN_LOW + offset, INPUT);
    }
}

byte DataBus::read()
{
    byte result = 0;
    detach();

    for (int pin = BUS_PIN_LOW; pin <= BUS_PIN_HIGH; pin++)
    {
        if (digitalRead(pin) == HIGH)
        {
            result |= (1 << (pin - BUS_PIN_LOW));
        }
    }

    return result;
}
