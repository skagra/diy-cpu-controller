#include <avr/pgmspace.h>

#include "Pins.h"
#include "EightBitBus.h"
#include "ControlLines.h"
#include "Controller.h"
#include "AluOps.h"
#include "Pins.h"

#include "Debug.h"

#define BAUD_RATE 57600
#define SLOW_MOTION_MILLIS 500

EightBitBus *cdataBus = new EightBitBus(DATA_BUS_PIN_LOW);
ControlLines *controlLines = new ControlLines();
Controller *controller = new Controller(controlLines, cdataBus);

void setup()
{
    Serial.begin(BAUD_RATE);

    while (!Serial)
        delay(100);

    Serial.println("DIY CPU Controller/Monitor");
    Serial.println();
    help();
}

bool slowMotion = false;
bool verbose = false;

void help()
{
    Serial.println("h => Print this help message.");
    Serial.println("v => Toggle verbose mode.");
    Serial.println("z => Toggle slow motion mode.");
    Serial.println("x => Reset.");
    Serial.println("r => Run.");
    Serial.println("c => Continue.");
    Serial.println("s => Single step.");
    Serial.println();
}

void execution()
{
    bool programComplete;
    bool error;

    while (true)
    {
        while (Serial.available() == 0)
            delay(100);

        char option = Serial.read();

        switch (option)
        {
        case 'h':
            help();
            break;
        case 'v':
            verbose = !verbose;
            Serial.print("Verbose mode ");
            Serial.println(verbose ? "on" : "off");
            break;
        case 'z':
            slowMotion = !slowMotion;
            Serial.print("Slow motion mode ");
            Serial.println(slowMotion ? "on" : "off");
            break;
        case 'r':
            Serial.println("Run");
            run();
            break;
        case 'c':
            Serial.println("Continue");
            go();
            break;
        case 'x':
            Serial.println("Reset");
            controller->reset();
            break;
        case 's':
            controller->uStep(programComplete, error);
            if (error)
            {
                reportError();
            }
            break;
        default:
            Serial.println("Invalid option");
            Serial.println();
            help();
        }
    }
}

void run()
{
    controller->reset();
    go();
}

void reportError()
{
    Serial.println();
    Serial.println("===============");
    Serial.println("==== ERROR ====");
    debugPrint("IR", controller->getIR());
    debugPrint("PC", controller->getPC());
    debugPrint("cuaddr", controller->getCUAddr());
    Serial.println("===============");
    Serial.println();
}

void go()
{
    bool done = false;
    bool error = false;

    while (!done && !error)
    {
        controller->uStep(done, error);
        if (slowMotion)
        {
            delay(SLOW_MOTION_MILLIS);
        }
    }

    Serial.println("Run complete");
    Serial.println();

    if (error)
    {
        reportError();
    }
}

void waitForKey()
{
    while (Serial.read() == -1)
    {
        delay(100);
    }
}

void loop()
{
    execution();
    waitForKey();
}