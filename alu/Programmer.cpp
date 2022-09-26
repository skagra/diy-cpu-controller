#include "Programmer.h"

using namespace AluTester;

// EEPROM pin numbers for AT28C64
// Change for different sizes of EEPROM
#define SHIFT_DATA_PIN 2  // -> Shift register 1 SER pin (14)
#define SHIFT_CLK_PIN 3   // -> Shift registers SRCLCK pins (11)
#define SHIFT_LATCH_PIN 4 // -> Shift registers RCLC pins (12)
#define EEPROM_D0_PIN 5   // -> EEPROM LSB data pin
#define EEPROM_D7_PIN 12  // -> EEPROM MSB data pin
#define EEPROM_WE_PIN 13  // -> EEPROM ~WE pin (27)
#define EEPROM_OE_PIN 14  // -> EEPROM ~OE pin (22)

Programmer::Programmer()
{
    pinMode(SHIFT_DATA_PIN, OUTPUT);
    pinMode(SHIFT_CLK_PIN, OUTPUT);
    pinMode(SHIFT_LATCH_PIN, OUTPUT);
    digitalWrite(EEPROM_WE_PIN, HIGH);
    pinMode(EEPROM_WE_PIN, OUTPUT);
    pinMode(EEPROM_OE_PIN, OUTPUT);
}

void Programmer::setAddress(unsigned short address)
{
    // Write the address to the shift registers
    shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, (address >> 8));
    shiftOut(SHIFT_DATA_PIN, SHIFT_CLK_PIN, MSBFIRST, address);

    // Pulse to latch the shift registers
    digitalWrite(SHIFT_LATCH_PIN, LOW);
    digitalWrite(SHIFT_LATCH_PIN, HIGH);
    digitalWrite(SHIFT_LATCH_PIN, LOW);
}

void Programmer::outputEnable(bool enable)
{
    if (enable)
    {
        digitalWrite(EEPROM_OE_PIN, LOW);
    }
    else
    {
        digitalWrite(EEPROM_OE_PIN, HIGH);
    }
}

void Programmer::writeByte(byte data, unsigned short address)
{
    // Set EEPROM address to write to
    setAddress(address);

    // We are writing
    outputEnable(false);

    // Set data pins to output
    for (int pin = EEPROM_D0_PIN; pin <= EEPROM_D7_PIN; pin++)
    {
        pinMode(pin, OUTPUT);
    }

    // To poll for completion of the write operation
    byte msb = data & 0x80;

    // Set the data to write to the EEPROM
    for (int pin = EEPROM_D0_PIN; pin <= EEPROM_D7_PIN; pin++)
    {
        digitalWrite(pin, data & 1);
        data >>= 1;
    }

    // Trigger write
    digitalWrite(EEPROM_WE_PIN, LOW);
    delayMicroseconds(1);
    digitalWrite(EEPROM_WE_PIN, HIGH);

    // A completed write is flagged as complete when
    // the MS-bit read back is equal to the value written.
    do
    {
        delay(1);
    } while ((readByte(address) & 0x80) != msb);
}

byte Programmer::readByte(unsigned short address)
{
    // Set EEPROM address to write to
    setAddress(address);

    // We are reading
    outputEnable(true);

    for (int pin = EEPROM_D0_PIN; pin <= EEPROM_D7_PIN; pin++)
    {
        pinMode(pin, INPUT);
    }

    // Read the byte from the EEPROM
    byte data = 0;
    for (int pin = EEPROM_D7_PIN; pin >= EEPROM_D0_PIN; pin--)
    {
        data = (data << 1) + digitalRead(pin);
    }

    return data;
}
