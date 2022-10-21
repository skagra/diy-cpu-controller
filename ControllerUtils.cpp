#include "ControllerUtils.h"

#define COUNT_DELAY_MILLIS 20
#define SCAN_DELAY_MILLIS 20

ControllerUtils::ControllerUtils(EightBitBus *dataBus, Controller *controller)
{
    _dataBus = dataBus;
    _controller = controller;
}

void ControllerUtils::count(byte ldLine)
{
    for (int value = 0; value < 256; value++)
    {
        _dataBus->set(value);
        _controller->step(ldLine);
        _dataBus->detach();
        delay(COUNT_DELAY_MILLIS);
    }
}

void ControllerUtils::scan(byte ldLine)
{

    for (int loop = 0; loop < 5; loop++)
    {
        for (int bit = 0; bit < 8; bit++)
        {
            _dataBus->set(1 << bit);
            _controller->step(ldLine);
            _dataBus->detach();
            delay(SCAN_DELAY_MILLIS);
        }

        for (int bit = 7; bit >= 0; bit--)
        {
            _dataBus->set(1 << bit);
            _controller->step(ldLine);
            _dataBus->detach();
            delay(SCAN_DELAY_MILLIS);
        }
    }
}