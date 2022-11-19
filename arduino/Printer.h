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
    static void setVerbosity(Verbosity verbosity);

    static void print(const char *message, Verbosity verbosity = Verbosity::minimal);

    static void println(const char *message, Verbosity verbosity = Verbosity::minimal);

    static void println(Verbosity verbosity = Verbosity::minimal);

    static void printBin(unsigned int value, Verbosity verbosity = Verbosity::minimal);

    static void printBin(unsigned long value, Verbosity verbosity = Verbosity::minimal);

    static void printHex(unsigned int value, Verbosity verbosity = Verbosity::minimal);

    static void printHex(unsigned long value, Verbosity verbosity = Verbosity::minimal);

    static void print(const char *message, const char *name, byte value, Verbosity verbosity = Verbosity::minimal, Base base = BASE_HEX, bool newline = true);

    static void print(const char *name, byte value, Verbosity verbosity = Verbosity::minimal, Base base = BASE_HEX, bool newline = true);

    static void print(const char *message, const char *name, unsigned int value, Verbosity verbosity = Verbosity::minimal, Base base = BASE_HEX, bool newline = true);

    static void print(const char *name, unsigned int value, Verbosity verbosity = Verbosity::minimal, Base base = BASE_HEX, bool newline = true);

    static void print(const char *message, const char *name, unsigned long value, Verbosity verbosity = Verbosity::minimal, Base base = BASE_HEX, bool newline = true);

    static void print(const char *name, unsigned long value, Verbosity verbosity = Verbosity::minimal, Base base = BASE_HEX, bool newline = true);
};

#endif