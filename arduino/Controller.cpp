#include <avr/pgmspace.h>

#include "Controller.h"
#include "Pins.h"
#include "Debug.h"

#include "uc/mModeDecoder.h"
#include "uc/mOpDecoder.h"
#include "uc/uROM_0.h"
#include "uc/uROM_1.h"
#include "uc/uROM_2.h"
#include "uc/uROM_3.h"
#include "uc/uROM_4.h"

// High and low time for a clock cycle
#define CLOCK_PULSE_DELAY_MICROS 100
#define PC_TO_MAR (PC_OUT_CADDR | MAR_LD_CADDR)

Controller::Controller(ControlLines *controlLines, EightBitBus *cdataBus)
{
    _controlLines = controlLines;
    _cdataBus = cdataBus;

    pinMode(CLOCK_PIN, OUTPUT);
    digitalWrite(CLOCK_PIN, LOW);
}

byte Controller::getCUAddr()
{
    return _cuaddr;
}

byte Controller::getIR()
{
    return _ir;
}

byte Controller::getPC()
{
    return _pc;
}

void Controller::pulseClock()
{
    digitalWrite(CLOCK_PIN, HIGH);
    delayMicroseconds(CLOCK_PULSE_DELAY_MICROS);
    digitalWrite(CLOCK_PIN, LOW);
    delayMicroseconds(CLOCK_PULSE_DELAY_MICROS);
}

void Controller::step(unsigned long controlLineBits, unsigned int delayMicros)
{
    _controlLines->set(controlLineBits);
    pulseClock();
    delayMicroseconds(delayMicros);
}

void Controller::setMAR(byte value)
{
    debugPrint("Setting MAR", "MAR", value);
    _cdataBus->set(value);
    step(MAR_LD_CADDR | CDATA_TO_CADDR);
    _cdataBus->detach();
}

bool Controller::executePhaseStep(unsigned long doneFlag, bool &error)
{
    bool done = false;

    debugPrint("cuaddr", _cuaddr);

    error = _cuaddr == 0xFF;

    if (!error)
    {
        unsigned long controlLines = makeControlLines(
            pgm_read_byte_near(uROM_4 + _cuaddr),
            pgm_read_byte_near(uROM_3 + _cuaddr),
            pgm_read_byte_near(uROM_2 + _cuaddr),
            pgm_read_byte_near(uROM_1 + _cuaddr));

        debugPrint("Control lines", (unsigned long)controlLines, BASE_BIN, true);

        error = controlLines == 0xFF;

        if (!error)
        {
            if ((controlLines & PC_TO_MAR) == PC_TO_MAR)
            {
                setMAR(_pc);
                controlLines &= ~(MAR_LD_CADDR | CDATA_TO_CADDR);
            }
            done = controlLines & doneFlag;
            if (controlLines & PC_INC)
            {
                _pc++;
                debugPrint("Incremented PC", "PC", _pc);
            }

            if (controlLines)
            {
                step(controlLines);
            }

            _cuaddr++;

            if (!done)
            {
                debugPrintln();
            }
        }
    }

    return done;
}

const char *Controller::PhaseToText(Phase phase)
{
    switch (phase)
    {
    case Phase::pI:
        return "Init";
        break;
    case Phase::p0:
        return "P0 (Fetch)";
        break;
    case Phase::p1:
        return "P1 (Addr)";
        break;
    case Phase::p2:
        return "P2 (Op)";
        break;
    }
}

void Controller::announcePhaseStart(Phase phase)
{
    Serial.print(PhaseToText(phase));
    Serial.println(" --->");
}

void Controller::announcePhaseEnd(Phase phase)
{
    Serial.print("<--- ");
    Serial.println(PhaseToText(phase));
    Serial.println();
}

void Controller::uStep(bool &programComplete, bool &error)
{
    bool phaseDone = false;

    if (newPhase)
    {
        if (_phase == Phase::p0)
        {
            Serial.println("================================");
            Serial.println();
        }
        announcePhaseStart(_phase);
    }

    switch (_phase)
    {
    case Phase::pI:
        if (newPhase)
        {
            newPhase = false;
        }

        phaseDone = executePhaseStep(uP0, error);

        if (phaseDone)
        {
            announcePhaseEnd(_phase);

            _phase = p0;
            newPhase = true;
        }
        break;

    case Phase::p0:
        error = p0Fetch();
        programComplete = _ir == 0;

        announcePhaseEnd(_phase);

        _phase = p1;
        newPhase = true;

        break;
    case Phase::p1:
        if (newPhase)
        {
            newPhase = false;
            _cuaddr = addrModeDecode();
        }

        phaseDone = executePhaseStep(uP2, error);

        if (phaseDone)
        {
            announcePhaseEnd(_phase);
            _phase = p2;
            newPhase = true;
        }
        break;
    case Phase::p2:
        if (newPhase)
        {
            newPhase = false;
            _cuaddr = opDecode();
        }
        phaseDone = executePhaseStep(uP0, error);
        if (phaseDone)
        {
            announcePhaseEnd(_phase);
            _phase = p0;
            newPhase = true;
        }
        break;
    }
}

bool Controller::p0Fetch()
{
    bool error = false;

    debugPrint("PC", _pc);
    debugPrintln();

    // Set MAR to pc
    setMAR(_pc);

    // Load the current instruction into the MBR
    step(MEM_OUT_XDATA | MBR_LD_XDATA);

    // Transfer the MBR into IR
    step(MBR_OUT_CDATA);
    _ir = _cdataBus->read();
    error = _ir == 0xFF;

    if (!error)
    {
        _pc++;

        debugPrintln();
        debugPrint("IR", _ir);
    }

    return error;
}

byte Controller::addrModeDecode()
{
    byte result = pgm_read_byte_near(mModeDecoder + _ir);
    return result;
}

unsigned long Controller::makeControlLines(byte rom4, byte rom3, byte rom2, byte rom1)
{
    unsigned long result = ((unsigned long)rom4) << 24 | ((unsigned long)rom3) << 16 | ((unsigned long)rom2) << 8 | rom1;

    return result;
}

byte Controller::opDecode()
{
    byte result = pgm_read_byte_near(mOpDecoder + _ir);

    return result;
}

void Controller::reset()
{
    _phase = Phase::pI;
    _cuaddr = 0;
    _pc = 0;
}

void Controller::run()
{
    reset();
    go();
}

void Controller::go()
{
    bool done = false;
    bool error = false;

    while (!done && !error)
    {
        uStep(done, error);
    }

    if (error)
    {
        Serial.println();
        Serial.println("===============");
        Serial.println("==== ERROR ====");
        debugPrintln("Error condition: ");
        debugPrint("IR", _ir, BASE_HEX, false);
        debugPrint(", PC", _pc, BASE_HEX, false);
        debugPrint(", cuaddr", _cuaddr, BASE_HEX, true);
        Serial.println("===============");
        Serial.println();

        while (true)
        {
            delay(1000);
        }
    }
}
