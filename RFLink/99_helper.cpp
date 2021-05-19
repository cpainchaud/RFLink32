#include "99_helper.h"
#include "3_Serial.h"

extern "C" {

void SerialPrintLn(const char* msg)
{
    Serial.println(msg);
}

void SerialPrintMsg(const char* msg)
{
    Serial.print(msg);
}

void SerialPrint(int value)
{
    Serial.print(value);
}


}
