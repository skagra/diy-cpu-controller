#include "ControlLines.h"
#include "Pins.h"

#define NUM_CONTROL_PINS 7

ControlLines::ControlLines()
{
    for (int controlPin = 0; controlPin < NUM_CONTROL_PINS; controlPin++)
    {
        pinMode(controlBitToPin(controlPin), OUTPUT);
        digitalWrite(controlBitToPin(controlPin), LOW);
    }
}

int ControlLines::controlBitToPin(byte bitNumber)
{
    return controlBits[bitNumber];
}

void ControlLines::set(byte lines)
{
    for (byte offset = 0; offset < 7; offset++)
    {
        digitalWrite(controlBitToPin(offset), (lines & ((byte)1 << offset)) ? HIGH : LOW);
    }
}

void ControlLines::reset()
{
    set(0);
}