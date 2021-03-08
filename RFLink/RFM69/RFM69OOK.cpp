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
#include "RFM69OOK.h"
#include "RFM69OOKregisters.h"
#include <SPI.h>

volatile byte RFM69OOK::_mode;  // current transceiver state
volatile int RFM69OOK::RSSI; 	// most accurate RSSI during reception (closest to the reception)
RFM69OOK* RFM69OOK::selfPointer;

bool RFM69OOK::initialize()
{
  const byte CONFIG[][2] =
  {
    /* 0x01 */ { REG_OPMODE, RF_OPMODE_SEQUENCER_OFF | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY },
    /* 0x02 */ { REG_DATAMODUL, RF_DATAMODUL_DATAMODE_CONTINUOUSNOBSYNC | RF_DATAMODUL_MODULATIONTYPE_OOK | RF_DATAMODUL_MODULATIONSHAPING_00 }, // no shaping
/*!!!! 0x03 */ { REG_BITRATEMSB, RF_BITRATEMSB_19200 }, // Bitrate => necessary even in OOK
/*!!!! 0x04 */ { REG_BITRATELSB, RF_BITRATELSB_19200 }, // 9600 instead of 4800 for Energy Harvesting Switch
/*!!!! 0x07 */ { REG_FRFMSB, 0x6C }, // Rem : FStep = FXOsc / 2^19
/*!!!! 0x08 */ { REG_FRFMID, 0x7A }, //       FRF = FStep * Frf
/*!!!! 0x09 */ { REG_FRFLSB, 0xE1 }, //       FRF = 433.92 MHz
    /* 0x18 */ { REG_LNA, RF_LNA_ZIN_200 | RF_LNA_CURRENTGAIN | RF_LNA_GAINSELECT_AUTO },
/*!!!! 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_16 | RF_RXBW_EXP_0 }, // DCCFREQ_010 = filter 4%BW / EXP1 = BW 125 kHz / EXP0 = BW 250 kHz
/*!!!! 0x1A */ { REG_AFCBW, RF_AFCBW_DCCFREQAFC_001 | RF_AFCBW_MANTAFC_16 | RF_AFCBW_EXPAFC_0 }, // IN AutoFreqCorrect Filter=1% & BW=250Hz
/*!!!! 0x1B */ { REG_OOKPEAK, RF_OOKPEAK_THRESHTYPE_PEAK | RF_OOKPEAK_PEAKTHRESHSTEP_000 | RF_OOKPEAK_PEAKTHRESHDEC_011 },
/*!!!! 0x1D */ { REG_OOKFIX, 6 }, // Fixed threshold value (in dB) in the OOK demodulator
/*!!!! 0x1E */ { REG_AFCFEI, RF_AFCFEI_AFCAUTO_OFF | RF_AFCFEI_AFCAUTOCLEAR_ON | RF_AFCFEI_AFC_START | RF_AFCFEI_AFC_CLEAR }, // AutoFrequencyCorrect at StartUp
    /* 0x25 */ { REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_11 | RF_DIOMAPPING1_DIO1_10 | RF_DIOMAPPING1_DIO2_01 | RF_DIOMAPPING1_DIO3_10 }, // DIO0 ModeReady, DIO1 RX/TX Ready, DIO2 Data, DIO3 AutoMode
    /* 0x26 */ { REG_DIOMAPPING2, RF_DIOMAPPING2_DIO4_11 | RF_DIOMAPPING2_DIO5_00 | RF_DIOMAPPING2_CLKOUT_OFF }, // DIO4 PllLock, DIO5 ClkOut disable
/*!!!! 0x29 */ { REG_RSSITHRESH, RF_RSSITHRESH_VALUE-28-2* 20 }, // RSSI threshold = REG_RSSITHRESH/2 = 100 + xx dBm 
/*!!!! 0x58 */ { REG_TESTLNA, RF_TESTLNA_HIGH_SENSITIVITY }, // High Sensitivity
    /* 0x6F */ { REG_TESTDAGC, RF_DAGC_IMPROVED_LOWBETA0 }, // run DAGC continuously in RX mode, recommended default for AfcLowBetaOn0
    {255, 0}
  };

  pinMode(_slaveSelectPin, OUTPUT);
  SPI.begin();

  for (byte i = 0; CONFIG[i][0] != 255; i++)
    writeReg(CONFIG[i][0], CONFIG[i][1]);

  setHighPower(_isRFM69HW); // called regardless if it's a RFM69W or RFM69HW
  setMode(RF69OOK_MODE_STANDBY);

  auto startTime = millis();

  while((readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00) { // Wait for ModeReady
    delay(1);
    if( millis() - startTime > 5000 ) {
      startTime = millis();
      Serial.println("RFM69 has not yet initialized properly, is it correctly wired? (repeat every 5 seconds)");
    }
  }

  selfPointer = this;
  return true;
}

void RFM69OOK::reset() // maroprjs : Manual Reset
{
	pinMode(RF69OOK_RST_PIN, OUTPUT);
	delay(10);
	digitalWrite(RF69OOK_RST_PIN, LOW);
	delay(10);
	digitalWrite(RF69OOK_RST_PIN, HIGH);
	delay(150);
	digitalWrite(RF69OOK_RST_PIN, LOW);
	delay(7);
}

// Poll for OOK signal
bool RFM69OOK::poll()
{
  return digitalRead(_interruptPin);
}

// Send a 1 or 0 signal in OOK mode
void RFM69OOK::send(bool signal)
{
  digitalWrite(_interruptPin, signal);
}

// Turn the radio into transmission mode
void RFM69OOK::transmitBegin()
{
  setMode(RF69OOK_MODE_TX);
  detachInterrupt(_interruptNum); // not needed in TX mode
  pinMode(_interruptPin, OUTPUT);
}

// Turn the radio back to standby
void RFM69OOK::transmitEnd()
{
  pinMode(_interruptPin, INPUT);
  setMode(RF69OOK_MODE_STANDBY);
}

// Turn the radio into OOK listening mode
void RFM69OOK::receiveBegin()
{
  pinMode(_interruptPin, INPUT);
  attachInterrupt(_interruptNum, RFM69OOK::isr0, CHANGE); // generate interrupts in RX mode
  setMode(RF69OOK_MODE_RX);
}

// Turn the radio back to standby
void RFM69OOK::receiveEnd()
{
  setMode(RF69OOK_MODE_STANDBY);
  detachInterrupt(_interruptNum); // make sure there're no surprises
}

// Handle pin change interrupts in OOK mode
void RFM69OOK::interruptHandler()
{
  if (userInterrupt != NULL) (*userInterrupt)();
}

// Set a user interrupt for all transfer methods in receive mode
// call with NULL to disable the user interrupt handler
void RFM69OOK::attachUserInterrupt(void (*function)())
{
  userInterrupt = function;
}

// return the frequency (in Hz)
uint32_t RFM69OOK::getFrequency()
{
  return RF69OOK_FSTEP * (((uint32_t)readReg(REG_FRFMSB)<<16) + ((uint16_t)readReg(REG_FRFMID)<<8) + readReg(REG_FRFLSB));
}

// Set literal frequency using floating point MHz value
void RFM69OOK::setFrequencyMHz(float f)
{
  setFrequency(f * 1000000);
}

// set the frequency (in Hz)
void RFM69OOK::setFrequency(uint32_t freqHz)
{
  // TODO: p38 hopping sequence may need to be followed in some cases
  freqHz /= RF69OOK_FSTEP; // divide down by FSTEP to get FRF
  writeReg(REG_FRFMSB, freqHz >> 16);
  writeReg(REG_FRFMID, freqHz >> 8);
  writeReg(REG_FRFLSB, freqHz);
}

// Set bitrate
void RFM69OOK::setBitrate(uint32_t bitrate)
{
  bitrate = 32000000 / bitrate; // 32M = XCO freq.
  writeReg(REG_BITRATEMSB, bitrate >> 8);
  writeReg(REG_BITRATELSB, bitrate);
}

// set OOK bandwidth
void RFM69OOK::setBandwidth(uint8_t bw)
{
  writeReg(REG_RXBW, (readReg(REG_RXBW) & 0xE0) | bw);
}

// set RSSI threshold
void RFM69OOK::setRSSIThreshold(int8_t rssi)
{
  writeReg(REG_RSSITHRESH, (-rssi << 1));
}

// set OOK fixed threshold
void RFM69OOK::setFixedThreshold(uint8_t threshold)
{
  writeReg(REG_OOKFIX, threshold);
}

// set sensitivity boost in REG_TESTLNA
// see: http://www.sevenwatt.com/main/rfm69-ook-dagc-sensitivity-boost-and-modulation-index
void RFM69OOK::setSensitivityBoost(uint8_t value)
{
  writeReg(REG_TESTLNA, value);
}

void RFM69OOK::setMode(byte newMode)
{
    if (newMode == _mode) return;

    switch (newMode) {
        case RF69OOK_MODE_TX:
            writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_TRANSMITTER);
      if (_isRFM69HW) setHighPowerRegs(true);
            break;
        case RF69OOK_MODE_RX:
            writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_RECEIVER);
      if (_isRFM69HW) setHighPowerRegs(false);
            break;
        case RF69OOK_MODE_SYNTH:
            writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_SYNTHESIZER);
            break;
        case RF69OOK_MODE_STANDBY:
            writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_STANDBY);
            break;
        case RF69OOK_MODE_SLEEP:
            writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_SLEEP);
            break;
        default: return;
    }

    // waiting for mode ready is necessary when going from sleep because the FIFO may not be immediately available from previous mode
    while (_mode == RF69OOK_MODE_SLEEP && (readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00); // Wait for ModeReady

    _mode = newMode;
}

