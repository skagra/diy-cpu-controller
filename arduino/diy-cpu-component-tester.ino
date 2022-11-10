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
unsigned long iterations = 0;
bool testMode = false;

EightBitBus *cdataBus = new EightBitBus(DATA_BUS_PIN_LOW);
ControlLines *controlLines = new ControlLines();
Controller *controller = new Controller(controlLines);
ControllerUtils *controllerUtils = new ControllerUtils(cdataBus, controller);

byte pc = 0;
byte ir = 0;
byte cuaddr = 0;

void setup()
{
    Serial.begin(BAUD_RATE);

    while (!Serial)
        delay(100);
}

void menu()
{
    Serial.setTimeout(MENU_INPUT_TIMEOUT_MILLIS);

    bool gotOption = false;
    while (!gotOption)
    {
        Serial.println(F("Select mode: r (run), z (slow motion), s (single step), t (test)"));
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
        case 't':
            Serial.println(F("Test mode."));
            slowMotionMillis = 0;
            waitForKeyPress = false;
            testMode = true;
            iterations = 0;
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

bool p0Fetch()
{
    bool error = false;

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
    error = ir == 0xFF;

    if (!error)
    {
        waitForKey();

        // Restore original MBR
        cdataBus->set(savedMBR);
        step(MBR_LD_CDATA);
        cdataBus->detach();

        // Increment the pc
        pc++;

        debugPrintln();
        debugPrint("IR", ir);
        debugPrintln("<--- P0");
    }

    return error;
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

bool executePhase(unsigned long doneFlag)
{
    bool error = false;
    bool done = false;

    while (!done && !error)
    {
        debugPrint("cuaddr", cuaddr);
        error = cuaddr == 0xFF;

        if (!error)
        {
            unsigned long controlLines = makeControlLines(
                pgm_read_byte_near(uROM_4 + cuaddr),
                pgm_read_byte_near(uROM_3 + cuaddr),
                pgm_read_byte_near(uROM_2 + cuaddr),
                pgm_read_byte_near(uROM_1 + cuaddr));

            debugPrint("Control lines", (unsigned long)controlLines, BASE_BIN, true);

            error = controlLines == 0xFF;

            if (!error)
            {
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

                cuaddr++;
                if (!done)
                {
                    debugPrintln();
                }
            }
        }
    }

    return error;
}

bool pInit()
{
    bool error = false;

    debugPrintln("\nPInit --->");
    debugPrint("PC", pc);
    debugPrintln();

    cuaddr = 0;
    error = executePhase(uP0);

    debugPrintln("<--- P1");

    return error;
}

bool p1Addr()
{
    bool error = false;

    debugPrintln("\nP1 --->");
    debugPrint("PC", pc);
    debugPrintln();

    byte addrModeAddr = addrModeDecode();
    cuaddr = addrModeAddr;

    error = executePhase(uP2);

    debugPrintln("<--- P1");

    return error;
}

byte opDecode()
{
    byte result = pgm_read_byte_near(mOpDecoder + ir);

    return result;
}

bool p2Op()
{
    bool error = false;

    debugPrintln("\nP2 --->");
    debugPrint("PC", pc);
    debugPrintln();

    byte addrOpCodeAddr = opDecode();
    cuaddr = addrOpCodeAddr;

    error = executePhase(uP0);

    debugPrintln("<--- P2");

    return error;
}

void loop()
{
    pInit();

    if (!testMode)
    {
        menu();
        waitForKey();
    }

    pc = 0;
    bool done = false;
    bool error = false;

    do
    {
        error = p0Fetch();
        done = ir == 0;

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
        debugPrint("IR", ir, BASE_HEX, false);
        debugPrint(", PC", pc, BASE_HEX, false);
        debugPrint(", cuaddr", cuaddr, BASE_HEX, true);
        Serial.print("Iterations="); // TODO
        Serial.println(iterations);
        Serial.println("===============");
        Serial.println();

        while (true)
        {
            delay(1000);
        }
    }

    iterations++;

    if (!testMode)
    {
        Serial.println("Press key to reset");

        while (Serial.read() == -1)
        {
            delay(100);
        }
    }
    waitForKeyPress = false;
}