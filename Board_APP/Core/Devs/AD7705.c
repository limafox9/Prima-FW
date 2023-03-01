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

#define ADC_RESET_LOW			HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_RESET)
#define ADC_RESET_HIGH 			HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_SET)
#define ADC_CS_LOW				HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET)
#define ADC_CS_HIGH				HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET)
#define ADC_DATA_OUT_0			HAL_GPIO_WritePin(ADC_MOSI_GPIO_Port, ADC_MOSI_Pin, GPIO_PIN_RESET)
#define ADC_DATA_OUT_1			HAL_GPIO_WritePin(ADC_MOSI_GPIO_Port, ADC_MOSI_Pin, GPIO_PIN_SET)
#define ADC_CLOCK_0				HAL_GPIO_WritePin(ADC_CLOCK_GPIO_Port, ADC_CLOCK_Pin, GPIO_PIN_RESET)
#define ADC_CLOCK_1				HAL_GPIO_WritePin(ADC_CLOCK_GPIO_Port, ADC_CLOCK_Pin, GPIO_PIN_SET)
#define ADC_DATA_IN				HAL_GPIO_ReadPin(ADC_MISO_GPIO_Port, ADC_MISO_Pin)
#define ADC_DATA_READY			HAL_GPIO_ReadPin(ADC_DATA_READY_GPIO_Port, ADC_DATA_READY_Pin)


//SPI_HandleTypeDef* AD7705_hspi;

//static uint8_t 	AD7705_GetCommRegister(AD7705_Register regSelect, AD7705_ReadWrite rw, AD7705_Channel ch);
//static uint8_t 	AD7705_GetSetupRegister(AD7705_ADCMode mode, AD7705_Gain gain, AD7705_Polarity polarity, AD7705_Buffer buffer);
//static uint8_t 	AD7705_GetClockRegister(AD7705_ClockFrequencyFilter freq, AD7705_ClockDis clockDisable);
static void 	AD7705_HWReset();
static void 	AD7705_SWReset();
static void 	AD7705_WaitDataReady();
static void 	AD7705_ClockRegisterSet(AD7705_Channel channel, AD7705_ClockFrequencyFilter freq, AD7705_ClockDis clockDisable);
static void 	AD7705_SetupRegisterSet(AD7705_Channel channel, AD7705_ADCMode mode, AD7705_Gain gain, AD7705_Polarity polarity, AD7705_Buffer buffer, AD7705_Operation fsync);
static uint8_t 	AD7705_CommRegisterGet(AD7705_Channel channel);
static float 	AD7705_CalculateTemp(float Rt, float offset);
float ADCToTemp1(uint16_t adcValue);
uint8_t read_adc_byte();
float ConvertRtoT(float R);

void adc_delay(uint8_t NUM)	{
	for(;NUM>0;NUM--);
}

void write_adc_byte(uint8_t chr) {

	ADC_CS_LOW;
	for(uint8_t i = 0; i < 8; i++)//Write eight times in a loop
	{
		ADC_CLOCK_0;//Clock signal
		adc_delay(1);
		if (chr & 0x80) {
			ADC_DATA_OUT_1;
		} else {
			ADC_DATA_OUT_0;
		}
		chr = chr << 1;//Shift one bit of data to the right, and write out the data bit by bit in a loop
		ADC_CLOCK_1;
		adc_delay(1);
	}
	ADC_CLOCK_1;
	ADC_CS_HIGH;
}

uint16_t read_adc_word()//Read data function
{	uint8_t i;
	uint16_t coder = 0;
	uint8_t dread ; // temporry read buffer

	ADC_CS_LOW;
	for(i=0;i<16;i++)//16-bit data, read in two bytes circularly
	{
		ADC_CLOCK_0;//Clock signal
		HAL_Delay(1);
		coder = coder<<1;
		dread =  ADC_DATA_IN ;
		if(dread) coder+=1;	//Detect pin level, read in data
		ADC_CLOCK_1;
		HAL_Delay(1);
	}
	ADC_CLOCK_1;
	ADC_CS_HIGH;
	return(coder);
}

