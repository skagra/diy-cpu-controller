#include "AluInterface.h"
#include "Pins.h"

using namespace AluTester;

#define BAUD_RATE 57600

#define OP_ADD_0 0
#define OP_ADD_1 0

#define OP_SUB_0 1
#define OP_SUB_1 0

#define OP_INC_0 0
#define OP_INC_1 1

#define OP_DEC_0 1
#define OP_DEC_1 1

#define ALU_DELAY_MICRO 1000

AluInterface aluInterface;

byte makeControlByte(bool op1, bool op0, bool triOut, bool lda, bool ldb, bool cIn)
{
    return (byte)(op1 << 5 | op0 << 4 | triOut << 3 | lda << 2 | ldb << 1 | cIn);
}

// TODO: C out and Z out
byte executeAddTest(byte a, byte b)
{
    aluInterface.writeBytes(makeControlByte(OP_ADD_1, OP_ADD_0, true, true, true, false), a, b);
    aluInterface.pulseClock();
    delayMicroseconds(ALU_DELAY_MICRO);
    return aluInterface.readByte();
}

// TODO: Add C, Z out tests and multibyte/cin tests
void runAddTests()
{
    Serial.println("Starting Add Tests");

    for (byte aluA = 0; aluA < 255; aluA++)
    {
        for (byte aluB = 0; aluB < 255; aluB++)
        {
            byte result = executeAddTest(aluA, aluB);
            byte expected = aluA + aluB;
            if (aluA == aluB)
            {
                Serial.print(".");
            }
            else
            {
                Serial.println();
                Serial.print("Error: Evaluated ");
                Serial.print(aluA);
                Serial.print("+");
                Serial.print(aluB);
                Serial.print(" expected: ");
                Serial.print(expected);
                Serial.print(" got:");
                Serial.print(result);
                Serial.println();
            }
        }
    }

    Serial.print("Done Add Tests");
}

void setup()
{
    Serial.begin(BAUD_RATE);
    runAddTests();
}

void loop()
{
}