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

Controller::Controller(ControlLines *controlLines, EightBitBus *cdataBus,
                       StepCallbackFunc stepCallback)
{
    _controlLines = controlLines;
    _cdataBus = cdataBus;
    _stepCallback = stepCallback;

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
    if (_stepCallback)
    {
        _stepCallback();
    }
}

void Controller::setMAR(byte value)
{
    debugPrint("Setting MAR", "MAR", value);
    _cdataBus->set(value);
    step(MAR_LD_CADDR | CDATA_TO_CADDR);
    _cdataBus->detach();
}

bool Controller::executePhase(unsigned long doneFlag)
{
    bool error = false;
    bool done = false;

    while (!done && !error)
    {
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
    }

    return error;
}

bool Controller::p0Fetch()
{
    bool error = false;

    debugPrintln("\nP0 --->");
    debugPrint("PC", _pc);
    debugPrintln();

    // Save MBR
    step(MBR_OUT_CDATA);
    byte savedMBR = _cdataBus->read();

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
        // Restore original MBR
        _cdataBus->set(savedMBR);
        step(MBR_LD_CDATA);
        _cdataBus->detach();

        // Increment the pc
        _pc++;

        debugPrintln();
        debugPrint("IR", _ir);
        debugPrintln("<--- P0");
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

bool Controller::pInit()
{
    bool error = false;

    debugPrintln("\nPInit --->");
    debugPrint("PC", _pc);
    debugPrintln();

    _cuaddr = 0;
    _pc = 0;

    error = executePhase(uP0);

    debugPrintln("<--- P1");

    return error;
}

bool Controller::p1Addr()
{
    bool error = false;

    debugPrintln("\nP1 --->");
    debugPrint("PC", _pc);
    debugPrintln();

    byte addrModeAddr = addrModeDecode();
    _cuaddr = addrModeAddr;

    error = executePhase(uP2);

    debugPrintln("<--- P1");

    return error;
}

byte Controller::opDecode()
{
    byte result = pgm_read_byte_near(mOpDecoder + _ir);

    return result;
}

bool Controller::p2Op()
{
    bool error = false;

    debugPrintln("\nP2 --->");
    debugPrint("PC", _pc);
    debugPrintln();

    byte addrOpCodeAddr = opDecode();
    _cuaddr = addrOpCodeAddr;

    error = executePhase(uP0);

    debugPrintln("<--- P2");

    return error;
}

void Controller::run()
{
    bool done = false;
    bool error = false;

    // _pc = 0;
    // _cdataBus = 0;

    // pInit();

    do
    {
        error = p0Fetch();
        done = _ir == 0;

        if (!done && !error)
        {
            if (!done && !error)
            {
                error = p1Addr();

                if (!error)
                {
                    error = p2Op();
                }
            }
        }
    } while (!done && !error);

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