uint8_t read_adc_byte()//Read data function
{	uint8_t i;
	uint16_t coder = 0;
	uint8_t dread ; // temporry read buffer

	ADC_CS_LOW;
	for(i=0;i<8;i++)//16-bit data, read in two bytes circularly
	{
		ADC_CLOCK_0;//Clock signal
		HAL_Delay(1);
		coder = coder<<1;
		dread =  ADC_DATA_IN ;
		if(dread) coder+=1;	//Detect pin level, read in data
		ADC_CLOCK_1;
		HAL_Delay(1);
	}
	ADC_CLOCK_1;
	ADC_CS_HIGH;
	return(coder);
}

void AD7705_Init() {

//	AD7705_hspi = hspi;


	AD7705_HWReset();
	ADC_CLOCK_1;
	ADC_CS_HIGH;

	AD7705_ClockRegisterSet(CHANNEL_0, CLOCK_2_20, CLOCK_DISABLE);
//	AD7705_SetupRegisterSet(CHANNEL_0, MODE_SELF_CAL, GAIN_1, POLARITY_UNIPOLAR, BUFFER_DISABLE, CONVERSION_START);
//	HAL_Delay(100);
	AD7705_SetupRegisterSet(CHANNEL_0, MODE_SELF_CAL, GAIN_1, POLARITY_BIPOLAR, BUFFER_ENABLE, CONVERSION_START);
//	AD7705_WaitDataReady();
//	AD7705_ClockRegisterSet(CHANNEL_1, CLOCK_4_9152_50, CLOCK_ENABLE);
//	AD7705_SetupRegisterSet(CHANNEL_1, MODE_SELF_CAL, GAIN_1, POLARITY_UNIPOLAR, BUFFER_ENABLE, CONVERSION_START);
//	AD7705_WaitDataReady();
//	AD7705_SetupRegisterSet(CHANNEL_0, MODE_NORMAL, GAIN_1, POLARITY_UNIPOLAR, BUFFER_DISABLE, CONVERSION_START);
//	AD7705_SWReset();
	HAL_Delay(100);
//	AD7705_ClockRegisterSet(CHANNEL_0, CLOCK_4_9152_50, CLOCK_ENABLE);
//	AD7705_SetupRegisterSet(CHANNEL_0, MODE_SELF_CAL, GAIN_1, POLARITY_UNIPOLAR, BUFFER_DISABLE, CONVERSION_START);
//	AD7705_SetupRegisterSet(CHANNEL_0, MODE_NORMAL, GAIN_1, POLARITY_UNIPOLAR, BUFFER_ENABLE, CONVERSION_START);
//	AD7705_CommRegisterGet();

}

float AD7705_GetTemp(AD7705_Channel channel, float offset) {
	uint16_t refRes = 2000; // ref resistor for voltage divider
	uint16_t adcVal = AD7705_GetValue(channel);
//	adcVal -= 32767;
	float tempVal = AD7705_CalculateTemp((float)adcVal * (float)refRes / 65535.0, offset);
//	float tempVal = AD7705_CalculateTemp((float)adcVal * (float)refRes / 32767.0, offset);
//	float tempVal = ADCToTemp1(adcVal);
	return tempVal;

}

uint16_t AD7705_GetValue(AD7705_Channel channel) {

//	AD7705_SWReset();
//	AD7705_SetupRegisterSet(CHANNEL_0, MODE_NORMAL, GAIN_1, POLARITY_BIPOLAR, BUFFER_ENABLE, CONVERSION_START);
//	AD7705_WaitDataReady();

	uint8_t commReg = DATA_REGISTER | READ | channel;
//	HAL_SPI_Transmit(AD7705_hspi, &commReg, 1, 500);
	write_adc_byte(commReg);

	AD7705_WaitDataReady();

	uint8_t b1, b2;
	uint16_t value;
//	HAL_SPI_Receive(AD7705_hspi, &b1, 1, 500);
//	HAL_SPI_Receive(AD7705_hspi, &b2, 1, 500);
	b1 = read_adc_byte();
	b2 = read_adc_byte();
	value = b1 << 8 | b2;
//	value = read_adc_word();
	HAL_Delay(1);

    return value;
}

