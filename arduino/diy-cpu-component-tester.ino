#include <avr/pgmspace.h>

#include "Pins.h"
#include "EightBitBus.h"
#include "ControlLines.h"
#include "Controller.h"
#include "ControllerUtils.h"
#include "AluOps.h"
#include "Pins.h"

#define BAUD_RATE 57600

#define SLOW_MOTION_MILLIS 1000
#define MENU_INPUT_TIMEOUT_MILLIS 10000

unsigned int slowMotionMillis = 0;
bool waitForKeyPress = false;

EightBitBus *cdataBus = new EightBitBus(DATA_BUS_PIN_LOW);
ControlLines *controlLines = new ControlLines();
Controller *controller = new Controller(controlLines);
ControllerUtils *controllerUtils = new ControllerUtils(cdataBus, controller);

void setup()
{
    Serial.begin(BAUD_RATE);

    while (!Serial)
        delay(100);

    cdataBus->detach();
    controlLines->reset();
}

void menu()
{
    Serial.setTimeout(MENU_INPUT_TIMEOUT_MILLIS);

    bool gotOption = false;
    while (!gotOption)
    {
        Serial.println(F("Select mode: r (run), z (slow motion), s (single step)"));
        while (Serial.available() == 0)
            delay(100);
        byte option = Serial.read();
        gotOption = true;
        switch (option)
        {
        case 'r':
            // Defaults are correct
            Serial.println(F("Normal run mode."));
            slowMotionMillis = 0;
            waitForKeyPress = false;
            break;
        case 'z':
            Serial.println(F("Slow motion mode."));
            slowMotionMillis = SLOW_MOTION_MILLIS;
            waitForKeyPress = false;
            break;
        case 's':
            Serial.println(F("Single step mode."));
            slowMotionMillis = 0;
            waitForKeyPress = true;
            break;
        default:
            gotOption = false;
        }
    }
}

void waitForKey()
{
    if (waitForKeyPress)
    {
        while (Serial.read() == -1)
        {
            delay(100);
        }
    }
}

byte pc = 0;
byte ir = 0;
bool halt = false;

void hexMessage(const char *message, byte value)
{
    Serial.print(message);
    Serial.println(value, HEX);
}

void step(unsigned int controlLines)
{
    controller->step(controlLines);
    delay(slowMotionMillis);
    waitForKey();
}

void setMAR(byte value)
{
    cdataBus->set(value);
    step(MAR_LD_CADDR | CDATA_TO_CADDR);
    cdataBus->detach();
}

void p0Fetch()
{
    Serial.println(F("\nP0 --->"));
    hexMessage("PC=", pc);

    // Save MBR
    step(MBR_OUT_CDATA);
    byte savedMBR = cdataBus->read();

    // Set MAR to pc
    setMAR(pc);

    // Load the current instruction into the MBR
    step(MAR_OUT_XADDR | MEM_OUT_XDATA | MBR_LD_XDATA);

    // Transfer the MBR into IR
    step(MBR_OUT_CDATA);
    ir = cdataBus->read();

    // Restore original MBR
    cdataBus->set(savedMBR);
    step(MBR_LD_CDATA);

    // Increment the pc
    pc++;

    hexMessage("IR=", ir);
    Serial.println(F("<--- P0"));
}

#define ADDR_MODE_IMMEDIATE 0
#define ADDR_MODE_ZP 1
#define ADDR_MODE_INVALID 255

const byte addrModeDecodeMapping[] PROGMEM{};

byte addrModeDecode()
{
    if (ir == 0xA9 || ir == 0x69)
    {
        return ADDR_MODE_IMMEDIATE;
    }
    else if (ir == 0x44 || ir == 0x65)
    {
        return ADDR_MODE_ZP;
    }
    else
        return ADDR_MODE_INVALID;
}

