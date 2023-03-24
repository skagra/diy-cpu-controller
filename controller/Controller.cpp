#include <avr/pgmspace.h>

#include "Controller.h"
#include "Pins.h"
#include "Printer.h"

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
    pinMode(PZ_PIN, INPUT);
    digitalWrite(CLOCK_PIN, LOW);
}

void Controller::setMCBreakpoint(byte breakpoint)
{
    _mcBreakpoint = breakpoint;
    _mcBreakpointSet = true;
}

void Controller::clearMCBreakpoint()
{
    _mcBreakpointSet = false;
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

// Set control lines a pulse clock to execute the current ucode instruction
void Controller::step(unsigned long controlLineBits, unsigned int delayMicros)
{
    _controlLines->set(controlLineBits);
    pulseClock();
    delayMicroseconds(delayMicros);
}

void Controller::setMAR(byte value)
{
    Printer::print("Setting MAR", "MAR", value, Printer::Verbosity::verbose);
    _cdataBus->set(value);
    step(MAR_LD_CADDR | CDATA_TO_CADDR);
    _cdataBus->detach();
}

// Calculate the next ucode addr
byte Controller::cuaddrNext(unsigned long currentControlLines)
{
    byte result;

    if (currentControlLines & uJMP)
    {
        // Absolute jump
        result = uROM_0[_cuaddr];
    }
    else if (currentControlLines & uZJMP)
    {
        // Jump based on Z flag
        bool pz = getPZ();
        if ((currentControlLines & uJMPINV) && !pz)
        {
            // Jump on not zero
            result = uROM_0[_cuaddr];
        }
        else if (pz)
        {
            // Jump if zero
            result = pgm_read_byte_near(uROM_0 + _cuaddr);
        }
        else
        {
            // Default
            result = _cuaddr + 1;
        }
    }
    else
    {
        // Default
        result = _cuaddr + 1;
    }

    return result;
}

bool Controller::executePhaseStep(unsigned long doneFlag, bool &error)
{
    bool done = false;

    Printer::print("cuaddr", _cuaddr, Printer::Verbosity::verbose);

    // It is an error id we have run to the end of he ucode
    error = _cuaddr == 0xFF;

    if (!error)
    {
        // Assemble a long representing the control lines from the current ucode instruction
        _currentControlLines = makeControlLines(
            pgm_read_byte_near(uROM_4 + _cuaddr),
            pgm_read_byte_near(uROM_3 + _cuaddr),
            pgm_read_byte_near(uROM_2 + _cuaddr),
            pgm_read_byte_near(uROM_1 + _cuaddr));

        Printer::print("Control lines", (unsigned long)_currentControlLines,
                       Printer::Verbosity::verbose, Printer::Base::BASE_BIN, true);

        // Uninitialized data is set to 0xFF, so retrieving 0xFF flags an error
        error = _currentControlLines == 0xFF;

        if (!error)
        {
            // Move the PC to the MAR if flagged
            if ((_currentControlLines & PC_TO_MAR) == PC_TO_MAR)
            {
                setMAR(_pc);
                // We've dealt with what both MAR_LD_CADDR and CDATA_TO_CADDR would have done
                // so reset both of them
                _currentControlLines &= ~(MAR_LD_CADDR | CDATA_TO_CADDR);
            }

            // Does the ucode instruction flag to move onto the next phase
            done = _currentControlLines & doneFlag;

            // PC updating

            // Increment the PC if flagged
            if (_currentControlLines & PC_INC)
            {
                _pc++;
                Printer::print("Incremented PC", "PC", _pc, Printer::Verbosity::verbose);
            }
            else if (_currentControlLines & PC_REL_CDATA)
            {
                // Relative jump from value in MBR
                _controlLines->set(MBR_OUT_CDATA);
                _pc += _cdataBus->read();
                _currentControlLines &= ~PC_REL_CDATA;
                Printer::print("Relative branch", "PC", _pc, Printer::Verbosity::verbose);
            }
            else if ((_currentControlLines & (MBR_OUT_CDATA | PC_LD_CDATA)) == (MBR_OUT_CDATA | PC_LD_CDATA))
            {
                // Absolute jump to value in MBR.
                // Only if the only line set are MBR_OUT_CDATA and PC_LD_CDATA.
                _controlLines->set(MBR_OUT_CDATA);
                _pc = _cdataBus->read();
                _currentControlLines &= ~(MBR_OUT_CDATA | PC_LD_CDATA);
                Printer::print("Absolute jump", "PC", _pc, Printer::Verbosity::verbose);
            }

            // Any control lines not processed internally?
            if (_currentControlLines)
            {
                // Have the hardware process them
                step(_currentControlLines);
            }

            // Calculate next ucode addr
            _cuaddr = cuaddrNext(_currentControlLines);

            if (!done)
            {
                Printer::println(Printer::Verbosity::verbose);
            }
        }
    }

    return done;
}

bool Controller::getPZ()
{
    return digitalRead(PZ_PIN);
}

byte Controller::getA()
{
    _controlLines->set(A_OUT_CDATA);
    byte result = _cdataBus->read();
    _controlLines->set(_currentControlLines);
    return result;
}

byte Controller::getX()
{
    _controlLines->set(X_OUT_CDATA);
    byte result = _cdataBus->read();
    _controlLines->set(_currentControlLines);
    return result;
}

byte Controller::getALUOut()
{
    _controlLines->set(ALUR_OUT_CDATA);
    byte result = _cdataBus->read();
    _controlLines->set(_currentControlLines);
    return result;
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
    Printer::print(PhaseToText(phase), Printer::Verbosity::verbose);
    Printer::println(" --->", Printer::Verbosity::verbose);
}

void Controller::announcePhaseEnd(Phase phase)
{
    Printer::print("<--- ", Printer::Verbosity::verbose);
    Printer::println(PhaseToText(phase), Printer::Verbosity::verbose);
    Printer::println(Printer::Verbosity::verbose);
}

// Run a microcode step
void Controller::uStep(bool &programComplete, bool &mcBreak, bool &error)
{
    bool phaseDone = false;

    // Is this the first step in a new phase?
    if (_newPhase)
    {
        if (_phase == Phase::p0)
        {
            Printer::println("================================", Printer::Verbosity::verbose);
            Printer::println(Printer::Verbosity::verbose);
        }
        announcePhaseStart(_phase);
    }

    // What phase are we in?
    switch (_phase)
    {
    case Phase::pI:
        // pI - One time initialization code
        if (_newPhase)
        {
            _newPhase = false;
        }

        // Execute the current microcode instruction - next phase is P0
        phaseDone = executePhaseStep(uP0, error);

        // If the ucode instruction moving onto the next phase
        if (phaseDone)
        {
            announcePhaseEnd(_phase);

            // Flag the new phase
            _phase = p0;
            _newPhase = true;
        }
        break;

    case Phase::p0:
        // p0 - fetch
        mcBreak = _mcBreakpointSet && _pc == _mcBreakpoint;

        // Do current opcode - not using the ucode but doing it directly in the control
        // MAR<-PC, MEM_OUT_XDATA | MBR_LD_XDATA, IR<-MBR_OUT_CDATA
        error = p0Fetch();

        // A zero mcode instuction halts the CPU
        programComplete = _ir == 0;

        announcePhaseEnd(_phase);

        _phase = p1;
        _newPhase = true;

        break;
    case Phase::p1:
        if (_newPhase)
        {
            _newPhase = false;
            // First ucode instruction of p1 so find the ucode to execute the appropriate
            // p1 based on the current mc opcode's addressing mode.
            _cuaddr = addrModeDecode();
        }

        // Execute the current ucode instruction
        phaseDone = executePhaseStep(uP2, error);

        if (phaseDone)
        {
            // Move to next phase (p2) if ucode instruction flags phase as done
            announcePhaseEnd(_phase);
            _phase = p2;
            _newPhase = true;
        }
        break;
    case Phase::p2:
        if (_newPhase)
        {
            _newPhase = false;
            // First ucode instruction of p2 so find the ucode to execute the appropriate
            // p2 based on the current mc opcode function.
            _cuaddr = opDecode();
        }
        phaseDone = executePhaseStep(uP0, error);
        if (phaseDone)
        {
            // Move to next phase (p0) if ucode instruction flags phase as done
            announcePhaseEnd(_phase);
            _phase = p0;
            _newPhase = true;
        }
        break;
    }
}

bool Controller::p0Fetch()
{
    bool error = false;

    Printer::print("PC", _pc, Printer::Verbosity::verbose);
    Printer::println(Printer::Verbosity::verbose);

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

        Printer::println(Printer::Verbosity::verbose);
        Printer::print("IR", _ir, Printer::Verbosity::verbose);
    }

    return error;
}

// Locate address of the ucode for the addressing mode of the opcode in IR
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

// Locate address of the ucode for the function of the opcode in IR
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
    bool mcBreak = false;

    while (!done && !error && !mcBreak)
    {
        uStep(done, mcBreak, error);
    }

    if (mcBreak)
    {
        Printer::println();
        Printer::print("MC Breakpoint hit at");
    }

    if (error)
    {
        Printer::println();
        Printer::println("===============");
        Printer::println("Halted due to ERROR condition.");
        Printer::print("cuaddr", getCUAddr());
        Printer::print("PC", getPC(), Printer::Verbosity::all, Printer::Base::BASE_HEX, false);
        Printer::print("IR", getIR(), Printer::Verbosity::all, Printer::Base::BASE_HEX, false);
        Printer::print("A", getA(), Printer::Verbosity::all, Printer::Base::BASE_HEX, false);
        Printer::print("X", getX(), Printer::Verbosity::all, Printer::Base::BASE_HEX, false);
        Printer::print("ALU", getALUOut(), Printer::Verbosity::all, Printer::Base::BASE_HEX, false);
        Printer::println("===============");
        Printer::println();

        while (true)
        {
            delay(1000);
        }
    }
}
