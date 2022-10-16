#ifndef _DATA_BUS_DOT_H_
#define _DATA_BUS_DOT_H_

#include <Arduino.h>

class DataBus
{
public:
    void set(byte value);
    void detach();
    byte read();
};

#endif