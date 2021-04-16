// ************************************* //
// * Arduino Project RFLink32        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef Misc_h
#define Misc_h

#include <Arduino.h>

#define PRINT_BUFFER_SIZE 120 // 90         // Maximum number of characters that a command should print in one go via the print buffer.

// extern byte PKSequenceNumber;     // 1 byte packet counter
extern char pbuffer[PRINT_BUFFER_SIZE]; // Buffer for printing data

void display_Header(void);
void display_Name(const char *);
void display_Footer(void);
void display_Splash(void);
void display_IDn(unsigned long, byte);
void display_IDc(const char *);
void display_SWITCH(byte);
void display_SWITCHc(const char *);
void display_CHAN(byte);
enum CMD_Group
{
    CMD_Single,
    CMD_All
};
enum CMD_OnOff
{
    CMD_Off,
    CMD_On,
    CMD_Bright,
    CMD_Dim,
    CMD_Unknown,
    CMD_Up,
    CMD_Down,
    CMD_Stop,
    CMD_Pair
};
void display_CMD(boolean, byte);
void display_SET_LEVEL(byte);
void display_TEMP(unsigned int);
void display_HUM(byte);
void display_BARO(unsigned int);
void display_HSTATUS(byte);
void display_BFORECAST(byte);
void display_UV(unsigned int);
void display_LUX(unsigned int);
void display_BAT(boolean);
void display_RAIN(unsigned int);
void display_RAINTOT(unsigned int);
void display_RAINRATE(unsigned int);
void display_WINSP(unsigned int);
void display_AWINSP(unsigned int);
void display_WINGS(unsigned int);
void display_WINDIR(unsigned int);
void display_WINCHL(unsigned int);
void display_WINTMP(unsigned int);
void display_CHIME(unsigned int);
enum SMOKE_OnOff
{
    SMOKE_Off,
    SMOKE_On
};
void display_SMOKEALERT(boolean);
enum PIR_OnOff
{
    PIR_Off,
    PIR_On
};
void display_PIR(boolean);
void display_CO2(unsigned int);
void display_SOUND(unsigned int);
void display_KWATT(unsigned int);
void display_WATT(unsigned int);
void display_CURRENT(unsigned int);
void display_DIST(unsigned int);
void display_METER(unsigned int);
void display_VOLT(unsigned int);
void display_RGBW(unsigned int);
void display_DEBUG(byte data[], unsigned int size);

// These functions are here to help writing the emitting part of a plugin by interpreting the received command
// A local copy of the original InputBuffer_Serial is split by semi colons into tokens seperated when calling
// retrieve_Init(). The token "pointer" is located at the first found token.
// After that, each retrieve_XX method looks for a given value, with an optional case insensitive prefix, and 
// returns wether it has found it or not. In that is case, the token "pointer" is moved to the next token, ready 
// to be parsed by a call to another retrieve_XX function. 
// Note that if the token bytes do not match the expected format, the method returns false and the token 
// "pointer" is left unchanged. 
// This can be quite convenient to test for  multiple names with the retrieve_Name method, for instance.
void retrieve_Init();
boolean retrieve_Name(const char *);  // checks if the next token is equal (case insensitive) to the given string

boolean retrieve_hasPrefix(const char*);  // checks if the next token starts (case insensitive) with the given string. If that is the case, the pointer is advanced to the character right next to the prefix, effectively skipping it.

boolean retrieve_decimalNumber(unsigned long &, byte maxDigits, const char* = NULL); // retrieves a long, expressed as at most maxDigits decimal digits. Returns false if value contains non digits or too much of them. Sets 0 if no digits is found. 
boolean retrieve_hexNumber(unsigned long &, byte maxNibbles, const char* = NULL); // retrieves a long, expressed as at most maxNibbles hex digits. Returns false if value contains non hex digits or too much of them. Sets 0 if no hex digits is found.
boolean retrieve_Command(byte &, const char*); // retrieves a command, returns false if the string value is not accepted by str2cmd

boolean retrieve_long(unsigned long &, const char* = NULL);  // calls retrieve_hexNumber(, 8, )
boolean retrieve_word(uint16_t &, const char* = NULL);  // calls retrieve_hexNumber(, 4, )
boolean retrieve_byte(byte &, const char* = NULL);  // calls retrieve_hexNumber(, 2, )
boolean retrieve_nibble(byte &, const char* = NULL);  // calls retrieve_hexNumber(, 1, )

boolean retrieve_ID(unsigned long &);  // calls retrieve_long(, "ID=") and limits the value to 0x03FFFFFF (26 bits)
boolean retrieve_Switch(byte &);  // calls retrieve_nibble(, "SWITCH=")
boolean retrieve_Command(byte &); // calls retrieve_command(, "CMD=") 

boolean retrieve_End();  // returns true if the token "pointer" is at the end of the input string

// This is a specialized method that allows for "SET_LEVEL=" or "CMD=" prefix and if the value is a command, then
// sets its first parameter to constants depending on the command. If not, it sets its second argument to the value.
boolean retrieve_Command(byte &, byte &);  

#define VALUE_PAIR 44
#define VALUE_ALLOFF 55
#define VALUE_OFF 74
#define VALUE_ON 75
#define VALUE_DIM 76
#define VALUE_BRIGHT 77
#define VALUE_UP 78
#define VALUE_DOWN 79
#define VALUE_STOP 80
#define VALUE_CONFIRM 81
#define VALUE_LIMIT 82
#define VALUE_ALLON 141

#define str2int(x) atoi(x)

int str2cmd(const char *);

void replacechar(char *, char, char);

#if (defined(ESP8266) || defined(ESP32))
uint8_t String2GPIO(String);
String GPIO2String(uint8_t uGPIO);
#endif // ESP8266 || ESP32

#endif
