#include <avr/pgmspace.h>

#include "uc/mModeDecoder.h"
#include "uc/mOpDecoder.h"
#include "uc/uROM_0.h"
#include "uc/uROM_1.h"
#include "uc/uROM_2.h"
#include "uc/uROM_3.h"
#include "uc/uROM_4.h"

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
byte cuAddr = 0;
bool halt = false;

void hexMessage(const char *message, byte value)
{
    Serial.print(message);
    Serial.println(value, HEX);
}

void step(unsigned int controlLines)
{
    Serial.print("Control lines=");
    Serial.println(controlLines, BIN);
    controller->step(controlLines);
    delay(slowMotionMillis);
    waitForKey();
}

#define EXT_A_LD_CDATA 0b00000000000000001000000000000000UL
#define EXT_A_OUT_CDATA 0b00000000000000010000000000000000UL
#define EXT_ALU_LD_A 0b00000000000100000000000000000000UL
#define EXT_ALU_LD_B 0b00000000001000000000000000000000UL
#define EXT_ALU_OP_0 0b00000000100000000000000000000000UL
#define EXT_ALU_OP_1 0b00000001000000000000000000000000UL
#define EXT_ALU_OUT 0b00000000010000000000000000000000UL
#define EXT_MEM_OUT_XDATA 0b00000000000000000000000000001000UL
#define EXT_MAR_LD_CADDR 0b00000000000000000000001000000000UL
#define EXT_MBR_LD_XDATA 0b00000000000000000000000000100000UL
#define EXT_MBR_OUT_CDATA 0b00000000000000000000000100000000UL
#define EXT_CDATA_TO_CADDR 0b00000000000000000000000000000010UL
#define EXT_MBR_LD_CDATA 0b00000000000000000000000010000000UL

#define EXT_P0 0b00010000000000000000000000000000UL
#define EXT_P1 0b00100000000000000000000000000000UL
#define EXT_P2 0b01000000000000000000000000000000UL
#define EXT_INC_PC 0b00000000000000000000010000000000UL
#define EXT_PC_TO_MAR 0b00000000000000000100001000000000UL

int mapControlLineValues(unsigned long value)
{
    int result = 0;

    result |= (value & EXT_A_LD_CDATA) ? A_LD_CDATA : 0;
    result |= (value & EXT_A_OUT_CDATA) ? A_OUT_CDATA : 0;
    result |= (value & EXT_ALU_LD_A) ? ALU_LD_A : 0;
    result |= (value & EXT_ALU_LD_B) ? ALU_LD_B : 0;
    result |= (value & EXT_ALU_OP_0) ? ALU_OP_0 : 0;
    result |= (value & EXT_ALU_OP_1) ? ALU_OP_1 : 0;
    result |= (value & EXT_ALU_OUT) ? ALU_OUT : 0;
    result |= (value & EXT_MEM_OUT_XDATA) ? MEM_OUT_XDATA : 0;
    result |= (value & EXT_MAR_LD_CADDR) ? MAR_LD_CADDR : 0;
    result |= (value & EXT_MBR_LD_XDATA) ? MBR_LD_XDATA : 0;
    result |= (value & EXT_MBR_OUT_CDATA) ? MBR_OUT_CDATA : 0;
    result |= (value & EXT_CDATA_TO_CADDR) ? CDATA_TO_CADDR : 0;
    result |= (value & EXT_MBR_LD_CDATA) ? MBR_LD_CDATA : 0;

    return result;
}

int mapControlLineValues(byte rom4, byte rom3, byte rom2, byte rom1)
{
    return mapControlLineValues(((unsigned long)rom4) << 24 | ((unsigned long)rom3) << 16 | ((unsigned long)rom2) << 8 | rom1);
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
    step(MEM_OUT_XDATA | MBR_LD_XDATA);

    // Transfer the MBR into IR
    step(MBR_OUT_CDATA);
    ir = cdataBus->read();

    // Restore original MBR
    cdataBus->set(savedMBR);
    step(MBR_LD_CDATA);

    // Increment the pc
    pc++;

    hexMessage("IR", ir);
    Serial.println(F("<--- P0"));
}

#define ADDR_MODE_IMMEDIATE 0
#define ADDR_MODE_ZP 1
#define ADDR_MODE_INVALID 255

const byte addrModeDecodeMapping[] PROGMEM{};

byte addrModeDecode()
{
    byte result = pgm_read_byte_near(mModeDecoder + ir);
    hexMessage("Decoded mode: ", result);
    return result;
    // if (ir == 0xA9 || ir == 0x69)
    // {
    //     return ADDR_MODE_IMMEDIATE;
    // }
    // else if (ir == 0x44 || ir == 0x65)
    // {
    //     return ADDR_MODE_ZP;
    // }
    // else
    //     return ADDR_MODE_INVALID;
}

unsigned long makeExtControlLines(byte rom4, byte rom3, byte rom2, byte rom1)
{
    Serial.print(rom4, BIN);
    Serial.print("-");
    Serial.print(rom3, BIN);
    Serial.print("-");
    Serial.print(rom2, BIN);
    Serial.print("-");
    Serial.println(rom1, BIN);

    return ((unsigned long)rom4) << 24 | ((unsigned long)rom3) << 16 | ((unsigned long)rom2) << 8 | rom1;
}

