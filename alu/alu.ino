#include "AluInterface.h"
// #include "Pins.h"

#define BAUD_RATE 57600

#define CLOCK_PIN 12

#define BUS_PIN_LOW 2
#define BUS_PIN_HIGH 9

#define CONTROL_PIN_OFFSET 10

#define REGISTER_LD 0b00000001  // 10
#define REGISTER_OUT 0b00000010 // 11
#define ALU_LD_A 0b00000100     // 14
#define ALU_LD_B 0b00001000     // 15
#define ALU_OP_0 0b00010000     // 16
#define ALU_OP_1 0b00100000     // 17
#define ALU_OUT 0b01000000      // 18

#define ALU_OP_ADD 0
#define ALU_OP_SUB ALU_OP_0
#define ALU_OP_INC ALU_OP_1
#define ALU_OP_DEC ALU_OP_0 | ALU_OP_1

#define NUM_CONTROL_PINS 7

#define CLOCK_PULSE_DELAY_MICROS 100
#define SLOMO_MILLIS 1000
#define MENU_INPUT_TIMEOUT_MILLIS 10000

unsigned int slowmoMillis = 0;
bool waitForKeyPress = false;

static int controlBits[] = {10, 11, 14, 15, 16, 17, 18};

void setup()
{
    Serial.begin(BAUD_RATE);

    while (!Serial)
        delay(100);

    detachDataBus();

    pinMode(CLOCK_PIN, OUTPUT);
    for (int controlPin = 0; controlPin < NUM_CONTROL_PINS; controlPin++)
    {
        pinMode(controlBits[controlPin], OUTPUT);
        digitalWrite(controlBits[controlPin], LOW);
    }
    pinMode(CLOCK_PIN, LOW);
    digitalWrite(CLOCK_PIN, LOW);

    for (int offset = 0; offset < 8; offset++)
    {
        pinMode(CONTROL_PIN_OFFSET + offset, OUTPUT);
    }
    resetControlLines();

    delay(100);
}

void resetControlLines()
{
    setControlLines(0);
}

void pulseClock()
{
    digitalWrite(CLOCK_PIN, HIGH);
    delayMicroseconds(CLOCK_PULSE_DELAY_MICROS);
    digitalWrite(CLOCK_PIN, LOW);
    delayMicroseconds(CLOCK_PULSE_DELAY_MICROS);
}

int controlBitToPin(byte bitNumber)
{
    return controlBits[bitNumber];
}

void setControlLines(byte lines)
{
    for (byte offset = 0; offset < 7; offset++)
    {
        digitalWrite(controlBitToPin(offset), (lines & ((byte)1 << offset)) ? HIGH : LOW);
    }
}

void setDataBus(byte value)
{
    for (byte offset = 0; offset < 8; offset++)
    {
        pinMode(BUS_PIN_LOW + offset, OUTPUT);
        digitalWrite(BUS_PIN_LOW + offset, (value & ((byte)1 << offset)) ? HIGH : LOW);
    }
}

void detachDataBus()
{
    for (int offset = 0; offset < 8; offset++)
    {
        digitalWrite(BUS_PIN_LOW + offset, LOW);
        pinMode(BUS_PIN_LOW + offset, INPUT);
    }
}

byte readDataBus()
{
    byte result = 0;
    detachDataBus();

    for (int pin = BUS_PIN_LOW; pin <= BUS_PIN_HIGH; pin++)
    {
        if (digitalRead(pin) == HIGH)
        {
            result |= (1 << (pin - BUS_PIN_LOW));
        }
    }

    return result;
}

void step(byte controlLines, unsigned int delayMicros = 1)
{
    setControlLines(controlLines);
    pulseClock();
    delayMicroseconds(delayMicros);

    delay(slowmoMillis);
    waitForKey();
}

