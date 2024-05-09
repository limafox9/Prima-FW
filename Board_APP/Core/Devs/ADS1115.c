/*==============================================================================

Filename: ADS1115.c

Created on: Apr 29, 2024

Original work: Copyright (c) 2024 Luca Favenzi
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

#include "ADS1115.h"

struct ADS1115_Config_Tag{
	I2C_HandleTypeDef 	*hi2c;
	uint16_t 			address;
	ADS1115_Config_t 	config;
};

extern I2C_HandleTypeDef hi2c1;


static void prepareConfigFrame(uint8_t *pOutFrame, ADS1115_Config_t config);


// Public functions

float ADS1115_GetRawData() {
	unsigned char ADSwrite[6];
	uint16_t reading;
	float voltage[4];
	const float voltageConv = 6.144 / 32768.0;

	ADSwrite[0] = 0x01;
	ADSwrite[1] = 0xC1; //11000001
	ADSwrite[2] = 0x83; //10000011 LSB

	HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS << 1, ADSwrite, 3, 100);
	ADSwrite[0] = 0x00;
	HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS << 1 , ADSwrite, 1 ,100);
	HAL_Delay(20);

	HAL_I2C_Master_Receive(&hi2c1, ADS1115_ADDRESS <<1, ADSwrite, 2, 100);

	reading = (ADSwrite[0] << 8 | ADSwrite[1] );

	return (float)reading * voltageConv;

}

uint16_t ADS1115_GetValue(uint8_t channel) {
	unsigned char ADSwrite[6];
	uint16_t reading;

	ADSwrite[0] = 0x01;

	switch (channel) {
	case 0:
		ADSwrite[1] = 0xC5;
		break;
	case 1:
		ADSwrite[1] = 0xD5;
		break;
	case 2:
		ADSwrite[1] = 0xE3;
		break;
	case 3:
		ADSwrite[1] = 0xF3;
		break;
	default:
		ADSwrite[1] = 0xC3;
		break;
	}

	ADSwrite[2] = 0x83; //10000011 LSB

	HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS << 1, ADSwrite, 3, 100);
	ADSwrite[0] = 0x00;
	HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS << 1 , ADSwrite, 1 ,100);
	HAL_Delay(20);

	HAL_I2C_Master_Receive(&hi2c1, ADS1115_ADDRESS <<1, ADSwrite, 2, 100);

	reading = (ADSwrite[0] << 8 | ADSwrite[1] );

	return reading;

}

float ADS1115_GetVoltage(uint8_t channel) {
	unsigned char ADSwrite[6];
	uint16_t reading;
	float voltage[4];
	const float voltageConv = 2.048 / 32768.0;

	ADSwrite[0] = 0x01;

	switch (channel) {
	case 0:
		ADSwrite[1] = 0xC5;
		break;
	case 1:
		ADSwrite[1] = 0xD5;
		break;
	case 2:
		ADSwrite[1] = 0xE5;
		break;
	case 3:
		ADSwrite[1] = 0xF5;
		break;
	default:
		ADSwrite[1] = 0xC3;
		break;
	}

	ADSwrite[2] = 0x83; //10000011 LSB

	HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS << 1, ADSwrite, 3, 100);
	ADSwrite[0] = 0x00;
	HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS << 1 , ADSwrite, 1 ,100);
	HAL_Delay(20);

	HAL_I2C_Master_Receive(&hi2c1, ADS1115_ADDRESS <<1, ADSwrite, 2, 100);

	reading = (ADSwrite[0] << 8 | ADSwrite[1] );

	return (float)reading * voltageConv;

}




// Private functions

ADS1115_Handle_t* ADS1115_init(I2C_HandleTypeDef *hi2c, uint16_t Addr, ADS1115_Config_t config){
	ADS1115_Handle_t *pConfig = malloc(sizeof(ADS1115_Handle_t));
	pConfig->hi2c = hi2c;
	pConfig->address = Addr;
	pConfig->config = config;
	return pConfig;
}

void ADS1115_deinit(ADS1115_Handle_t* pConfig){
	free(pConfig);
}

void ADS1115_updateConfig(ADS1115_Handle_t *pConfig, ADS1115_Config_t config){
	pConfig->config = config;

	uint8_t bytes[3] = {0};
	prepareConfigFrame(bytes, pConfig->config);

	HAL_I2C_Master_Transmit(pConfig->hi2c, (pConfig->address << 1), bytes, 3, 100);
}

void ADS1115_updateI2Chandler(ADS1115_Handle_t *pConfig, I2C_HandleTypeDef *hi2c){
	pConfig->hi2c = hi2c;
}

void ADS1115_updateAddress(ADS1115_Handle_t *pConfig, uint16_t address){
	pConfig->address = address;
}

uint16_t ADS1115_oneShotMeasure(ADS1115_Handle_t *pConfig){
	uint8_t bytes[3] = {0};

	prepareConfigFrame(bytes, pConfig->config);

	bytes[1] |= (1 << 7); // OS one shot measure

	HAL_I2C_Master_Transmit(pConfig->hi2c, (pConfig->address << 1), bytes, 3, 100);

	return ADS1115_getData(pConfig);
}

uint16_t ADS1115_getData(ADS1115_Handle_t *pConfig){
	uint8_t bytes[2] = {0};
	bytes[0] = 0x00;
	HAL_I2C_Master_Transmit(pConfig->hi2c, (pConfig->address << 1), bytes, 1, 50);

	if(HAL_I2C_Master_Receive(pConfig->hi2c, (pConfig->address << 1), bytes, 2, 50) != HAL_OK)
		return 0;

	uint16_t readValue = ((bytes[0] << 8) | bytes[1]);
//	if(readValue < 0)
//		readValue = 0;

	return readValue;
}


void ADS1115_setThresholds(ADS1115_Handle_t *pConfig, int16_t lowValue, int16_t highValue){
	uint8_t ADSWrite[3] = { 0 };

	//hi threshold reg
	ADSWrite[0] = 0x03;
	ADSWrite[1] = (uint8_t)((highValue & 0xFF00) >> 8);
	ADSWrite[2] = (uint8_t)(highValue & 0x00FF);
	HAL_I2C_Master_Transmit(pConfig->hi2c, (pConfig->address << 1), ADSWrite, 3, 100);

	//lo threshold reg
	ADSWrite[0] = 0x02;
	ADSWrite[1] = (uint8_t)((lowValue & 0xFF00) >> 8);
	ADSWrite[2] = (uint8_t)(lowValue & 0x00FF);
	HAL_I2C_Master_Transmit(pConfig->hi2c, (pConfig->address << 1), ADSWrite, 3, 100);
}

void ADS1115_flushData(ADS1115_Handle_t* pConfig){
	ADS1115_getData(pConfig);
}

void ADS1115_setConversionReadyPin(ADS1115_Handle_t* pConfig){
	ADS1115_setThresholds(pConfig, 0x0000, 0xFFFF);
}

void ADS1115_startContinousMode(ADS1115_Handle_t* pConfig){
	uint8_t bytes[3] = {0};

	ADS1115_Config_t configReg = pConfig->config;
	configReg.operatingMode = MODE_CONTINOUS;
	prepareConfigFrame(bytes, configReg);

	HAL_I2C_Master_Transmit(pConfig->hi2c, (pConfig->address << 1), bytes, 3, 100);
}

void ADS1115_stopContinousMode(ADS1115_Handle_t* pConfig){
	uint8_t bytes[3] = {0};
	ADS1115_Config_t configReg = pConfig->config;
	configReg.operatingMode = MODE_SINGLE_SHOT;
	prepareConfigFrame(bytes, configReg);

	HAL_I2C_Master_Transmit(pConfig->hi2c, (pConfig->address << 1), bytes, 3, 100);
}

static void prepareConfigFrame(uint8_t *pOutFrame, ADS1115_Config_t config){
	pOutFrame[0] = 0x01;
	pOutFrame[1] |= (config.channel << 4) | (config.pgaConfig << 1)
					| (config.operatingMode << 0);
	pOutFrame[2] |= (config.dataRate << 5) | (config.compareMode << 4) | (config.polarityMode << 3)
					| (config.latchingMode << 2) | (config.queueComparator << 0);
}
