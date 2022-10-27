#include "ControlLines.h"
#include "Pins.h"

ControlLines::ControlLines()
{
    pinMode(SHIFT_DATA_PIN, OUTPUT);
    pinMode(SHIFT_CLK_PIN, OUTPUT);
    pinMode(SHIFT_LATCH_PIN, OUTPUT);

    reset();
}

void ControlLines::set(unsigned int lines)
{
    decode(lines);

    digitalWrite(SHIFT_LATCH_PIN, LOW);

    shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, (byte)lines);
    shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, (byte)(lines >> 8));

    // HACK
    shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, (byte)lines);
    shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, (byte)(lines >> 8));

    digitalWrite(SHIFT_LATCH_PIN, HIGH);
}

void ControlLines::reset()
{
    set(0);
}

void ControlLines::decode(unsigned int match, unsigned int lines, const char *value)
{
    if (match & lines)
    {
        Serial.print(value);
        Serial.print(" ");
    }
}

void ControlLines::decode(unsigned int lines)
{
    Serial.print("Decoded: ");
    decode(A_LD_CDATA, lines, "A_LD_CDATA");
    decode(A_OUT_CDATA, lines, "A_OUT_CDATA");
    decode(ALU_LD_A, lines, "ALU_LD_A");
    decode(ALU_LD_B, lines, "ALU_LD_B");
    decode(ALU_OP_0, lines, "ALU_OP_0");
    decode(ALU_OP_1, lines, "ALU_OP_1");
    decode(ALU_OUT, lines, "ALU_OUT");
    decode(MEM_OUT_XDATA, lines, "MEM_OUT_XDATA");
    decode(MAR_LD_CADDR, lines, "MAR_LD_CADDR");
    decode(MBR_LD_XDATA, lines, "MBR_LD_XDATA");
    decode(MBR_OUT_CDATA, lines, "MBR_OUT_CDATA");
    decode(CDATA_TO_CADDR, lines, "CDATA_TO_CADDR");
    decode(MBR_LD_CDATA, lines, "MBR_LD_CDATA");
    Serial.println();
}