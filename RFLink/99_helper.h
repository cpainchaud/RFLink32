/**
 * This header is here to allow calling Serial.printXXX methods from C files
 * It is most useful when "debugging" the rtl_433 code
*/

#ifndef Helper_H
#define Helper_H

#ifdef __cplusplus
extern "C" {
#endif
void SerialPrintLn(const char* msg);
void SerialPrintMsg(const char* msg);
void SerialPrint(int value);
void SerialPrintFreeHeap();
void SerialPrintMaxAllocHeap();
void SerialPrintFreeMemInfo();
#ifdef __cplusplus
}
#endif

#endif