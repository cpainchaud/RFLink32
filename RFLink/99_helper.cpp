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

void SerialPrintFreeHeap()
{
    Serial.print(ESP.getFreeHeap());
}

void SerialPrintMaxAllocHeap()
{
    Serial.print(ESP.getMaxAllocHeap());
}

void SerialPrintFreeMemInfo()
{
    Serial.print("8bit aligned, Free = ");
    Serial.print(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    Serial.print(", Max block = ");
    Serial.print(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    Serial.print(", 32bit aligned, Free = ");
    Serial.print(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT));
    Serial.print(", Max block = ");
    Serial.print(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT));
}

}
