/*==============================================================================

Filename: TM7705.c

Created on: Jan 4, 2023

Original work: Copyright (c) 2023 Luca Favenzi
Modified work:

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Visit www.favenzi.it
      luca@favenzi.it

==================================================================================
PURPOSE: Modbus CRC16 calculation
USAGE:
================================================================================*/

#include <stdio.h>
#include "globals.h"
#include "TM7705.h"

extern SPI_HandleTypeDef hspi1;
//
//static const int pinMOSI = 11; //MOSI
//static const int pinMISO = 12; //MISO
//static const int pinSPIClock = 13; //SCK
//static const int pinCS = 10; //CS
double VRef;
unsigned int readADResult();

uint8_t spiTransfer(volatile uint8_t data) {

	uint8_t txData, rxData;
	txData = data;

	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, LOW);
	HAL_SPI_Transmit(&hspi1,txData,1,500);
	HAL_SPI_Receive(&hspi1,rxData,1,500);
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, HIGH);

	return rxData;
 }


//write communication register
//   7        6      5      4      3      2      1      0
//0/DRDY(0) RS2(0) RS1(0) RS0(0) R/W(0) STBY(0) CH1(0) CH0(0)

void setNextOperation(uint8_t reg, uint8_t channel, uint8_t readWrite) {
	uint8_t r = 0;
    r = reg << 4 | readWrite << 3 | channel;
    spiTransfer(r);
}

//Clock Register
//   7      6       5        4        3        2      1      0
//ZERO(0) ZERO(0) ZERO(0) CLKDIS(0) CLKDIV(0) CLK(1) FS1(0) FS0(1)
//
//CLKDIS: master clock disable bit
//CLKDIV: clock divider bit

void writeClockRegister(uint8_t CLKDIS, uint8_t CLKDIV, uint8_t outputUpdateRate) {
	uint8_t r = CLKDIS << 4 | CLKDIV << 3 | outputUpdateRate;

    r &= ~(1 << 2); // clear CLK

    spiTransfer(r);
}

//Setup Register
//  7     6     5     4     3      2      1      0
//MD10) MD0(0) G2(0) G1(0) G0(0) B/U(0) BUF(0) FSYNC(1)

void writeSetupRegister(uint8_t operationMode, uint8_t gain, uint8_t unipolar, uint8_t buffered, uint8_t fsync) {
	uint8_t r = operationMode << 6 | gain << 3 | unipolar << 2 | buffered << 1 | fsync;

    spiTransfer(r);
}

unsigned int readADResult() {

	uint8_t b1 = spiTransfer(0x0);
	uint8_t b2 = spiTransfer(0x0);


    unsigned int r = b1 << 8 | b2;

    return r;
}

unsigned int readADResultRaw(uint8_t channel) {
    while (!dataReady(channel)) {
    };
    setNextOperation(REG_DATA, channel, 1);

    return readADResult();
}

double readADResult(uint8_t channel, float refOffset) {
    return readADResultRaw(channel) * 1.0 / 65536.0 * VRef - refOffset;
}

bool dataReady(uint8_t channel) {
    setNextOperation(REG_CMM, channel, 1);

    uint8_t b1 = spiTransfer(0x0);


    return (b1 & 0x80) == 0x0;
}

void reset() {

    for (int i = 0; i < 100; i++)
        spiTransfer(0xff);

}

//AD770X::AD770X(double vref) {
//    VRef = vref;
//    pinMode(pinMOSI, OUTPUT);
//    pinMode(pinMISO, INPUT);
//    pinMode(pinSPIClock, OUTPUT);
//    pinMode(pinCS, OUTPUT);
//
//    digitalWrite(pinCS, HIGH);
//    SPCR = _BV(SPE) | _BV(MSTR) | _BV(CPOL) | _BV(CPHA) | _BV(SPI2X) | _BV(SPR1) | _BV(SPR0);
//}

void TM7705_Init(uint8_t channel, uint8_t clkDivider, uint8_t polarity, uint8_t gain, uint8_t updRate) {
    setNextOperation(REG_CLOCK, channel, 0);
    writeClockRegister(0, clkDivider, updRate);

    setNextOperation(REG_SETUP, channel, 0);
    writeSetupRegister(MODE_SELF_CAL, gain, polarity, 0, 0);

    while (!dataReady(channel)) {
    };
}

void TM7705_Init_Def(uint8_t channel) {
    init(channel, CLK_DIV_1, BIPOLAR, GAIN_1, UPDATE_RATE_25);
}



