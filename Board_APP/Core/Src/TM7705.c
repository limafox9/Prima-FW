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

#include "TM7705.h"

#include <stdio.h>
#include "globals.h"

//	Communication register bits
const uint8_t COM_DRDY_BIT = 7;
const uint8_t COM_RS_BIT = 4;
const uint8_t COM_RW_BIT = 3;
const uint8_t COM_STBY_BIT = 2;
const uint8_t COM_CH1_BIT = 1;
const uint8_t COM_CH0_BIT = 0;

//	Setup register bits
const uint8_t SETUP_MODE_BIT = 6;
const uint8_t SETUP_GAIN_BIT = 3;
const uint8_t SETUP_BIPOLAR_MODE_BIT = 2;
const uint8_t SETUP_BUFFER_CTRL_BIT = 1;
const uint8_t SETUP_FSYNC_BIT = 0;

//	Clock Register
const uint8_t CLOCK_CLKDIS_BIT = 4;
const uint8_t CLOCK_CLKDIV_BIT = 3;
const uint8_t CLOCK_CLK_BIT = 2;
const uint8_t CLOCK_FS = 0;

SPI_HandleTypeDef* TM7705_hspi;

static uint8_t spiTransfer(uint8_t* txByte, uint8_t* rxByte);
static void waitDataReady();
static void writeToCommsRegister(TM7705_RegisterSelection registerID, TM7705_ReadWrite readWrite, TM7705_Channel channel);
static void setClockRegister(TM7705_ClockFrequency clockFrequency, const e_Boolean clockOutDisable);
static void setSetupRegister(const TM7705_Channel channel, const TM7705_ADCMode adcMode, const TM7705_Gain gain, const TM7705_Polarity polarity, const e_Boolean buffered);
static void reset();


uint8_t GetCommRegister(TM7705_RegisterSelection regSelect, TM7705_ReadWrite rw, TM7705_Channel ch) {
	return (regSelect << COM_RS_BIT) | (rw << COM_RW_BIT) | (ch << COM_STBY_BIT) | CHANNEL_0;
}

uint8_t GetSetupRegister(TM7705_ADCMode mode, TM7705_Gain gain, TM7705_Polarity polarity, TM7705_Buffer buffer) {
	return (mode << SETUP_MODE_BIT) | (gain << SETUP_GAIN_BIT) | (polarity << SETUP_BIPOLAR_MODE_BIT) | (buffer << SETUP_BUFFER_CTRL_BIT);
}

uint8_t GetClockRegister(TM7705_ClockOut clkdis, TM7705_ClockFrequency freq, TM7705_Rate rate) {
	return (clkdis << CLOCK_CLKDIS_BIT) | (freq << CLOCK_CLK_BIT) | rate;
}

void TM7705_Init(SPI_HandleTypeDef* hspi) {

	TM7705_hspi = hspi;

	TM7705_hspi->Init.Mode = SPI_MODE_MASTER;
	TM7705_hspi->Init.Direction = SPI_DIRECTION_2LINES;
	TM7705_hspi->Init.DataSize = SPI_DATASIZE_8BIT;
	TM7705_hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;

	TM7705_hspi->Init.CLKPolarity = SPI_POLARITY_HIGH;
	TM7705_hspi->Init.CLKPhase = SPI_PHASE_2EDGE;
	TM7705_hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;

	TM7705_hspi->Init.NSS = SPI_NSS_SOFT;
	TM7705_hspi->Init.TIMode = SPI_TIMODE_DISABLE;
	TM7705_hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	TM7705_hspi->Init.CRCPolynomial = 10;

	if (HAL_SPI_Init(TM7705_hspi) != HAL_OK)
	{
		Error_Handler();
	}

	uint8_t commReg;
	uint8_t setupReg  = GetSetupRegister(MODE_NORMAL, GAIN_1, POLARITY_UNIPOLAR, BUFFER_OFF);
	uint8_t clockReg  = GetClockRegister(CLKOUT_ENABLE, CLOCK_4_9152, UPDATE_RATE_50);

	uint8_t rx;

	//reset();

//	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
//	HAL_Delay(1);
	commReg = GetCommRegister(SETUP_REGISTER, WRITE, CHANNEL_0);
	spiTransfer(&commReg, &rx);
	spiTransfer(&setupReg, &rx);

	commReg = GetCommRegister(CLOCK_REGISTER, WRITE, CHANNEL_0);
	spiTransfer(&commReg, &rx);
	spiTransfer(&clockReg, &rx);

//    spiTransfer(0b00100000);
//    spiTransfer(0b00011100);
//
//    spiTransfer(0b00010000);
//    spiTransfer(0b01000000);

//	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);

//
//
//	//osDelay(1);
//	setClockRegister(CLOCK_4_9152, TRUE);
//	//setSetupRegister(CHANNEL_0, MODE_NORMAL, GAIN_1, BIPOLAR, FALSE);
//	setSetupRegister(CHANNEL_0, MODE_SELF_CAL, GAIN_1, BIPOLAR, FALSE);

	//setSetupRegister(CHANNEL_1, adcMode, gain, polarity, FALSE);
	waitDataReady();
}

