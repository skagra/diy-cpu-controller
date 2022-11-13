#ifndef _PRINTER_DOT_H_
#define _PRINTER_DOT_H_

#include <Arduino.h>

class Printer
{
public:
    enum Verbosity
    {
        none = 0,
        minimal = 1,
        verbose = 2,
        all = 3
    };

    enum Base
    {
        BASE_BIN,
        BASE_HEX,
        BASE_DEC
    };

private:
    enum ValueType
    {
        TYPE_BYTE,
        TYPE_INT,
        TYPE_LONG
    };

    static char *buffer;

    static Verbosity _verbosity;

    static void Print(const char *message, const char *name, byte byteValue, unsigned int intValue,
                      unsigned long longValue, ValueType valueType, Verbosity verbosity, Base base, bool newline);
    static void PrintBin(byte value, Verbosity verbosity, bool ms = true);
    static void PrintHex(byte value, Verbosity verbosity, bool ms = true);

public:
    static void SetVerbosity(Verbosity verbosity);

    static void Print(const char *message, Verbosity verbosity = Verbosity::minimal);

    static void Println(const char *message, Verbosity verbosity = Verbosity::minimal);

    static void Println(Verbosity verbosity = Verbosity::minimal);

    static void PrintBin(unsigned int value, Verbosity verbosity = Verbosity::minimal);

    static void PrintBin(unsigned long value, Verbosity verbosity = Verbosity::minimal);

    static void PrintHex(unsigned int value, Verbosity verbosity = Verbosity::minimal);

    static void PrintHex(unsigned long value, Verbosity verbosity = Verbosity::minimal);

    static void Print(const char *message, const char *name, byte value, Verbosity verbosity = Verbosity::minimal, Base base = BASE_HEX, bool newline = true);

    static void Print(const char *name, byte value, Verbosity verbosity = Verbosity::minimal, Base base = BASE_HEX, bool newline = true);

    static void Print(const char *message, const char *name, unsigned int value, Verbosity verbosity = Verbosity::minimal, Base base = BASE_HEX, bool newline = true);

    static void Print(const char *name, unsigned int value, Verbosity verbosity = Verbosity::minimal, Base base = BASE_HEX, bool newline = true);

    static void Print(const char *message, const char *name, unsigned long value, Verbosity verbosity = Verbosity::minimal, Base base = BASE_HEX, bool newline = true);

    static void Print(const char *name, unsigned long value, Verbosity verbosity = Verbosity::minimal, Base base = BASE_HEX, bool newline = true);
};

#endif