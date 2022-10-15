#include "ControllerUtils.h"

ControllerUtils::ControllerUtils(DataBus *dataBus, Controller *controller)
{
    _dataBus = dataBus;
    _controller = controller;
}

void ControllerUtils::count(byte ldLine)
{
    for (int value = 0; value < 256; value++)
    {
        _dataBus->setDataBus(value);
        _controller->step(ldLine, 1000);
        _dataBus->detachDataBus();
    }
}

void ControllerUtils::scan(byte ldLine)
{

    for (int loop = 0; loop < 1; loop++)
    {
        for (int bit = 0; bit < 8; bit++)
        {
            _dataBus->setDataBus(1 << bit);
            _controller->step(ldLine, (unsigned int)64000);
            _dataBus->detachDataBus();
            delay(100);
        }

        for (int bit = 7; bit >= 0; bit--)
        {
            _dataBus->setDataBus(1 << bit);
            _controller->step(ldLine, (unsigned int)64000);
            _dataBus->detachDataBus();
            delay(100);
        }
    }
}