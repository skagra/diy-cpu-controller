#ifndef _CONTROLLER_DOT_H_
#define _CONTROLLER_DOT_H_

#include "ControlLines.h"
#include "EightBitBus.h"

class Controller
{
public:
    enum Phase
    {
        pI,
        p0,
        p1,
        p2
    };

private:
    ControlLines *_controlLines;
    EightBitBus *_cdataBus;

    Phase _phase = pI;
    bool _newPhase = true;

    byte _pc = 0;
    byte _ir = 0;
    byte _cuaddr = 0;

    unsigned long _currentControlLines;

    bool _mcBreakpointSet;
    byte _mcBreakpoint;

    byte opDecode();
    byte addrModeDecode();

    unsigned long makeControlLines(byte rom4, byte rom3, byte rom2, byte rom1);
    bool executePhase(unsigned long doneFlag);
    void pulseClock();
    void step(unsigned long controlLineBits, unsigned int delayMicros = 1);
    bool executePhaseStep(unsigned long doneFlag, bool &error);
    void announcePhaseStart(Phase phase);
    void announcePhaseEnd(Phase phase);
    const char *PhaseToText(Phase phase);
    bool p0Fetch();
    void setMAR(byte value);
    byte cuaddrNext(unsigned long currentControlLines);

public:
    Controller(ControlLines *controlLines,
               EightBitBus *cdataBus);
    byte getIR();
    byte getPC();
    byte getCUAddr();
    byte getA();
    byte getX();
    byte getALUOut();
    bool getPZ();

    void reset();
    void run();
    void go();
    void uStep(bool &programComplete, bool &mcBreak, bool &error);

    void setMCBreakpoint(byte breakpoint);
    void clearMCBreakpoint();
};

#endif