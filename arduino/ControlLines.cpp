#include "ControlLines.h"
#include "Pins.h"

#include "Debug.h"

ControlLines::ControlLines()
{
    pinMode(SHIFT_DATA_PIN, OUTPUT);
    pinMode(SHIFT_CLK_PIN, OUTPUT);
    pinMode(SHIFT_LATCH_PIN, OUTPUT);

    reset();
}

void ControlLines::set(unsigned long lines)
{
    decode(lines);

    digitalWrite(SHIFT_LATCH_PIN, LOW);

    shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, (byte)lines);
    shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, (byte)(lines >> 8));
    shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, (byte)(lines >> 16));
    shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, (byte)(lines >> 24));

    digitalWrite(SHIFT_LATCH_PIN, HIGH);
}

void ControlLines::reset()
{
    set(0);
}

const char *decoder[] = {
    "CDATA_LD_0",
    "CDATA_TO_CADDR",
    "MEM_LD_XDATA",
    "MEM_OUT_XDATA",
    "IR_LD_XDATA",
    "MBR_LD_XDATA",
    "MBR_OUT_XDATA",
    "MBR_LD_CDATA",
    "MBR_OUT_CDATA",
    "MAR_LD_CADDR",
    "PC_INC",
    "PC_REL_CDATA",
    "PC_LD_CDATA",
    "PC_OUT_CDATA",
    "PC_OUT_CADDR",
    "A_LD_CDATA",
    "A_OUT_CDATA",
    "X_LD_CDATA",
    "X_OUT_CDATA",
    "PZ_LD",
    "ALUA_LD_CDATA",
    "ALUB_LD_CDATA",
    "ALUR_OUT_CDATA",
    "ALUOP_0",
    "ALUOP_1",
    "Z_SRC_CDATA",
    "uJMP",
    "uJMPINV",
    "uP0",
    "uP1",
    "uP2",
    "uZJMP"};

void ControlLines::decode(unsigned long lines)
{
    bool printing = false;
    debugPrint("Decoded: ");
    for (int bit = 0; bit < 32; bit++)
    {
        if ((lines >> bit) & 1UL)
        {
            if (printing)
            {
                debugPrint(" | ");
            }
            debugPrint(decoder[bit]);
            printing = true;
        }
    }
    debugPrintln();
}