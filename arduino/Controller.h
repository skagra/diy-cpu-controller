#ifndef _CONTROLLER_DOT_H_
#define _CONTROLLER_DOT_H_

#include "ControlLines.h"
#include "EightBitBus.h"

class Controller
{
public:
    typedef void (*StepCallbackFunc)();

private:
    ControlLines *_controlLines;
    EightBitBus *_cdataBus;
    StepCallbackFunc _stepCallback;

    byte _pc = 0;
    byte _ir = 0;
    byte _cuaddr = 0;

    byte opDecode();
    byte addrModeDecode();

    unsigned long makeControlLines(byte rom4, byte rom3, byte rom2, byte rom1);
    bool executePhase(unsigned long doneFlag);
    void pulseClock();
    void step(unsigned long controlLineBits, unsigned int delayMicros = 1);

public:
    Controller(ControlLines *controlLines,
               EightBitBus *cdataBus,
               StepCallbackFunc stepCallback);
    void setMAR(byte value);
    bool p0Fetch();
    bool pInit();
    bool p1Addr();
    bool p2Op();
    byte getIR();
    byte getPC();
    byte getCUAddr();
    void run();
};

#endif