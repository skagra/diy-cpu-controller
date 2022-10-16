#ifndef _CONTROL_LINES_DOT_H_
#define _CONTROL_LINES_DOT_H_

#include <Arduino.h>

#define REGISTER_LD 0b00000001  // 10
#define REGISTER_OUT 0b00000010 // 11
#define ALU_LD_A 0b00000100     // 14
#define ALU_LD_B 0b00001000     // 15
#define ALU_OP_0 0b00010000     // 16
#define ALU_OP_1 0b00100000     // 17
#define ALU_OUT 0b01000000      // 18

class ControlLines
{
private:
    const int controlBits[7] = {10, 11, 14, 15, 16, 17, 18};
    int controlBitToPin(byte bitNumber);

public:
    ControlLines();
    void reset();
    void set(byte lines);
};

#endif