void p1Addr()
{
    Serial.println(F("\nP1 --->"));
    hexMessage("PC=", pc);

    byte mar;
    switch (addrModeDecode())
    {
    case ADDR_MODE_IMMEDIATE:
        Serial.println(F("Mode: IMM"));
        mar = pc;
        setMAR(pc);
        pc++;
        break;
    case ADDR_MODE_ZP:
        Serial.println(F("Mode: ZP"));
        mar = pc;
        setMAR(pc);
        step(MAR_OUT_XADDR | MEM_OUT_XDATA | MBR_LD_XDATA);
        pc++;
        step(MBR_OUT_CDATA | CDATA_TO_CADDR | MAR_LD_CADDR);
        break;
    default:
        Serial.println(F("Invalid addressing mode"));
        halt = true;
        break;
    }
    hexMessage("MAR=", mar);
    Serial.println(F("<--- P1"));
}

#define OP_CODE_LDA 0
#define OP_CODE_ADD 1
#define OP_CODE_INVALID 255

byte opDecode()
{
    if (ir == 0xA9 || ir == 0x44)
    {
        return OP_CODE_LDA;
    }
    else if (ir == 0x69 || ir == 0x65)
    {
        return OP_CODE_ADD;
    }
    else
        return ADDR_MODE_INVALID;
}

void p2Op()
{
    Serial.println(F("\nP2 --->"));
    hexMessage("PC=", pc);

    switch (opDecode())
    {
    case OP_CODE_LDA:
        Serial.println(F("OpCode: LDA"));
        step(MAR_OUT_XADDR | MEM_OUT_XDATA | MBR_LD_XDATA);
        step(MBR_OUT_CDATA | REGISTER_A_LD);
        break;
    case OP_CODE_ADD:
        Serial.println(F("OpCode: ADD"));
        step(REGISTER_A_OUT | ALU_LD_A);
        step(MAR_OUT_XADDR | MEM_OUT_XDATA | MBR_LD_XDATA);
        step(MBR_OUT_CDATA | ALU_LD_B);
        step(ALU_OP_ADD | ALU_OUT | REGISTER_A_LD);
        break;
    default:
        Serial.println(F("Invalid op code"));
        halt = true;
        break;
    }
    Serial.println(F("<--- P2"));
}

void loop()
{
    menu();
    pc = 0;

    bool done = false;
    halt = false;
    do
    {
        p0Fetch();

        done = ir == 0;
        if (!done)
        {
            if (!halt)
            {
                p1Addr();
                if (!halt)
                {
                    p2Op();
                }
            }
        }
    } while (!done && !halt);

    // for (byte addr = 0; addr < 0x0F; addr++)
    // {
    //     pc = addr;
    //     p0Fetch();
    // }

    // setMAR(0x00);

    // Serial.println(F("ROM -> XDATA, XDATA -> MBR (0x07)"));
    // step(MAR_OUT_XADDR | MEM_OUT_XDATA | MBR_LD_XDATA);

    // Serial.println(F("MBR -> ACC (0x07)"));
    // step(MBR_OUT_CDATA | REGISTER_A_LD);

    // Serial.println(F("ACC -> ALUA (0x7)"));
    // step(REGISTER_A_OUT | ALU_LD_A);

    // setMAR(0x01);

    // Serial.println(F("ROM -> XDATA, XDATA -> MBR (0x09)"));
    // step(MAR_OUT_XADDR | MEM_OUT_XDATA | MBR_LD_XDATA);

    // Serial.println(F("MBR -> ALUB (0x09)"));
    // step(MBR_OUT_CDATA | ALU_LD_B);

    // Serial.println(F("Add ALUA and ALUB, put the result on the bus and load it into the A register (0x10)"));
    // step(ALU_OP_ADD | ALU_OUT | REGISTER_A_LD);

    // Serial.println(F("Move the result into the MAR (so we've calculated an address) (0x10)"));
    // step(REGISTER_A_OUT | CDATA_TO_CADDR | MAR_LD_CADDR);

    // Serial.println(F("ROM -> XDATA, XDATA -> MBR (0x08)"));
    // step(MAR_OUT_XADDR | MEM_OUT_XDATA | MBR_LD_XDATA);

    // Serial.println(F("MBR -> ACC (0x08)"));
    // step(MBR_OUT_CDATA | REGISTER_A_LD);
}