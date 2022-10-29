#include "Debug.h"

void debugPrint(const char *message)
{
#ifdef DEBUG
    Serial.print(message);
#endif
}

void debugPrintln(const char *message)
{
#ifdef DEBUG
    Serial.println(message);
#endif
}

void debugPrintln()
{
#ifdef DEBUG
    Serial.println();
#endif
}

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)       \
    (byte & 0x80 ? '1' : '0'),     \
        (byte & 0x40 ? '1' : '0'), \
        (byte & 0x20 ? '1' : '0'), \
        (byte & 0x10 ? '1' : '0'), \
        (byte & 0x08 ? '1' : '0'), \
        (byte & 0x04 ? '1' : '0'), \
        (byte & 0x02 ? '1' : '0'), \
        (byte & 0x01 ? '1' : '0')

char *buffer = new char[9];
void debugPrintBin(byte value, bool ms)
{
    if (ms)
    {
        debugPrint("0b");
    }
    sprintf(buffer, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(value));
    debugPrint(buffer);
}

void debugPrintBin(unsigned int value)
{
    debugPrintBin((byte)(value >> 8));
    debugPrint("-");
    debugPrintBin((byte)value, false);
}

void debugPrintBin(unsigned long value)
{
    debugPrintBin((byte)(value >> 24));
    debugPrint("-");
    debugPrintBin((byte)(value >> 16), false);
    debugPrint("-");
    debugPrintBin((byte)(value >> 8), false);
    debugPrint("-");
    debugPrintBin((byte)value, false);
}

void debugPrintHex(byte value, bool ms)
{
    if (ms)
    {
        debugPrint("0x");
    }
    sprintf(buffer, "%.2X", value);
    debugPrint(buffer);
}

void debugPrintHex(unsigned int value)
{
    debugPrintHex((byte)(value >> 8));
    debugPrint("-");
    debugPrintHex((byte)value, false);
}

void debugPrintHex(unsigned long value)
{
    debugPrintHex((byte)(value >> 24));
    debugPrint("-");
    debugPrintHex((byte)(value >> 16), false);
    debugPrint("-");
    debugPrintHex((byte)(value >> 8), false);
    debugPrint("-");
    debugPrintHex((byte)value, false);
}

enum DebugValueType
{
    TYPE_BYTE,
    TYPE_INT,
    TYPE_LONG
};

void debugPrint(const char *message, const char *name, byte byteValue, unsigned int intValue,
                unsigned long longValue, DebugValueType valueType, Base base, bool newline)
{
    if (message != (char *)0)
    {
        debugPrint(message);
        debugPrint(", ");
    }

    debugPrint(name);
    debugPrint("=");

    switch (valueType)
    {
    case TYPE_BYTE:
        if (base == BASE_BIN)
        {
            debugPrintBin(byteValue);
        }
        else
        {
            debugPrintHex(byteValue);
        }
        break;
    case TYPE_INT:
        if (base == BASE_BIN)
        {
            debugPrintBin(intValue);
        }
        else
        {
            debugPrintHex(intValue);
        }
        break;
    case TYPE_LONG:
        if (base == BASE_BIN)
        {
            debugPrintBin(longValue);
        }
        else
        {
            debugPrintHex(longValue);
        }
        break;
    }

    if (newline)
    {
        debugPrintln();
    }
}

void debugPrint(const char *message, const char *name, byte value, Base base, bool newline)
{
    debugPrint(message, name, value, 0, 0, TYPE_BYTE, base, newline);
}

void debugPrint(const char *name, byte value, Base base, bool newline)
{
    debugPrint((const char *)0, name, value, base, newline);
}

void debugPrint(const char *message, const char *name, unsigned int value, Base base, bool newline)
{
    debugPrint(message, name, 0, value, 0, TYPE_INT, base, newline);
}

void debugPrint(const char *name, unsigned int value, Base base, bool newline)
{
    debugPrint((const char *)0, name, value, base, newline);
}

void debugPrint(const char *message, const char *name, unsigned long value, Base base, bool newline)
{
    debugPrint(message, name, 0, 0, value, TYPE_LONG, base, newline);
}

void debugPrint(const char *name, unsigned long value, Base base, bool newline)
{
    debugPrint((const char *)0, name, value, base, newline);
}