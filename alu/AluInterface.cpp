// #include "AluInterface.h"

// #define CLOCK_PULSE_DELAY_MICROS 1000

// using namespace AluTester;

// AluInterface::AluInterface()
// {
//     pinMode(SHIFT_DATA_PIN, OUTPUT);
//     pinMode(SHIFT_CLK_PIN, OUTPUT);
//     pinMode(SHIFT_LATCH_PIN, OUTPUT);

//     pinMode(CLOCK_PIN, OUTPUT);

//     for (int pin = READ_D0_PIN; pin <= READ_D7_PIN; pin++)
//     {
//         pinMode(pin, INPUT);
//     }
// }

// void AluInterface::writeBytes(byte value2, byte value1, byte value0)
// {
//     // Write the address to the shift registers
//     shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, value0);
//     shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, value1);
//     shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, value2);

//     // Pulse to latch the shift registers
//     digitalWrite(SHIFT_LATCH_PIN, LOW);
//     digitalWrite(SHIFT_LATCH_PIN, HIGH);
//     digitalWrite(SHIFT_LATCH_PIN, LOW);
// }

// byte AluInterface::readByte()
// {
//     byte data = 0;
//     for (int pin = READ_D7_PIN; pin >= READ_D0_PIN; pin--)
//     {
//         data = (data << 1) + digitalRead(pin);
//     }

//     return data;
// }

// void AluInterface::pulseClock()
// {
//     digitalWrite(CLOCK_PIN, HIGH);
//     delayMicroseconds(CLOCK_PULSE_DELAY_MICROS);
//     digitalWrite(CLOCK_PIN, LOW);
// }