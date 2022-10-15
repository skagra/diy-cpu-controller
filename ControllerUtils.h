#ifndef _CONTROLLER_UTILS_DOT_H_
#define _CONTROLLER_UTILS_DOT_H_

#include "Controller.h"
#include "DataBus.h"

class ControllerUtils
{
private:
    Controller *_controller;
    DataBus *_dataBus;

public:
    ControllerUtils(DataBus *dataBus, Controller *controller);
    void count(byte ldLine);
    void scan(byte ldLine);
};

#endif