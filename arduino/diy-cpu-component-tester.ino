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

#include "Debug.h"

#define BAUD_RATE 57600

#define SLOW_MOTION_MILLIS 1000
#define MENU_INPUT_TIMEOUT_MILLIS 10000

#define ADDR_MODE_INVALID 255
#define PC_TO_MAR (PC_OUT_CADDR | MAR_LD_CADDR)

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

void step(unsigned long controlLines)
{
    controller->step(controlLines);
    delay(slowMotionMillis);
    waitForKey();
}

void setMAR(byte value)
{
    debugPrint("Setting MAR", "MAR", value);
    cdataBus->set(value);
    step(MAR_LD_CADDR | CDATA_TO_CADDR);
    cdataBus->detach();
}

void p0Fetch()
{
    debugPrintln("\nP0 --->");
    debugPrint("PC", pc);
    debugPrintln();

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
    waitForKey();

    // Restore original MBR
    cdataBus->set(savedMBR);
    step(MBR_LD_CDATA);

    // Increment the pc
    pc++;

    debugPrintln();
    debugPrint("IR", ir);
    debugPrintln("<--- P0");
}

byte addrModeDecode()
{
    byte result = pgm_read_byte_near(mModeDecoder + ir);
    return result;
}

unsigned long makeControlLines(byte rom4, byte rom3, byte rom2, byte rom1)
{
    unsigned long result = ((unsigned long)rom4) << 24 | ((unsigned long)rom3) << 16 | ((unsigned long)rom2) << 8 | rom1;

    return result;
}

void executePhase(unsigned long doneFlag)
{
    bool done = false;

    while (!done)
    {
        debugPrint("cuaddr", cuAddr);

        unsigned long controlLines = makeControlLines(
            pgm_read_byte_near(uROM_4 + cuAddr),
            pgm_read_byte_near(uROM_3 + cuAddr),
            pgm_read_byte_near(uROM_2 + cuAddr),
            pgm_read_byte_near(uROM_1 + cuAddr));

        debugPrint("Control lines", (unsigned long)controlLines, BASE_BIN);

        if ((controlLines & PC_TO_MAR) == PC_TO_MAR)
        {
            setMAR(pc);
            controlLines &= ~(MAR_LD_CADDR | CDATA_TO_CADDR);
        }
        done = controlLines & doneFlag;
        if (controlLines & PC_INC)
        {
            pc++;
            debugPrint("Incremented PC", "PC", pc);
        }

        if (controlLines)
        {
            step(controlLines);
        }

        cuAddr++;
        if (!done)
        {
            debugPrintln();
        }
    }
}

void p1Addr()
{
    debugPrintln("\nP1 --->");
    debugPrint("PC", pc);
    debugPrintln();

    byte addrModeAddr = addrModeDecode();
    cuAddr = addrModeAddr;

    executePhase(uP2);

    debugPrintln("<--- P1");
}

#define OP_CODE_INVALID 255

byte opDecode()
{
    byte result = pgm_read_byte_near(mOpDecoder + ir);

    return result;
}

void p2Op()
{
    debugPrintln("\nP2 --->");
    debugPrint("PC", pc);
    debugPrintln();

    byte addrOpCodeAddr = opDecode();
    cuAddr = addrOpCodeAddr;

    executePhase(uP0);

    debugPrintln("<--- P2");
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
}