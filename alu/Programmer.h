#ifndef _PROGRAMMER_DOT_H_
#define _PROGRAMMER_DOT_H_

#include <Arduino.h>

namespace AluTester
{
    class Programmer
    {
    private:
        void setAddress(unsigned short address);
        void outputEnable(bool enable);

    public:
        Programmer();
        void writeByte(byte data, unsigned short address);
        byte readByte(unsigned short address);
    };
}

#endif