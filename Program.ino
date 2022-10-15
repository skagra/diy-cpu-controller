#include "Pins.h"
#include "DataBus.h"
#include "ControlLines.h"
#include "Controller.h"

#define ALU_OP_ADD 0
#define ALU_OP_SUB ALU_OP_0
#define ALU_OP_INC ALU_OP_1
#define ALU_OP_DEC ALU_OP_0 | ALU_OP_1

#define BAUD_RATE 57600

#define SLOW_MOTION_MILLIS 1000
#define MENU_INPUT_TIMEOUT_MILLIS 10000

unsigned int slowMotionMillis = 0;
bool waitForKeyPress = false;

DataBus *dataBus = new DataBus();
ControlLines *controlLines = new ControlLines();
Controller *controller = new Controller(controlLines);

void setup()
{
    Serial.begin(BAUD_RATE);

    while (!Serial)
        delay(100);

    dataBus->detachDataBus();
    controlLines->resetControlLines();

    delay(100);
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
            slowMotionMillis = 0;
            waitForKeyPress = false;
            break;
        case 'z':
            Serial.println("Slow motion mode.");
            slowMotionMillis = SLOW_MOTION_MILLIS;
            waitForKeyPress = false;
            break;
        case 's':
            Serial.println("Single step mode.");
            slowMotionMillis = 0;
            waitForKeyPress = true;
            break;
        default:
            gotOption = false;
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

void step(byte controlLines)
{
    controller->step(controlLines);
    delay(slowMotionMillis);
    waitForKey();
}

void loop()
{
    menu();

    // Load 0xFF into the register
    Serial.println("Loading 0xFF into the register");
    dataBus->setDataBus(0xFF);
    step(REGISTER_LD);
    dataBus->detachDataBus();

    // Zero out the register, ALUA and ALUB
    Serial.println("Zeroing out register");
    dataBus->setDataBus(0);
    step(REGISTER_LD);
    dataBus->detachDataBus();

    Serial.println("Transferring register into ALUA and ALUB");
    step(REGISTER_OUT | ALU_LD_A | ALU_LD_B);

    // Store 134 in ALUA
    Serial.println("Loading 134 into register");
    dataBus->setDataBus(134);
    step(REGISTER_LD);
    dataBus->detachDataBus();

    Serial.println("Transferring register into ALUA");
    step(REGISTER_OUT | ALU_LD_A);

    // Store 50 in ALUB
    Serial.println("Loading 50 into register");
    dataBus->setDataBus(50);
    step(REGISTER_LD);
    dataBus->detachDataBus();

    Serial.println("Transferring register into ALUB");
    step(REGISTER_OUT | ALU_LD_B);

    // Add ALUA and ALUB, put the result on the bus and load into the register
    Serial.println("Add ALUA and ALUB, put the result on the bus and load it into the register");
    step(ALU_OP_ADD | ALU_OUT | REGISTER_LD);

    // Read the result
    Serial.print("Result=");
    byte result = dataBus->readDataBus();
    Serial.println(result);

    // Move the result of the previous calculation into ALUA
    Serial.println("Moving calculation result in the register result into ALUA");
    step(REGISTER_OUT | ALU_LD_A);

    // Store 22 in ALUB
    Serial.println("Loading 22 into register");
    dataBus->setDataBus(22);
    step(REGISTER_LD);
    dataBus->detachDataBus();

    Serial.println("Transferring register into ALUB");
    step(REGISTER_OUT | ALU_LD_B);

    // Subtract ALUA and ALUB, put the result on the bus and load into the register
    Serial.println("Subtract ALUA and ALUB, put the result on the bus and load it into the register");
    step(ALU_OP_SUB | ALU_OUT | REGISTER_LD);

    // Read the result
    Serial.print("Result=");
    result = dataBus->readDataBus();
    Serial.println(result);
}