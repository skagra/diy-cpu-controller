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

    Serial.setTimeout(-1);

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
    Printer::Println("b [XX] => Set/clear breakpoint.");
    Printer::Println();
}

void Beep()
{
    Serial.print((char)7);
}

int readString(char *buffer, int bufferSize)
{
    bool done = false;
    char input;
    int bufferIndex = 0;

    while (!done)
    {
        while (Serial.available() == 0)
            delay(100);
        input = Serial.read();

        if (input == 8)
        {
            if (bufferIndex > 0)
            {
                Serial.print((char)8);
                Serial.print(" ");
                Serial.print((char)8);
                Serial.print((char)127);
                bufferIndex--;
            }
            else
            {
                Beep();
            }
        }
        else
        {
            if (bufferIndex < bufferSize)
            {
                buffer[bufferIndex++] = input;
                Serial.print(input);
            }
            else
            {
                Beep();
            }
        }
        done = input == '\r';
    }
    buffer[bufferIndex - 1] = 0;

    return bufferIndex;
}

#define INPUT_BUFFER_SIZE 20

char *inputBuffer = (char *)malloc(INPUT_BUFFER_SIZE * sizeof(char));

bool charToNibble(char hexChar, byte &result)
{
    byte ok = true;

    if (hexChar >= '0' && hexChar <= '9')
    {
        result = hexChar - '0';
    }
    else if (hexChar >= 'a' && hexChar <= 'f')
    {
        result = hexChar - 'a' + 10;
    }
    else if (hexChar >= 'A' && hexChar <= 'F')
    {
        result = hexChar - 'A' + 10;
    }
    else
    {
        result = 0;
        ok = false;
    }

    return ok;
}

bool parseHex(char char1, char char0, byte &result)
{
    byte highNibble;
    byte lowNibble;

    bool ok = charToNibble(char1, highNibble);
    if (ok)
    {
        ok = charToNibble(char0, lowNibble);
        if (ok)
        {
            result = highNibble << 4 | lowNibble;
        }
    }

    return ok;
}

bool readHex(char *buffer, int bufferLength, byte &result)
{
    int index = 0;
    bool ok = false;

    // Skip past spaces
    bool done = false;
    while (index < bufferLength && !done)
    {
        done = buffer[index] != ' ';
        if (!done)
        {
            index++;
        }
    }

    if (done && index < bufferLength - 2)
    {
        ok = parseHex(buffer[index], buffer[index + 1], result);
    }

    return ok;
}

void execution()
{
    bool programComplete;
    bool error;
    byte byteRead;
    bool mcBreakpointHit;

    while (true)
    {
        Serial.print("> ");
        int charsRead = readString(inputBuffer, INPUT_BUFFER_SIZE);

        if (charsRead > 1)
        {
            char option = inputBuffer[0];

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
                controller->uStep(programComplete, mcBreakpointHit, error);
                if (error)
                {
                    reportError();
                }
                break;
            case 'b':
                if (charsRead == 2)
                {
                    controller->clearMCBreakpoint();
                    Printer::Println("Breakpoint cleared");
                }
                else
                {
                    if (readHex(inputBuffer + 1, charsRead - 1, byteRead))
                    {
                        controller->setMCBreakpoint(byteRead);
                        Printer::Print("Breakpoint set", byteRead);
                    }
                    else
                    {
                        Beep();
                        Serial.println();
                    }
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
        else
        {
            Beep();
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
    bool mcBreak = false;

    while (!done && !error && !mcBreak)
    {
        controller->uStep(done, mcBreak, error);
        if (slowMotion)
        {
            delay(SLOW_MOTION_MILLIS);
        }
    }

    if (error)
    {
        reportError();
    }
    else if (mcBreak)
    {
        Printer::Println();
        Printer::Println("Stopped at breakpoint");
    }
    else
    {
        Printer::Println("Run complete");
        Printer::Println();
    }
}

void test()
{
    unsigned long iteration = 0;
    bool done = false;
    bool error = false;
    bool mcBreakpointHit = false;

    while (!error)
    {
        done = false;
        controller->reset();
        while (!done && !error)
        {
            controller->uStep(done, mcBreakpointHit, error);
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