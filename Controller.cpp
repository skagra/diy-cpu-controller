#include "Controller.h"
#include "Pins.h"

#define CLOCK_PULSE_DELAY_MICROS 100

Controller::Controller(ControlLines *controlLines)
{
    _controlLines = controlLines;

    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(CLOCK_PIN, LOW);
    digitalWrite(CLOCK_PIN, LOW);
}

void Controller::pulseClock()
{
    digitalWrite(CLOCK_PIN, HIGH);
    delayMicroseconds(CLOCK_PULSE_DELAY_MICROS);
    digitalWrite(CLOCK_PIN, LOW);
    delayMicroseconds(CLOCK_PULSE_DELAY_MICROS);
}

void Controller::step(byte controlLineBits, unsigned int delayMicros)
{
    _controlLines->setControlLines(controlLineBits);
    pulseClock();
    delayMicroseconds(delayMicros);
}