#ifndef _CONTROL_LINES_DOT_H_
#define _CONTROL_LINES_DOT_H_

#include <Arduino.h>

#define REGISTER_LD 0b0000000000000001U  // ALU0 QA
#define REGISTER_OUT 0b0000000000000010U // ALU0 QB
#define ALU_LD_A 0b0000000000000100U     // ALU0 QC
#define ALU_LD_B 0b0000000000001000U     // ALU0 QD
#define ALU_OP_0 0b0000000000010000U     // ALU0 QE
#define ALU_OP_1 0b0000000000100000U     // ALU0 QF
#define ALU_OUT 0b0000000001000000U      // ALU0 QG

class ControlLines
{
public:
    ControlLines();
    void reset();
    void set(unsigned int lines);
};

#endif