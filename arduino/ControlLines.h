#ifndef _CONTROL_LINES_DOT_H_
#define _CONTROL_LINES_DOT_H_

#include <Arduino.h>

// Shift 0 - ROM 0
#define CDATA_LD_0 0b00000000000000000000000000000001UL     // QA
#define CDATA_TO_CADDR 0b00000000000000000000000000000010UL // QB
#define MEM_LD_XDATA 0b00000000000000000000000000000100UL   // QC
#define MEM_OUT_XDATA 0b00000000000000000000000000001000UL  // QD
#define IR_LD_XDATA 0b00000000000000000000000000010000UL    // QE
#define MBR_LD_XDATA 0b00000000000000000000000000100000UL   // QF
#define MBR_OUT_XDATA 0b00000000000000000000000001000000UL  // QG
#define MBR_LD_CDATA 0b00000000000000000000000010000000UL   // QH

// Shift 1 - ROM 1
#define MBR_OUT_CDATA 0b00000000000000000000000100000000UL // QA
#define MAR_LD_CADDR 0b00000000000000000000001000000000UL  // QB
#define PC_INC 0b00000000000000000000010000000000UL        // QC
#define PC_REL_CDATA 0b00000000000000000000100000000000UL  // QD
#define PC_LD_CDATA 0b00000000000000000001000000000000UL   // QE
#define PC_OUT_CDATA 0b00000000000000000010000000000000UL  // QF // Needed ? NO it is only used for stack operations TODO REMOVE FOR NOW
#define PC_OUT_CADDR 0b00000000000000000100000000000000UL  // QG
#define A_LD_CDATA 0b00000000000000001000000000000000UL    // QH

// Shift 2 - ROM 2
#define A_OUT_CDATA 0b00000000000000010000000000000000UL    // QA
#define X_LD_CDATA 0b00000000000000100000000000000000UL     // QB
#define X_OUT_CDATA 0b00000000000001000000000000000000UL    // QC
#define PZ_LD 0b00000000000010000000000000000000UL          // QD
#define ALUA_LD_CDATA 0b00000000000100000000000000000000UL  // QE
#define ALUB_LD_CDATA 0b00000000001000000000000000000000UL  // QF
#define ALUR_OUT_CDATA 0b00000000010000000000000000000000UL // QG
#define ALUOP_0 0b00000000100000000000000000000000UL        // QH

// Shift 3 - ROM 3
#define ALUOP_1 0b00000001000000000000000000000000UL     // QA
#define Z_SRC_CDATA 0b00000010000000000000000000000000UL // QB
#define uJMP 0b00000100000000000000000000000000UL        // QC
#define uJMPINV 0b00001000000000000000000000000000UL     // QD
#define uP0 0b00010000000000000000000000000000UL         // QE
#define uP1 0b00100000000000000000000000000000UL         // QF
#define uP2 0b01000000000000000000000000000000UL         // QG
#define uZJMP 0b10000000000000000000000000000000UL       // QH

class ControlLines
{
private:
    static const char *decoder[];

public:
    ControlLines();
    void reset();
    void set(unsigned long lines);
    void decode(unsigned long lines);
};

#endif