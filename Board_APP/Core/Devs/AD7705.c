/*==============================================================================

Filename: AD7705.c

Created on: 28 feb 2023

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

================================================================================
PURPOSE:
USAGE:
==============================================================================*/

#include "AD7705.h"
#include "stm32f1xx_hal.h"

#define RESET_ON 	HAL_GPIO_WritePin(SPI1_RESET_GPIO_Port, SPI1_RESET_Pin, GPIO_PIN_RESET)
#define RESET_OFF 	HAL_GPIO_WritePin(SPI1_RESET_GPIO_Port, SPI1_RESET_Pin, GPIO_PIN_SET)
#define SPI_CS_ON	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET)
#define SPI_CS_OFF	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET)



SPI_HandleTypeDef* AD7705_hspi;

static uint8_t 	AD7705_GetCommRegister(AD7705_Register regSelect, AD7705_ReadWrite rw, AD7705_Channel ch);
static uint8_t 	AD7705_GetSetupRegister(AD7705_ADCMode mode, AD7705_Gain gain, AD7705_Polarity polarity, AD7705_Buffer buffer);
static uint8_t 	AD7705_GetClockRegister(AD7705_ClockFrequencyFilter freq, AD7705_ClockDis clockDisable);
static void 	AD7705_HWReset();
static void 	AD7705_SWReset();
static void 	AD7705_WaitDataReady();
static void 	AD7705_ClockRegisterSet(AD7705_ClockFrequencyFilter freq, AD7705_ClockDis clockDisable);
static void 	AD7705_SetupRegisterSet(AD7705_ADCMode mode, AD7705_Gain gain, AD7705_Polarity polarity, AD7705_Buffer buffer, AD7705_Operation fsync);
static uint8_t 	AD7705_CommRegisterGet();
static float 	AD7705_CalculateTemp(float Rt, float offset);

void AD7705_Init(SPI_HandleTypeDef* hspi) {

	AD7705_hspi = hspi;

//	AD7705_HWReset();
//	HAL_Delay(1);
	AD7705_SWReset();
	HAL_Delay(1);
	AD7705_ClockRegisterSet(CLOCK_4_9152_50, CLOCK_ENABLE);
	AD7705_SetupRegisterSet(MODE_SELF_CAL, GAIN_1, POLARITY_UNIPOLAR, BUFFER_DISABLE, CONVERSION_START);
	AD7705_SetupRegisterSet(MODE_NORMAL, GAIN_1, POLARITY_UNIPOLAR, BUFFER_ENABLE, CONVERSION_START);
//	AD7705_CommRegisterGet();

}

float AD7705_GetTemp(AD7705_Channel channel, float offset) {
	uint16_t refRes = 2000; // ref resistor for voltage divider
	float retVal = 0;
	retVal = AD7705_CalculateTemp((float)AD7705_GetValue(channel) * (float)refRes / 65535.0, offset);
	return retVal;

}

uint16_t AD7705_GetValue(AD7705_Channel channel) {

	SPI_CS_ON;
	uint8_t commReg = DATA_REGISTER | READ | channel;
	HAL_SPI_Transmit(AD7705_hspi, &commReg, 1, 100);
	HAL_Delay(20);
	SPI_CS_OFF;

	AD7705_WaitDataReady();

	uint8_t b1, b2;
	uint16_t value;
	SPI_CS_ON;
//	HAL_SPI_Receive(AD7705_hspi, &b1, 1, 100);
//	HAL_SPI_Receive(AD7705_hspi, &b2, 1, 100);
//	uint16_t value = b1 << 8 | b2;
	HAL_SPI_Receive(AD7705_hspi, &value, 2, 100);
	SPI_CS_OFF;

    return value;
}