void RFM69OOK::sleep() {
  setMode(RF69OOK_MODE_SLEEP);
}

// set output power: 0=min, 31=max
// this results in a "weaker" transmitted signal, and directly results in a lower RSSI at the receiver
void RFM69OOK::setPowerLevel(byte powerLevel)
{
  _powerLevel = powerLevel;
  writeReg(REG_PALEVEL, (readReg(REG_PALEVEL) & 0xE0) | (_powerLevel > 31 ? 31 : _powerLevel));
}

void IRAM_ATTR RFM69OOK::isr0() { selfPointer->interruptHandler(); }

int8_t RFM69OOK::readRSSI(bool forceTrigger) {
  if (forceTrigger)
  {
    // RSSI trigger not needed if DAGC is in continuous mode
    writeReg(REG_RSSICONFIG, RF_RSSI_START);
    while ((readReg(REG_RSSICONFIG) & RF_RSSI_DONE) == 0x00); // Wait for RSSI_Ready
  }
  return -(readReg(REG_RSSIVALUE) >> 1);
}

byte RFM69OOK::readReg(byte addr)
{
  select();
  SPI.transfer(addr & 0x7F);
  byte regval = SPI.transfer(0);
  unselect();
  return regval;
}

void RFM69OOK::writeReg(byte addr, byte value)
{
  select();
  SPI.transfer(addr | 0x80);
  SPI.transfer(value);
  unselect();
}

