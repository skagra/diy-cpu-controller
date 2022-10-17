#include "ControlLines.h"
#include "Pins.h"

#define NUM_CONTROL_PINS 7

#define SHIFT_DATA_PIN 2  // -> Shift register 1 SER pin (14)
#define SHIFT_CLK_PIN 3   // -> Shift registers SRCLCK pins (11)
#define SHIFT_LATCH_PIN 4 // -> Shift registers RCLC pins (12)

ControlLines::ControlLines()
{
    pinMode(SHIFT_DATA_PIN, OUTPUT);
    pinMode(SHIFT_CLK_PIN, OUTPUT);
    pinMode(SHIFT_LATCH_PIN, OUTPUT);

    reset();
}

void ControlLines::set(byte lines)
{
    digitalWrite(SHIFT_LATCH_PIN, LOW);
    shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, lines);
    digitalWrite(SHIFT_LATCH_PIN, HIGH);
}

void ControlLines::reset()
{
    set(0);
}