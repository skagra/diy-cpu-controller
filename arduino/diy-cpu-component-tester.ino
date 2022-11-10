#include <avr/pgmspace.h>

#include "Pins.h"
#include "EightBitBus.h"
#include "ControlLines.h"
#include "Controller.h"
#include "AluOps.h"
#include "Pins.h"

#include "Debug.h"

#define BAUD_RATE 57600

#define SLOW_MOTION_MILLIS 1000
#define MENU_INPUT_TIMEOUT_MILLIS 10000

#define ADDR_MODE_INVALID 255

unsigned int slowMotionMillis = 0;
bool waitForKeyPress = false;
unsigned long iterations = 0;
bool testMode = false;

EightBitBus *cdataBus = new EightBitBus(DATA_BUS_PIN_LOW);
ControlLines *controlLines = new ControlLines();
Controller *controller = new Controller(controlLines, cdataBus, step);

void step()
{
    delay(slowMotionMillis);
    waitForKey();
}

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

void loop()
{
    controller->pInit();

    if (!testMode)
    {
        menu();
        waitForKey();
    }

    controller->run();

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