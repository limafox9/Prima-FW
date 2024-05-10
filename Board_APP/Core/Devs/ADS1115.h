/*==============================================================================

Filename: ADS1115.h

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



#ifndef LIBS_ADS1115_ADS1115_H_
#define LIBS_ADS1115_ADS1115_H_


#include "stdint.h"
#include "stdlib.h"

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"

#define ADS1115_ADDRESS 0x48

typedef enum{
	CHANNEL_AIN0_AIN1 = 0,
	CHANNEL_AIN0_AIN3,
	CHANNEL_AIN1_AIN3,
	CHANNEL_AIN2_AIN3,
	CHANNEL_AIN0_GND,
	CHANNEL_AIN1_GND,
	CHANNEL_AIN2_GND,
	CHANNEL_AIN3_GND
}MultiplexerConfig_t;

typedef enum{
	PGA_6_144 = 0,
	PGA_4_096,
	PGA_2_048,
	PGA_1_024,
	PGA_0_512,
	PGA_0_256
}PGA_Config_t;


typedef enum{
	MODE_CONTINOUS = 0,
	MODE_SINGLE_SHOT
}OperatingMode_t;

typedef enum{
	DRATE_8 = 0,
	DRATE_16,
	DRATE_32,
	DRATE_64,
	DRATE_128,
	DRATE_250,
	DRATE_475,
	DRATE_860
}DataRate_t;

typedef enum{
	COMP_HYSTERESIS = 0,
	COMP_WINDOW
}CompareMode_t;

typedef enum{
	POLARITY_ACTIVE_LOW = 0,
	POLARITY_ACTIVE_HEIGH
}ComparePolarity_t;

typedef enum{
	LATCHING_NONE = 0,
	LATCHING_COMPARATOR
}LatchingMode_t;

typedef enum{
	QUEUE_ONE,
	QUEUE_TWO,
	QUEUE_FOUR,
	QUEUE_DISABLE
}QueueComparator_t;

typedef struct{
	MultiplexerConfig_t channel;
	PGA_Config_t 		pgaConfig;
	OperatingMode_t 	operatingMode;
	DataRate_t			dataRate;
	CompareMode_t		compareMode;
	ComparePolarity_t	polarityMode;
	LatchingMode_t		latchingMode;
	QueueComparator_t	queueComparator;
}ADS1115_Config_t;

typedef struct{
	uint16_t	operationalStatus	: 1;
	uint16_t	inputMultiplexer	: 3;
	uint16_t	pgaConfig			: 3;
	uint16_t	operatingMode		: 1;
	uint16_t	dataRate			: 3;
	uint16_t	comparatorMode		: 1;
	uint16_t	comparatorPolarity	: 1;
	uint16_t	compartorLatching	: 1;
	uint16_t	comparatorQueue		: 2;
}ADS1115_ConfigReg_t;

typedef struct ADS1115_Config_Tag ADS1115_Handle_t;

float ADS1115_GetRawData();
float ADS1115_GetVoltage2(uint8_t channel);
float ADS1115_GetVoltage4(uint8_t channel);
uint16_t ADS1115_GetValue(uint8_t channel);

ADS1115_Handle_t* ADS1115_init(I2C_HandleTypeDef *hi2c, uint16_t Addr, ADS1115_Config_t config);
void ADS1115_deinit();
void ADS1115_updateConfig(ADS1115_Handle_t *pConfig, ADS1115_Config_t config);
void ADS1115_updateI2Chandler(ADS1115_Handle_t *pConfig, I2C_HandleTypeDef *hi2c);
void ADS1115_updateAddress(ADS1115_Handle_t *pConfig, uint16_t address);
uint16_t ADS1115_oneShotMeasure();
uint16_t ADS1115_getData();
void ADS1115_setThresholds(ADS1115_Handle_t *pConfig, int16_t lowValue, int16_t highValue);
void ADS1115_flushData(ADS1115_Handle_t* pConfig);
void ADS1115_setConversionReadyPin(ADS1115_Handle_t* pConfig);
void ADS1115_startContinousMode(ADS1115_Handle_t* pConfig);
void ADS1115_stopContinousMode(ADS1115_Handle_t* pConfig);

#endif /* LIBS_ADS1115_ADS1115_H_ */
