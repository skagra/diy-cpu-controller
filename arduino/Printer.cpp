#include "Printer.h"

char *Printer::buffer = new char[9];
Printer::Verbosity Printer::_verbosity = Printer::Verbosity::all;

void Printer::setVerbosity(Verbosity verbosity)
{
    _verbosity = verbosity;
}

void Printer::print(const char *message, Verbosity verbosity)
{
    if (verbosity <= _verbosity)
    {
        Serial.print(message);
    }
}

void Printer::println(const char *message, Verbosity verbosity)
{
    if (verbosity <= _verbosity)
    {
        Serial.println(message);
    }
}

void Printer::println(Verbosity verbosity)
{
    if (verbosity <= _verbosity)
    {
        Serial.println();
    }
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

void Printer::PrintBin(byte value, Verbosity verbosity, bool ms)
{
    if (ms)
    {
        print("0b", verbosity);
    }
    sprintf(buffer, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(value));
    print(buffer, verbosity);
}

void Printer::printBin(unsigned int value, Verbosity verbosity)
{
    PrintBin((byte)(value >> 8), verbosity);
    print("-", verbosity);
    PrintBin((byte)value, verbosity);
}

void Printer::printBin(unsigned long value, Verbosity verbosity)
{
    PrintBin((byte)(value >> 24), verbosity);
    print("-", verbosity);
    PrintBin((byte)(value >> 16), verbosity, false);
    print("-", verbosity);
    PrintBin((byte)(value >> 8), verbosity, false);
    print("-", verbosity);
    PrintBin((byte)value, verbosity, false);
}

void Printer::PrintHex(byte value, Verbosity verbosity, bool ms)
{
    if (ms)
    {
        print("0x", verbosity);
    }
    sprintf(buffer, "%.2X", value);
    print(buffer, verbosity);
}

void Printer::printHex(unsigned int value, Verbosity verbosity)
{
    PrintHex((byte)(value >> 8), verbosity);
    print("-", verbosity);
    PrintHex((byte)value, verbosity, false);
}

void Printer::printHex(unsigned long value, Verbosity verbosity)
{
    PrintHex((byte)(value >> 24), verbosity);
    print("-", verbosity);
    PrintHex((byte)(value >> 16), verbosity, false);
    print("-", verbosity);
    PrintHex((byte)(value >> 8), verbosity, false);
    print("-", verbosity);
    PrintHex((byte)value, verbosity, false);
}

void Printer::Print(const char *message, const char *name, byte byteValue, unsigned int intValue,
                    unsigned long longValue, ValueType valueType, Verbosity verbosity, Base base, bool newline)
{
    if (message != (char *)0)
    {
        print(message, verbosity);
        print(", ", verbosity);
    }

    print(name, verbosity);
    print("=", verbosity);

    switch (valueType)
    {
    case TYPE_BYTE:
        if (base == BASE_BIN)
        {
            PrintBin(byteValue, verbosity);
        }
        else
        {
            PrintHex(byteValue, verbosity);
        }
        break;
    case TYPE_INT:
        if (base == BASE_BIN)
        {
            printBin(intValue, verbosity);
        }
        else
        {
            printHex(intValue, verbosity);
        }
        break;
    case TYPE_LONG:
        if (base == BASE_BIN)
        {
            printBin(longValue, verbosity);
        }
        else
        {
            printHex(longValue, verbosity);
        }
        break;
    }

    if (newline)
    {
        println(verbosity);
    }
}

void Printer::print(const char *message, const char *name, byte value, Verbosity verbosity, Base base, bool newline)
{
    Print(message, name, value, 0, 0, TYPE_BYTE, verbosity, base, newline);
}

void Printer::print(const char *name, byte value, Verbosity verbosity, Base base, bool newline)
{
    print((const char *)0, name, value, verbosity, base, newline);
}

void Printer::print(const char *message, const char *name, unsigned int value, Verbosity verbosity, Base base, bool newline)
{
    Print(message, name, 0, value, 0, TYPE_INT, verbosity, base, newline);
}

void Printer::print(const char *name, unsigned int value, Verbosity verbosity, Base base, bool newline)
{
    print((const char *)0, name, value, verbosity, base, newline);
}

void Printer::print(const char *message, const char *name, unsigned long value, Verbosity verbosity, Base base, bool newline)
{
    Print(message, name, 0, 0, value, TYPE_LONG, verbosity, base, newline);
}

void Printer::print(const char *name, unsigned long value, Verbosity verbosity, Base base, bool newline)
{
    print((const char *)0, name, value, verbosity, base, newline);
}