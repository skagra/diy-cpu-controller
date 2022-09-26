#ifndef _ALU_INTERFACE_DOT_H_
#define _ALU_INTERFACE_DOT_H_

#include <Arduino.h>
#include "Pins.h"

namespace AluTester
{
    class AluInterface
    {
    public:
        AluInterface();
        void pulseClock();
        void writeBytes(byte value2, byte value1, byte value0);
        byte readByte();
    };
}

#endif