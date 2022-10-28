#ifndef _DEBUG_DOT_H_
#define _DEBUG_DOT_H_

#include <Arduino.h>

#define DEBUG 1

enum Base
{
    BASE_BIN,
    BASE_HEX
};

void debugPrint(const char *message);

void debugPrintln(const char *message);

void debugPrintln();

void debugPrintBin(byte value, bool ms = true);

void debugPrintBin(unsigned int value);

void debugPrintBin(unsigned long value);

void debugPrintHex(byte value, bool ms = true);

void debugPrintHex(unsigned int value);

void debugPrintHex(unsigned long value);

void debugPrint(const char *message, const char *name, byte value, Base base = BASE_HEX, bool newline = true);

void debugPrint(const char *name, byte value, Base base = BASE_HEX, bool newline = true);

void debugPrint(const char *message, const char *name, unsigned int value, Base base = BASE_HEX, bool newline = true);

void debugPrint(const char *name, unsigned int value, Base base = BASE_HEX, bool newline = true);

void debugPrint(const char *message, const char *name, unsigned long value, Base base = BASE_HEX, bool newline = true);

void debugPrint(const char *name, unsigned long value, Base base = BASE_HEX, bool newline = true);

#endif