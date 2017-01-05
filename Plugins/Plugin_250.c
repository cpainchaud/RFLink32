//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                       Plugin-250: Addons                                          ##
//#######################################################################################################

/*********************************************************************************************\
 * This plugin takes care of addons for Nodo Small boards
 * 
 * Support            : http://www.esp8266.nu/forum
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************/

#ifdef PLUGIN_TX_250

// prototyping functions
byte Plugin_250_GetArgv(char *, char *, int);
unsigned long Plugin_250_FreeMem(void);
void Plugin_250_Beep(int, int);


boolean Plugin_250(byte function, char *string)
{
  return false;
}


boolean PluginTX_250(byte function, char *string)
{
  boolean success=false;
  char TmpStr1[80];
  TmpStr1[0] = 0;
  long Par1 = 0;
  long Par2 = 0;
  if (Plugin_250_GetArgv(string, TmpStr1, 3)) Par1 = atol(TmpStr1);
  if (Plugin_250_GetArgv(string, TmpStr1, 4)) Par2 = atol(TmpStr1);

  if (strncasecmp_P(InputBuffer_Serial+3,PSTR("FREEMEM"),7) == 0)
  { 
    Serial.println(Plugin_250_FreeMem());
    success=true;
  }

  if (strncasecmp_P(InputBuffer_Serial+3,PSTR("SOUND"),5) == 0)
  {
    if (Par2 < 10000)
      {
        pinMode(PIN_SPEAKER, OUTPUT);
        Plugin_250_Beep(Par1,Par2);
      }
    success=true;
  }

  if (strncasecmp_P(InputBuffer_Serial+3,PSTR("GPIO"),4) == 0)
  {
    if (Par1 >= 7 && Par1 <=10)
    {
      pinMode(Par1, OUTPUT);
      digitalWrite(Par1, Par2);
    }
    success=true;
  }

  if (strncasecmp_P(InputBuffer_Serial+3,PSTR("STATUS"),6) == 0)
  {
    int state=0;
    if (Par1 >= 0 && Par1 <=3) state = analogRead(Par1);
    if (Par1 >= 7 && Par1 <=10) state = digitalRead(Par1);
    sprintf_P(pbuffer,PSTR("20;%02X;ESPEASY;PinValue%u"), PKSequenceNumber++, Par1);
    Serial.print(pbuffer); 
    sprintf(pbuffer,"=%u",state);
    Serial.println(pbuffer);
    success=true;
  }

  return success;
}

void Plugin_250_Scan()
{
  static unsigned long timer;
  if (millis()-timer > 100)
  {
    // add timer logic for reporting ana
    timer=millis();
  }
}


byte Plugin_250_GetArgv(char *string, char *argv, int argc)
{
  int string_pos=0,argv_pos=0,argc_pos=0; 
  char c,d;

  while(string_pos<strlen(string))
  {
    c=string[string_pos];
    d=string[string_pos+1];

    if       (c==' ' && d==' '){}
    else if  (c==' ' && d==';'){}
    else if  (c==';' && d==' '){}
    else if  (c==' ' && d>=33 && d<=126){}
    else if  (c==';' && d>=33 && d<=126){}
    else 
      {
      if(c!=' ' && c!=';')
        {
        argv[argv_pos++]=c;
        argv[argv_pos]=0;
        }          

      if(d==' ' || d==';' || d==0)
        {
        // Bezig met toevoegen van tekens aan een argument, maar er kwam een scheidingsteken.
        argv[argv_pos]=0;
        argc_pos++;

        if(argc_pos==argc)
          return string_pos+1;
          
        argv[0]=0;
        argv_pos=0;
        string_pos++;
      }
    }
    string_pos++;
  }
  return 0;
}


uint8_t *heapptr, *stackptr;
unsigned long Plugin_250_FreeMem(void)
  {
  stackptr = (uint8_t *)malloc(4);        // use stackptr temporarily
  heapptr = stackptr;                     // save value of heap pointer
  free(stackptr);                         // free up the memory again (sets stackptr to 0)
  stackptr =  (uint8_t *)(SP);            // save value of stack pointer
  return (stackptr-heapptr);
  }


void Plugin_250_Beep(int frequency, int duration)//Herz,millisec 
  {
  long halfperiod=500000L/frequency;
  long loops=(long)duration*frequency/(long)1000;

  for(loops;loops>0;loops--) 
    {
    digitalWrite(PIN_SPEAKER, HIGH);
    delayMicroseconds(halfperiod);
    digitalWrite(PIN_SPEAKER, LOW);
    delayMicroseconds(halfperiod);
    }
  }

#endif // Plugin_TX_250
