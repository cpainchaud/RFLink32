/*********************************************************************************************\
   Convert HEX or DEC tring to unsigned long HEX, DEC
  \*********************************************************************************************/
unsigned long str2int(char *string) {
  return (strtoul(string, NULL, 0));
}
/*********************************************************************************************\
   Convert string to command code
  \*********************************************************************************************/
/*
  int str2cmd(char *command) {
  if (strcasecmp(command, "ON") == 0) return VALUE_ON;
  if (strcasecmp(command, "OFF") == 0) return VALUE_OFF;
  if (strcasecmp(command, "ALLON") == 0) return VALUE_ALLON;
  if (strcasecmp(command, "ALLOFF") == 0) return VALUE_ALLOFF;
  if (strcasecmp(command, "PAIR") == 0) return VALUE_PAIR;
  if (strcasecmp(command, "DIM") == 0) return VALUE_DIM;
  if (strcasecmp(command, "BRIGHT") == 0) return VALUE_BRIGHT;
  if (strcasecmp(command, "UP") == 0) return VALUE_UP;
  if (strcasecmp(command, "DOWN") == 0) return VALUE_DOWN;
  if (strcasecmp(command, "STOP") == 0) return VALUE_STOP;
  if (strcasecmp(command, "CONFIRM") == 0) return VALUE_CONFIRM;
  if (strcasecmp(command, "LIMIT") == 0) return VALUE_LIMIT;
  return false;
  }
*/
/********************************************************************************************\
   Convert unsigned long to float long through memory
  \*********************************************************************************************/
float ul2float(unsigned long ul) {
  float f;
  memcpy(&f, &ul, 4);
  return f;
}
/*********************************************************************************************/
void PrintHex8(uint8_t *data, uint8_t length) { // prints 8-bit data in hex (lowercase)
  char tmp[length * 2 + 1];
  byte first ;
  int j = 0;
  for (uint8_t i = 0; i < length; i++) {
    first = (data[i] >> 4) | 48;
    if (first > 57) tmp[j] = first + (byte)39;
    else tmp[j] = first ;
    j++;

    first = (data[i] & 0x0F) | 48;
    if (first > 57) tmp[j] = first + (byte)39;
    else tmp[j] = first;
    j++;
  }
  tmp[length * 2] = 0;
  Serial.print(tmp);
}
/*********************************************************************************************/
// todo: make uppercase?  3a = 3 or 48 (0x30) = 0x33   >57 (0x39)   a>3a >39 >   +27
void PrintHexByte(uint8_t data) { // prints 8-bit value in hex (single byte)
  char tmp[3];
  byte first ;
  first = (data >> 4) | 48;                   // or with 0x30
  if (first > 57) tmp[0] = first + (byte)7;   // 39;  // if > 0x39 add 0x27
  else tmp[0] = first ;

  first = (data & 0x0F) | 48;
  if (first > 57) tmp[1] = first + (byte)7;  // 39;
  else tmp[1] = first;
  tmp[2] = 0;
  Serial.print(tmp);
}
/*********************************************************************************************/
// Reverse all bits in a byte
byte reverseBits(byte data) {
  byte b = data;
  for (byte i = 0; i < 8; ++i) {
    data = (data << 1) | (b & 1);
    b >>= 1;
  }
  return data;
}
/*********************************************************************************************/