void reset() {

	HAL_GPIO_WritePin(SPI1_RESET_GPIO_Port, SPI1_RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(SPI1_RESET_GPIO_Port, SPI1_RESET_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
}

static uint8_t spiTransfer(uint8_t* txByte, uint8_t* rxByte) {

	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_SPI_Transmit(TM7705_hspi, txByte, 1, 500);
	HAL_SPI_Receive(TM7705_hspi, rxByte, 1, 500);
	HAL_Delay(1);
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);

	return rxByte;
}

void waitDataReady() {

	int8_t w;

	do {
		w = HAL_GPIO_ReadPin(SPI1_DATA_READY_GPIO_Port, SPI1_DATA_READY_Pin);
		//osDelay(1);
	} while (w);
}

//uint16_t TM7705_GetValue(const TM7705_Channel channel) {
//
//	waitDataReady();
//
//	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
//	HAL_Delay(1);
//
//	//writeToCommsRegister(DATA_REGISTER, READ, channel);
//    spiTransfer(0b00111000);
//    //  Wait for data ready to go low
//	HAL_Delay(1);
//
//    uint16_t value = spiTransfer(0) << 8;
//	HAL_Delay(1);
//
//    value |= spiTransfer(0);
//
//    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
//	HAL_Delay(1);
//
//    return value;
//}


//
//void setClockRegister(const TM7705_ClockFrequency clockFrequency, const e_Boolean clockOutDisable) {
//
//    writeToCommsRegister(CLOCK_REGISTER, WRITE, CHANNEL_0);
//    spiTransfer(0b00011100);
//}
//
//void setSetupRegister(const TM7705_Channel channel, const TM7705_ADCMode adcMode, const TM7705_Gain gain, const TM7705_Polarity polarity, const e_Boolean buffered) {
//
//	uint8_t setupRegValue =	0;
//	setupRegValue =	((adcMode & 0x3) << SETUP_MODE_BIT) | ( (gain & 0x7) << SETUP_GAIN_BIT) | (polarity << SETUP_BIPOLAR_MODE_BIT) | (buffered << SETUP_BUFFER_CTRL_BIT);
//
//    writeToCommsRegister(SETUP_REGISTER, WRITE, channel);
//
//    spiTransfer(setupRegValue);
////    spiTransfer(0x10);
////    spiTransfer(0x40);
//}
//
//void writeToCommsRegister(const TM7705_RegisterSelection registerID, const TM7705_ReadWrite readWrite, const TM7705_Channel channel) {
//	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
//	HAL_Delay(1);
//
//	uint8_t value = 0;
//	value = ((registerID & 0x7) << COM_RS_BIT) | (readWrite << COM_RW_BIT) | (0x3 & channel);
//
//    spiTransfer(value);
//    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
//	HAL_Delay(1);
//}

//
//uint32_t TM7705_ReadOffsetRegister(const TM7705_Channel channel)
//{
//    writeToCommsRegister(OFFSET_REGISTER, READ, channel);
//
//    uint32_t value = ((uint32_t)spiTransfer(0x0)) << 16;
//    value |= ((uint32_t)spiTransfer(0x0)) << 8;
//    value |= spiTransfer(0x0);
//
//    return value;
//}
//
//uint32_t TM7705_ReadFullScaleRegister(const TM7705_Channel channel)
//{
//    writeToCommsRegister(OFFSET_REGISTER, READ, channel);
//
//    uint32_t value = ((uint32_t)spiTransfer(0x0)) << 16;
//    value |= ((uint32_t)spiTransfer(0x0)) << 8;
//    value |= spiTransfer(0x0);
//
//    return value;
//}







//adc = TM7705_ReadAdc(2);
//		res = ((double)adc) * refRes / 65535;
//		tmp = calTemp(res);
//		if(((tmp < realPara.t2 + 5) && (tmp + 5 > realPara.t2)) || (uTime.now < uTime.powerOn + 20))
//		{
//			realPara.t2 = adjust_t1t2(tmp);




























//void TM7705_Init_Default(SPI_HandleTypeDef* hspi, uint8_t channel) {
//	TM7705_Init(hspi, channel, CLK_DIV_1, BIPOLAR, GAIN_1, UPDATE_RATE_25);
//}
//
//uint16_t TM7705_GetData(uint8_t channel) {
//
//	while (!dataReady(channel)) {
//    };
//
//	setNextOperation(REG_DATA, channel, 1);
//
//	uint8_t b1 = spiTransfer(0x0);
//	uint8_t b2 = spiTransfer(0x0);
//
//    uint16_t r = b1 << 8 | b2;
//
//    return r;
//}
//
////Write Communication Register
////   7        6      5      4      3      2      1      0
////0/DRDY(0) RS2(0) RS1(0) RS0(0) R/W(0) STBY(0) CH1(0) CH0(0)
//
//void setNextOperation(uint8_t reg, uint8_t channel, uint8_t readWrite) {
//	uint8_t r = 0;
//    r = reg << 4 | readWrite << 3 | channel;
//    spiTransfer(r);
//}
//
////Clock Register
////   7      6       5        4        3        2      1      0
////ZERO(0) ZERO(0) ZERO(0) CLKDIS(0) CLKDIV(0) CLK(1) FS1(0) FS0(1)
////
////CLKDIS: master clock disable bit
////CLKDIV: clock divider bit
//
//void writeClockRegister(TM7705_ClockFrequency clockFrequency, const uint8_t	filterSelect) {
//
//    uint8_t clockReg =	(boolToBit(clkDivBit) << CLOCK_CLKDIV_BIT) | (BitTools::boolToBit(clockBit) << CLOCK_CLK_BIT ) | (filterSelect & 0x3 );
//
//	uint8_t r = CLKDIS << 4 | CLKDIV << 3 | outputUpdateRate;
//    r &= ~(1 << 2); // clear CLK
//    spiTransfer(r);
//}
//
////Setup Register
////  7     6     5     4     3      2      1      0
////MD10) MD0(0) G2(0) G1(0) G0(0) B/U(0) BUF(0) FSYNC(1)
//
//void writeSetupRegister(uint8_t operationMode, uint8_t gain, uint8_t unipolar, uint8_t buffered, uint8_t fsync) {
//	uint8_t r = operationMode << 6 | gain << 3 | unipolar << 2 | buffered << 1 | fsync;
//    spiTransfer(r);
//}
//
//
////AD770X::AD770X(double vref) {
////    VRef = vref;
////    pinMode(pinMOSI, OUTPUT);
////    pinMode(pinMISO, INPUT);
////    pinMode(pinSPIClock, OUTPUT);
////    pinMode(pinCS, OUTPUT);
////
////    digitalWrite(pinCS, HIGH);
////    SPCR = _BV(SPE) | _BV(MSTR) | _BV(CPOL) | _BV(CPHA) | _BV(SPI2X) | _BV(SPR1) | _BV(SPR0);
////}
//


