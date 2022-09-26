#include "Protocol.h"
#include "Programmer.h"

using namespace EEPROMProgrammer;

#define BAUD_RATE 57600

Protocol *protocol;

void setup()
{
    Serial.begin(BAUD_RATE);
    protocol = new Protocol(new Programmer());
}

void loop()
{
    protocol->tick();
}