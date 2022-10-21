#ifndef _CONTROLLER_UTILS_DOT_H_
#define _CONTROLLER_UTILS_DOT_H_

#include "Controller.h"
#include "EightBitBus.h"

class ControllerUtils
{
private:
    Controller *_controller;
    EightBitBus *_dataBus;

public:
    ControllerUtils(EightBitBus *dataBus, Controller *controller);
    void count(byte ldLine);
    void scan(byte ldLine);
};

#endif