static float AD7705_CalculateTemp(float Rt, float offset) {

	const float RTD_A = 3.9083e-3;
	const float RTD_B = -5.775e-7;

	float Z1, Z2, Z3, Z4, temp;

	Z1 = -RTD_A;
	Z2 = RTD_A * RTD_A - (4 * RTD_B);
	Z3 = (4 * RTD_B) / 1000;
	Z4 = 2 * RTD_B;

	temp = Z2 + (Z3 * Rt);
	temp = (sqrt(temp) + Z1) / Z4;

	temp += offset;

	return temp;
}

static void AD7705_ClockRegisterSet(AD7705_ClockFrequencyFilter freq, AD7705_ClockDis clockDisable) {

	SPI_CS_ON;
	uint8_t commReg = CLOCK_REGISTER | WRITE | CHANNEL_0;
	HAL_SPI_Transmit(AD7705_hspi, &commReg, 1, 100);
	HAL_Delay(10);
	uint8_t clockReg  = 0b10100000 | freq | clockDisable;
	HAL_SPI_Transmit(AD7705_hspi, &clockReg, 1, 100);
	SPI_CS_OFF;
	HAL_Delay(20);
}

static void AD7705_SetupRegisterSet(AD7705_ADCMode mode, AD7705_Gain gain, AD7705_Polarity polarity, AD7705_Buffer buffer, AD7705_Operation fsync) {

	SPI_CS_ON;
	uint8_t commReg = SETUP_REGISTER | WRITE | CHANNEL_0;
	HAL_SPI_Transmit(AD7705_hspi, &commReg, 1, 100);
	HAL_Delay(10);
	uint8_t setupReg  = mode | gain | polarity | buffer | fsync; // FSYNC = 0 to start calibration
	HAL_SPI_Transmit(AD7705_hspi, &setupReg, 1, 100);
	SPI_CS_OFF;
	HAL_Delay(20);
}

static uint8_t AD7705_CommRegisterGet() {
	uint8_t w;
	uint8_t registerValue;
	do {
		SPI_CS_ON;
		uint8_t commReg = COMMS_REGISTER | READ | CHANNEL_0;
		HAL_SPI_Transmit(AD7705_hspi, &commReg, 1, 100);


		HAL_SPI_Receive(AD7705_hspi, &registerValue, 1, 100);
		HAL_Delay(10);
		SPI_CS_OFF;

		w = HAL_GPIO_ReadPin(SPI1_DATA_READY_GPIO_Port, SPI1_DATA_READY_Pin);
		osDelay(1);

	} while (w != 0);

	return registerValue;
}

//static uint8_t AD7705_GetCommRegister(AD7705_Register regSelect, AD7705_ReadWrite rw, AD7705_Channel ch) {
//	return (regSelect | rw | ch);
//}
//
//static uint8_t AD7705_GetSetupRegister(AD7705_ADCMode mode, AD7705_Gain gain, AD7705_Polarity polarity, AD7705_Buffer buffer) {
//	return (mode | gain | polarity | buffer);
//}
//
//static uint8_t AD7705_GetClockRegister(AD7705_ClockFrequencyFilter freq, AD7705_ClockDis clockDisable) {
//	return (freq | clockDisable);
//}

static void AD7705_HWReset() {
	RESET_ON;
	HAL_Delay(10);
	RESET_OFF;
}

static void AD7705_SWReset() {
	SPI_CS_ON;
	HAL_Delay(1);
	HAL_SPI_Transmit(AD7705_hspi, 0xff, 1, 500);
	HAL_SPI_Transmit(AD7705_hspi, 0xff, 1, 500);
	HAL_SPI_Transmit(AD7705_hspi, 0xff, 1, 500);
	HAL_SPI_Transmit(AD7705_hspi, 0xff, 1, 500);
	HAL_Delay(1);
	SPI_CS_OFF;
}

static void AD7705_WaitDataReady() {
	uint8_t w;
	do {
		w = HAL_GPIO_ReadPin(SPI1_DATA_READY_GPIO_Port, SPI1_DATA_READY_Pin);
		osDelay(1);
	} while (w);
}
