#include "ControlLines.h"
#include "Pins.h"

ControlLines::ControlLines()
{
    for (int controlPin = 0; controlPin < NUM_CONTROL_PINS; controlPin++)
    {
        pinMode(controlBits[controlPin], OUTPUT);
        digitalWrite(controlBits[controlPin], LOW);
    }
}

int ControlLines::controlBitToPin(byte bitNumber)
{
    return controlBits[bitNumber];
}

void ControlLines::setControlLines(byte lines)
{
    for (byte offset = 0; offset < 7; offset++)
    {
        digitalWrite(controlBitToPin(offset), (lines & ((byte)1 << offset)) ? HIGH : LOW);
    }
}

void ControlLines::resetControlLines()
{
    setControlLines(0);
}