void executePhase(unsigned long doneFlag)
{
    bool done = false;

    while (!done)
    {
        hexMessage("cuaddr=", cuAddr);

        unsigned long extControlLines = makeExtControlLines(
            pgm_read_byte_near(uROM_4 + cuAddr),
            pgm_read_byte_near(uROM_3 + cuAddr),
            pgm_read_byte_near(uROM_2 + cuAddr),
            pgm_read_byte_near(uROM_1 + cuAddr));

        Serial.println(extControlLines, HEX);
        Serial.println(EXT_PC_TO_MAR, HEX);
        Serial.println(extControlLines & EXT_PC_TO_MAR);

        if ((extControlLines & EXT_PC_TO_MAR) == EXT_PC_TO_MAR)
        {
            hexMessage("Setting MAR=", pc);
            setMAR(pc);
        }
        done = extControlLines & doneFlag;
        if (extControlLines & EXT_INC_PC)
        {
            pc++;
            hexMessage("Incremented PC to ", pc);
        }
        unsigned int ctrlLines = mapControlLineValues(extControlLines);
        if (ctrlLines)
        {
            step(ctrlLines);
        }
        cuAddr++;
    }
}

void p1Addr()
{
    Serial.println(F("\nP1 --->"));
    hexMessage("PC=", pc);

    byte addrModeAddr = addrModeDecode();
    cuAddr = addrModeAddr;

    executePhase(EXT_P2);

    // byte mar;
    // switch (addrModeDecode())
    // {
    // case ADDR_MODE_IMMEDIATE:
    //     Serial.println(F("Mode: IMM"));
    //     mar = pc;
    //     setMAR(pc);
    //     pc++;
    //     break;
    // case ADDR_MODE_ZP:
    //     Serial.println(F("Mode: ZP"));
    //     mar = pc;
    //     setMAR(pc);
    //     step(MEM_OUT_XDATA | MBR_LD_XDATA);
    //     pc++;
    //     step(MBR_OUT_CDATA | CDATA_TO_CADDR | MAR_LD_CADDR);
    //     break;
    // default:
    //     Serial.println(F("Invalid addressing mode"));
    //     halt = true;
    //     break;
    // }
    // hexMessage("MAR=", mar);
    Serial.println(F("<--- P1"));
}

#define OP_CODE_LDA 0
#define OP_CODE_ADD 1
#define OP_CODE_INVALID 255

byte opDecode()
{
    byte result = pgm_read_byte_near(mOpDecoder + ir);

    hexMessage("Decoded OpCode: ", result);

    return result;

    // if (ir == 0xA9 || ir == 0x44)
    // {
    //     return OP_CODE_LDA;
    // }
    // else if (ir == 0x69 || ir == 0x65)
    // {
    //     return OP_CODE_ADD;
    // }
    // else
    //     return ADDR_MODE_INVALID;
}

void p2Op()
{
    Serial.println(F("\nP2 --->"));
    hexMessage("PC=", pc);

    byte addrOpCodeAddr = opDecode();
    cuAddr = addrOpCodeAddr;

    executePhase(EXT_P0);

    Serial.println(F("<--- P2"));

    // switch (opDecode())
    // {
    // case OP_CODE_LDA:
    //     Serial.println(F("OpCode: LDA"));
    //     step(MEM_OUT_XDATA | MBR_LD_XDATA);
    //     step(MBR_OUT_CDATA | A_LD_CDATA);
    //     break;
    // case OP_CODE_ADD:
    //     Serial.println(F("OpCode: ADD"));
    //     step(A_OUT_CDATA | ALU_LD_A);
    //     step(MEM_OUT_XDATA | MBR_LD_XDATA);
    //     step(MBR_OUT_CDATA | ALU_LD_B);
    //     step(ALU_OP_ADD | ALU_OUT | A_LD_CDATA);
    //     break;
    // default:
    //     Serial.println(F("Invalid op code"));
    //     halt = true;
    //     break;
    // }
}

void loop()
{
    // for (byte index = 0; index < 20; index++)
    // {
    //     unsigned int mapped = mapControlLineValues(pgm_read_byte_near(uROM_4 + index), pgm_read_byte_near(uROM_3 + index),
    //                                                pgm_read_byte_near(uROM_2 + index), pgm_read_byte_near(uROM_1 + index));
    //     Serial.print(index, HEX);
    //     Serial.print(" ");
    //     Serial.print(pgm_read_byte_near(uROM_4 + index), BIN);
    //     Serial.print("-");
    //     Serial.print(pgm_read_byte_near(uROM_3 + index), BIN);
    //     Serial.print("-");
    //     Serial.print(pgm_read_byte_near(uROM_2 + index), BIN);
    //     Serial.print("-");
    //     Serial.print(pgm_read_byte_near(uROM_1 + index), BIN);
    //     Serial.print(" -> ");
    //     Serial.println(mapped, BIN);
    // }

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
    // step(MBR_OUT_CDATA | A_LD_CDATA);

    // Serial.println(F("ACC -> ALUA (0x7)"));
    // step(A_OUT_CDATA | ALU_LD_A);

    // setMAR(0x01);

    // Serial.println(F("ROM -> XDATA, XDATA -> MBR (0x09)"));
    // step(MAR_OUT_XADDR | MEM_OUT_XDATA | MBR_LD_XDATA);

    // Serial.println(F("MBR -> ALUB (0x09)"));
    // step(MBR_OUT_CDATA | ALU_LD_B);

    // Serial.println(F("Add ALUA and ALUB, put the result on the bus and load it into the A register (0x10)"));
    // step(ALU_OP_ADD | ALU_OUT | A_LD_CDATA);

    // Serial.println(F("Move the result into the MAR (so we've calculated an address) (0x10)"));
    // step(A_OUT_CDATA | CDATA_TO_CADDR | MAR_LD_CADDR);

    // Serial.println(F("ROM -> XDATA, XDATA -> MBR (0x08)"));
    // step(MAR_OUT_XADDR | MEM_OUT_XDATA | MBR_LD_XDATA);

    // Serial.println(F("MBR -> ACC (0x08)"));
    // step(MBR_OUT_CDATA | A_LD_CDATA);
}