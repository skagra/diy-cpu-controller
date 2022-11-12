#include <avr/pgmspace.h>

#include "Pins.h"
#include "EightBitBus.h"
#include "ControlLines.h"
#include "Controller.h"
#include "AluOps.h"
#include "Pins.h"
#include "Printer.h"

#define BAUD_RATE 57600
#define SLOW_MOTION_MILLIS 500

EightBitBus *cdataBus = new EightBitBus(DATA_BUS_PIN_LOW);
ControlLines *controlLines = new ControlLines();
Controller *controller = new Controller(controlLines, cdataBus);

bool slowMotion = false;
bool verbose = true;

void setup()
{
    Serial.begin(BAUD_RATE);

    while (!Serial)
        delay(100);

    Printer::SetVerbosity(Printer::Verbosity::all);

    Printer::Println("DIY CPU Controller/Monitor");
    Printer::Println();
    help();
}

void help()
{
    Printer::Println("h => Print this help message.");
    Printer::Println("v => Toggle verbose mode.");
    Printer::Println("z => Toggle slow motion mode.");
    Printer::Println("x => Reset.");
    Printer::Println("r => Run.");
    Printer::Println("c => Continue.");
    Printer::Println("s => Single step.");
    Printer::Println("t => Test mode (continuous run)");
    Printer::Println();
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
            Printer::Print("Verbose mode ");
            Printer::Println(verbose ? "on" : "off");
            if (verbose)
            {
                Printer::SetVerbosity(Printer::Verbosity::all);
            }
            else
            {
                Printer::SetVerbosity(Printer::Verbosity::minimal);
            }
            break;
        case 'z':
            slowMotion = !slowMotion;
            Printer::Print("Slow motion mode ");
            Printer::Println(slowMotion ? "on" : "off");
            break;
        case 'r':
            Printer::Println("Run");
            run();
            break;
        case 'c':
            Printer::Println("Continue");
            go();
            break;
        case 'x':
            Printer::Println("Reset");
            controller->reset();
            break;
        case 's':
            controller->uStep(programComplete, error);
            if (error)
            {
                reportError();
            }
            break;
        case 't':
            Printer::Println("Continuous test");
            test();
            break;
        default:
            Printer::Println("Invalid option");
            Printer::Println();
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
    Printer::Println();
    Printer::Println("===============");
    Printer::Println("==== ERROR ====");
    Printer::Print("IR", controller->getIR());
    Printer::Print("PC", controller->getPC());
    Printer::Print("cuaddr", controller->getCUAddr());
    Printer::Println("===============");
    Printer::Println();
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

    Printer::Println("Run complete");
    Printer::Println();

    if (error)
    {
        reportError();
    }
}

void test()
{
    unsigned long iteration = 0;
    bool done = false;
    bool error = false;

    while (!error)
    {
        done = false;
        controller->reset();
        while (!done && !error)
        {
            controller->uStep(done, error);
        }
        iteration++;
        if (!verbose)
        {
            Serial.print(".");
            if (iteration % 100 == 0)
            {
                Serial.println();
                Serial.println(iteration);
            }
        }
    }
    reportError();
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
}