// Select the transceiver
void RFM69OOK::select() {
  noInterrupts();
  // save current SPI settings
  #if !defined(ESP32) && !defined(ESP8266)
  _SPCR = SPCR;
  _SPSR = SPSR;
  #endif
  // set RFM69 SPI settings
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4); //decided to slow down from DIV2 after SPI stalling in some instances, especially visible on mega1284p when RFM69 and FLASH chip both present
  digitalWrite(_slaveSelectPin, LOW);
}

/// UNselect the transceiver chip
void RFM69OOK::unselect() {
  digitalWrite(_slaveSelectPin, HIGH);
  // restore SPI settings to what they were before talking to RFM69
   #if !defined(ESP32) && !defined(ESP8266)
  SPCR = _SPCR;
  SPSR = _SPSR;
  #endif
  interrupts();
}

void RFM69OOK::setHighPower(bool onOff) {
  _isRFM69HW = onOff;
  writeReg(REG_OCP, _isRFM69HW ? RF_OCP_OFF : RF_OCP_ON);
  if (_isRFM69HW) // turning ON
    writeReg(REG_PALEVEL, (readReg(REG_PALEVEL) & 0x1F) | RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_ON); // enable P1 & P2 amplifier stages
  else
    writeReg(REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | _powerLevel); // enable P0 only
}

void RFM69OOK::setHighPowerRegs(bool onOff) {
  writeReg(REG_TESTPA1, onOff ? 0x5D : 0x55);
  writeReg(REG_TESTPA2, onOff ? 0x7C : 0x70);
}

void RFM69OOK::setCS(byte newSPISlaveSelect) {
  _slaveSelectPin = newSPISlaveSelect;
  pinMode(_slaveSelectPin, OUTPUT);
}

// for debugging
void RFM69OOK::readAllRegs()
{
  byte regVal;
  for (byte regAddr = 1; regAddr <= 0x4F; regAddr++) {
    regVal = readReg(regAddr);
    Serial.print(regAddr, HEX);
    Serial.print(" - ");
    Serial.print(regVal,HEX);
    Serial.print(" - ");
    Serial.println(regVal,BIN);
  }
}

byte RFM69OOK::readTemperature(byte calFactor)  // returns centigrade
{
  setMode(RF69OOK_MODE_STANDBY);
  writeReg(REG_TEMP1, RF_TEMP1_MEAS_START);
  while ((readReg(REG_TEMP1) & RF_TEMP1_MEAS_RUNNING));
  return ~readReg(REG_TEMP2) + COURSE_TEMP_COEF + calFactor; // 'complement' corrects the slope, rising temp = rising val
}                                                            // COURSE_TEMP_COEF puts reading in the ballpark, user can add additional correction

void RFM69OOK::rcCalibration()
{
  writeReg(REG_OSC1, RF_OSC1_RCCAL_START);
  while ((readReg(REG_OSC1) & RF_OSC1_RCCAL_DONE) == 0x00);
}