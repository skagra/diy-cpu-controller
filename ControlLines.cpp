#include "ControlLines.h"
#include "Pins.h"

ControlLines::ControlLines()
{
    pinMode(SHIFT_DATA_PIN, OUTPUT);
    pinMode(SHIFT_CLK_PIN, OUTPUT);
    pinMode(SHIFT_LATCH_PIN, OUTPUT);

    reset();
}

void ControlLines::set(unsigned int lines)
{
    digitalWrite(SHIFT_LATCH_PIN, LOW);

    shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, (byte)lines);
    shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, (byte)(lines >> 8));

    digitalWrite(SHIFT_LATCH_PIN, HIGH);
}

void ControlLines::reset()
{
    set(0);
}