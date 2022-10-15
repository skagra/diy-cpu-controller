#ifndef _DATA_BUS_DOT_H_
#define _DATA_BUS_DOT_H_

#include <Arduino.h>

class DataBus
{
public:
    void setDataBus(byte value);
    void detachDataBus();
    byte readDataBus();
};

#endif