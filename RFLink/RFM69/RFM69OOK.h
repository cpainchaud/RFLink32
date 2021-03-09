// **********************************************************************************
// Driver definition for HopeRF RFM69W/RFM69HW/RFM69CW/RFM69HCW, Semtech SX1231/1231H
// **********************************************************************************
// Copyright Felix Rusu (2014), felix@lowpowerlab.com
// http://lowpowerlab.com/
// **********************************************************************************
// License
// **********************************************************************************
// This program is free software; you can redistribute it
// and/or modify it under the terms of the GNU General
// Public License as published by the Free Software
// Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will
// be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE. See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General
// Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.
//
// Licence can be viewed at
// http://www.gnu.org/licenses/gpl-3.0.txt
//
// Please maintain this license information along with authorship
// and copyright notices in any redistribution of this code
// **********************************************************************************
#ifndef RFM69OOK_h
#define RFM69OOK_h
#include <Arduino.h>            //assumes Arduino IDE v1.0 or greater

#define RF69OOK_SPI_CS  SS // SS is the SPI slave select pin, for instance D10 on atmega328

// INT0 on AVRs should be connected to RFM69's DIO0 (ex on Atmega328 it's D2, on Atmega644/1284 it's D2)
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega88) || defined(__AVR_ATmega8__) || defined(__AVR_ATmega88__)
  #define RF69OOK_IRQ_PIN          3
  #define RF69OOK_IRQ_NUM          1
  #define RF69OOK_RST_PIN          NOT_A_PIN
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
  #define RF69OOK_IRQ_PIN          10
  #define RF69OOK_IRQ_NUM          0
  #define RF69OOK_RST_PIN          NOT_A_PIN
#elif defined(__AVR_ATmega32U4__)
  #define RF69OOK_IRQ_PIN          3
  #define RF69OOK_IRQ_NUM          0
  #define RF69OOK_RST_PIN          NOT_A_PIN
#elif defined(CORE_TEENSY) && defined(__MK20DX128__)
  #define RF69OOK_IRQ_PIN          4
  #define RF69OOK_IRQ_NUM          0
  #define RF69OOK_RST_PIN          NOT_A_PIN
#elif defined(CORE_TEENSY) && defined(__MK20DX256__)
  #define RF69OOK_IRQ_PIN          4
  #define RF69OOK_IRQ_NUM          0
  #define RF69OOK_RST_PIN          NOT_A_PIN
#elif defined(CORE_TEENSY) && defined(__MK66FX1M0__)
  #define RF69OOK_IRQ_PIN          4
  #define RF69OOK_IRQ_NUM          0
  #define RF69OOK_RST_PIN          NOT_A_PIN
#elif defined(ARDUINO_SAMD_ZERO)
  #define RF69OOK_IRQ_PIN          6
  #define RF69OOK_IRQ_NUM          0
  #define RF69OOK_RST_PIN          NOT_A_PIN
#elif defined(ESP32) || defined(ESP8266)
  #define RF69OOK_RST_PIN          4
  //#define RF69OOK_SPI_CS           5
#endif

#define RF69OOK_MODE_SLEEP       0 // XTAL OFF
#define RF69OOK_MODE_STANDBY     1 // XTAL ON
#define RF69OOK_MODE_SYNTH       2 // PLL ON
#define RF69OOK_MODE_RX          3 // RX MODE
#define RF69OOK_MODE_TX          4 // TX MODE

#define null                  0
#define COURSE_TEMP_COEF    -90 // puts the temperature reading in the ballpark, user can fine tune the returned value
#define RF69OOK_FSTEP 61.03515625 // == FXOSC/2^19 = 32mhz/2^19 (p13 in DS)

class RFM69OOK {
  public:
    static volatile int RSSI; //most accurate RSSI during reception (closest to the reception)
    static volatile byte _mode; //should be protected?

    RFM69OOK(byte slaveSelectPin=RF69OOK_SPI_CS, bool isRFM69HW=false) {
      _slaveSelectPin = slaveSelectPin;
      _mode = RF69OOK_MODE_STANDBY;
      _powerLevel = 31;
      _isRFM69HW = isRFM69HW;
    }

    bool initialize();
    void reset();
    uint32_t getFrequency();
    void setFrequency(uint32_t freqHz);
    void setFrequencyMHz(float f);
    void setCS(byte newSPISlaveSelect);
    int8_t readRSSI(bool forceTrigger=false);
    void setHighPower(bool onOFF=true); //have to call it after initialize for RFM69HW
    void setPowerLevel(byte level); //reduce/increase transmit power level
    void sleep();
    byte readTemperature(byte calFactor=0); //get CMOS temperature (8bit)
    void rcCalibration(); //calibrate the internal RC oscillator for use in wide temperature variations - see datasheet section [4.3.5. RC Timer Accuracy]

    // allow hacking registers by making these public
    byte readReg(byte addr);
    void writeReg(byte addr, byte val);
    void readAllRegs();

    // functions related to OOK mode
    void receiveBegin();
    void receiveEnd();
    void transmitBegin();
    void transmitEnd();
    bool poll();
    void send(bool signal);
	void setBandwidth(uint8_t bw);
    void setBitrate(uint32_t bitrate);
	void setRSSIThreshold(int8_t rssi);
	void setFixedThreshold(uint8_t threshold);
	void setSensitivityBoost(uint8_t value);

    void select();
    void unselect();

  protected:
    static RFM69OOK* selfPointer;
    byte _slaveSelectPin;
    byte _powerLevel;
    bool _isRFM69HW;
    byte _SPCR;
    byte _SPSR;

    void setMode(byte mode);
    void setHighPowerRegs(bool onOff);

    // functions related to OOK mode
    void (*userInterrupt)();
    void ookInterruptHandler();
};

#endif