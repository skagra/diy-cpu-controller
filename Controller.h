#ifndef _CONTROLLER_DOT_H
#define _CONTROLLER_DOT_H

#include "ControlLines.h"

class Controller
{
private:
    ControlLines *_controlLines;

public:
    Controller(ControlLines *controlLines);
    void pulseClock();
    void step(byte controlLineBits, unsigned int delayMicros = 1);
};

#endif