void menu()
{
    Serial.setTimeout(MENU_INPUT_TIMEOUT_MILLIS);

    bool gotOption = false;
    while (!gotOption)
    {
        Serial.println("Select mode: r (run), z (slow motion), s (single step)");
        while (Serial.available() == 0)
            delay(100);
        byte option = Serial.read();
        gotOption = true;
        switch (option)
        {
        case 'r':
            // Defaults are correct
            Serial.println("Normal run mode.");
            slowmoMillis = 0;
            waitForKeyPress = false;
            break;
        case 'z':
            Serial.println("Slowmotion mode.");
            slowmoMillis = SLOMO_MILLIS;
            waitForKeyPress = false;
            break;
        case 's':
            Serial.println("Single step mode.");
            slowmoMillis = 0;
            waitForKeyPress = true;
            break;
        default:
            gotOption = false;
        }
    }
}

void count(byte ldLine)
{
    for (int value = 0; value < 256; value++)
    {
        setDataBus(value);
        step(ldLine, 1000);
        detachDataBus();
    }
}

void scan(byte ldLine)
{

    for (int loop = 0; loop < 1; loop++)
    {
        for (int bit = 0; bit < 8; bit++)
        {
            setDataBus(1 << bit);
            step(ldLine, (unsigned int)64000);
            detachDataBus();
            delay(100);
        }

        for (int bit = 7; bit >= 0; bit--)
        {
            setDataBus(1 << bit);
            step(ldLine, (unsigned int)64000);
            detachDataBus();
            delay(100);
        }
    }
}

void waitForKey()
{
    if (waitForKeyPress)
    {
        while (Serial.read() == -1)
        {
            delay(100);
        }
    }
}

// TODO - add single step, full speed run, slow speed run options selectable via serial port
void loop()
{
    menu();

    // scan(REGISTER_LD);
    //      count(ALU_LD_A);
    //      count(ALU_LD_B);

    // Load 0xFF into the register
    Serial.println("Loading 0xFF into the register");
    setDataBus(0xFF);
    step(REGISTER_LD);
    detachDataBus();

    // Zero out the register, ALUA and ALUB
    Serial.println("Zeroing out register");
    setDataBus(0);
    step(REGISTER_LD);
    detachDataBus();

    Serial.println("Transferring register into ALUA and ALUB");
    step(REGISTER_OUT | ALU_LD_A | ALU_LD_B);

    // Store 134 in ALUA
    Serial.println("Loading 134 into register");
    setDataBus(134);
    step(REGISTER_LD);
    detachDataBus();

    Serial.println("Transferring register into ALUA");
    step(REGISTER_OUT | ALU_LD_A);

    // Store 50 in ALUB
    Serial.println("Loading 50 into register");
    setDataBus(50);
    step(REGISTER_LD);
    detachDataBus();

    Serial.println("Transferring register into ALUB");
    step(REGISTER_OUT | ALU_LD_B);

    // Add ALUA and ALUB, put the result on the bus and load into the register
    Serial.println("Add ALUA and ALUB, put the result on the bus and load it into the register");
    step(ALU_OP_ADD | ALU_OUT | REGISTER_LD);

    // Read the result
    Serial.print("Result=");
    byte result = readDataBus();
    Serial.println(result);

    // Move the result of the previous calculation into ALUA
    Serial.println("Moving calculation result in the register result into ALUA");
    step(REGISTER_OUT | ALU_LD_A);

    // Store 22 in ALUB
    Serial.println("Loading 22 into register");
    setDataBus(22);
    step(REGISTER_LD);
    detachDataBus();

    Serial.println("Transferring register into ALUB");
    step(REGISTER_OUT | ALU_LD_B);

    // Subtract ALUA and ALUB, put the result on the bus and load into the register
    Serial.println("Subtract ALUA and ALUB, put the result on the bus and load it into the register");
    step(ALU_OP_SUB | ALU_OUT | REGISTER_LD);

    // Read the result
    Serial.print("Result=");
    result = readDataBus();
    Serial.println(result);
}