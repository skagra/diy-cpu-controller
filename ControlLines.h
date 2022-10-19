#ifndef _CONTROL_LINES_DOT_H_
#define _CONTROL_LINES_DOT_H_

#include <Arduino.h>

// SHIFT-3 SHIFT-2 SHIFT-1 SHIFT-0

#define REGISTER_A_LD 0b0000000000000001U  // SHIFT-0 QA
#define REGISTER_A_OUT 0b0000000000000010U // SHIFT-0 QB
#define ALU_LD_A 0b0000000000000100U       // SHIFT-0 QC
#define ALU_LD_B 0b0000000000001000U       // SHIFT-0 QD
#define ALU_OP_0 0b0000000000010000U       // SHIFT-0 QE
#define ALU_OP_1 0b0000000000100000U       // SHIFT-0 QF
#define ALU_OUT 0b0000000001000000U        // SHIFT-0 QG
#define REGISTER_X_LD 0b0000000010000000U  // SHIFT-0 QH

#define REGISTER_X_OUT 0b0000000100000000U // SHIFT-1 QA

class ControlLines
{
public:
    ControlLines();
    void reset();
    void set(unsigned int lines);
};

#endif