float ConvertRtoT(float R) {
    float A=3.9083 * pow(10,-3);
    float B=5775 * pow(10,-7);
    float T;

    T = (0-A + sqrt(pow(A,2) - 4 * B * (1 - R/1000))) / 2 * B;

    Serial.print(T);
    Serial.print(";");
    T=(0-A - sqrt(pow(A,2) - 4 * B * (1 - R/1000))) / 2 * B;
    Serial.print(T);
    return T;

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

float ADCToTemp1(uint16_t adcValue) {
	const float RTD_A =	3.9083e-3;
	const float RTD_B =	-5.775e-7;

	const float Zz1 = -RTD_A;
	const float Zz2 = RTD_A * RTD_A - (4 * RTD_B);
	const float Zz3 = (4 * RTD_B) / 1000;
	const float Zz4 = 2 * RTD_B;

	float readVolt, Rt, rRef, vRef, tCelsius;

	vRef = 3.3;
	rRef = 1.0;

	readVolt = (vRef / (float)(pow(2, 14) - 1) * (float)adcValue); 	// Voltage read
	Rt = ((vRef - readVolt) * rRef / readVolt); // PTC ohm

	tCelsius = Zz2 + (Zz3 * Rt);
	tCelsius = (sqrt(tCelsius) + Zz1) / Zz4;

//	tCelsius += (settings.T1Offset / 10.0);

	return tCelsius;
}

static void AD7705_ClockRegisterSet(AD7705_Channel channel, AD7705_ClockFrequencyFilter freq, AD7705_ClockDis clockDisable) {

	HAL_Delay(10);
	uint8_t commReg = CLOCK_REGISTER | WRITE | channel;
	 write_adc_byte(commReg);
	uint8_t clockReg  = freq | clockDisable;
	write_adc_byte(clockReg);
	HAL_Delay(10);
}

static void AD7705_SetupRegisterSet(AD7705_Channel channel, AD7705_ADCMode mode, AD7705_Gain gain, AD7705_Polarity polarity, AD7705_Buffer buffer, AD7705_Operation fsync) {

	HAL_Delay(10);
	uint8_t commReg = SETUP_REGISTER | WRITE | channel;
	write_adc_byte(commReg);
	uint8_t setupReg  = mode | gain | polarity | buffer | fsync; // FSYNC = 0 to start calibration
	write_adc_byte(setupReg);
	HAL_Delay(10);
}

//static uint8_t AD7705_CommRegisterGet(AD7705_Channel channel) {
//	uint8_t w;
//	uint8_t registerValue;
//	do {
//		SPI_CS_ON;
//		HAL_Delay(1);
//		uint8_t commReg = COMMS_REGISTER | READ | channel;
//		HAL_SPI_Transmit(AD7705_hspi, &commReg, 1, 500);
//		HAL_Delay(1);
//		HAL_SPI_Receive(AD7705_hspi, &registerValue, 1, 500);
//		HAL_Delay(1);
//		SPI_CS_OFF;
//		HAL_Delay(20);
//
//		w = HAL_GPIO_ReadPin(SPI1_DATA_READY_GPIO_Port, SPI1_DATA_READY_Pin);
//		HAL_Delay(20);
//
//	} while (w != 0);
//
//	return registerValue;
//}

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
	ADC_RESET_LOW;
	HAL_Delay(10);
	ADC_RESET_HIGH;
}

static void AD7705_SWReset() {
	write_adc_byte(0xff);
	write_adc_byte(0xff);
	write_adc_byte(0xff);
	write_adc_byte(0xff);
	HAL_Delay(10);
}

static void AD7705_WaitDataReady() {
	uint8_t w;
	do {
		w = ADC_DATA_READY;
		osDelay(1);
	} while (w);
	HAL_Delay(10);
}
