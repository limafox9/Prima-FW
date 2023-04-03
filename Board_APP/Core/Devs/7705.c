/*==============================================================================

Filename: 7705.c

Created on: Mar 1, 2023

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

#include "7705.h"

#include <stdio.h>
#include "stm32f1xx_hal.h"
#include "main.h"

#define SOFT_SPI // C51 IO

#define __CH1_GAIN_BIPOLAR_BUF	(GAIN_1 | UNIPOLAR | BUF_EN)
#define __CH2_GAIN_BIPOLAR_BUF	(GAIN_1 | UNIPOLAR | BUF_EN)

#ifdef SOFT_SPI

#define RESET_0				HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_RESET)
#define RESET_1 			HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_SET)
#define CS_0				HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET)
#define CS_1				HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET)
#define DATA_OUT_0			HAL_GPIO_WritePin(ADC_MOSI_GPIO_Port, ADC_MOSI_Pin, GPIO_PIN_RESET)
#define DATA_OUT_1			HAL_GPIO_WritePin(ADC_MOSI_GPIO_Port, ADC_MOSI_Pin, GPIO_PIN_SET)
#define CLOCK_0				HAL_GPIO_WritePin(ADC_CLOCK_GPIO_Port, ADC_CLOCK_Pin, GPIO_PIN_RESET)
#define CLOCK_1				HAL_GPIO_WritePin(ADC_CLOCK_GPIO_Port, ADC_CLOCK_Pin, GPIO_PIN_SET)
#define DATA_IN				HAL_GPIO_ReadPin(ADC_MISO_GPIO_Port, ADC_MISO_Pin)
#define DATA_READY			HAL_GPIO_ReadPin(ADC_DATA_READY_GPIO_Port, ADC_DATA_READY_Pin)

#endif

/* Communication register bit definition */
enum {
	/* register selection RS2 RS1 RS0  */
	REG_COMM	= 0x00,
	REG_SETUP	= 0x10,
	REG_CLOCK	= 0x20,
	REG_DATA	= 0x30,
	REG_ZERO_CH1	= 0x60,
	REG_FULL_CH1	= 0x70,
	REG_ZERO_CH2	= 0x61,
	REG_FULL_CH2	= 0x71,

	WRITE 		= 0x00,
	READ 		= 0x08,

	CH_1		= 0,	/* AIN1+  AIN1- */
	CH_2		= 1,	/* AIN2+  AIN2- */
	CH_3		= 2,	/* AIN1-  AIN1- */
	CH_4		= 3		/* AIN1-  AIN2- */
};

enum {
	MD_NORMAL		= (0 << 6),
	MD_CAL_SELF		= (1 << 6),
	MD_CAL_ZERO		= (2 << 6),
	MD_CAL_FULL		= (3 << 6),

	GAIN_1			= (0 << 3),
	GAIN_2			= (1 << 3),
	GAIN_4			= (2 << 3),
	GAIN_8			= (3 << 3),
	GAIN_16			= (4 << 3),
	GAIN_32			= (5 << 3),
	GAIN_64			= (6 << 3),
	GAIN_128		= (7 << 3),

	BIPOLAR			= (0 << 2),
	UNIPOLAR		= (1 << 2),

	BUF_NO			= (0 << 1),
	BUF_EN			= (1 << 1),

	FSYNC_0			= 0,
	FSYNC_1			= 1
};


enum {
	CLKDIS_0	= 0x00,
	CLKDIS_1	= 0x10,

 	CLK_4_9152M = 0x08,
	CLK_2_4576M = 0x00,
	CLK_1M 		= 0x04,
	CLK_2M 		= 0x0C,

	FS_50HZ		= 0x00,
	FS_60HZ		= 0x01,
	FS_250HZ	= 0x02,
	FS_500HZ	= 0x04,

 	ZERO_0		= 0x00,
	ZERO_1		= 0x80
};


static void TM7705_SyncSPI();
static void TM7705_Send8Bit(unsigned char _data);
static unsigned char TM7705_Recive8Bit();
static void TM7705_WriteByte(unsigned char _data);
static void TM7705_Write3Byte(unsigned long _data);
static unsigned char TM7705_ReadByte();
static unsigned short TM7705_Read2Byte();
static unsigned long TM7705_Read3Byte();
static void TM7705_WaitDRDY();
static void TM7705_ResetHard();


void TM7705_Init() {
	TM7705_ResetHard();
	HAL_Delay(5);

	TM7705_SyncSPI();
	HAL_Delay(5);

 	TM7705_WriteByte(REG_CLOCK | WRITE | CH_1);
 	TM7705_WriteByte(CLKDIS_0 | CLK_4_9152M | FS_50HZ);

 	TM7705_CalibSelf(1);
 	HAL_Delay(5);
}

static void TM7705_ResetHard() {
	RESET_1;
	HAL_Delay(1);
	RESET_0;
	HAL_Delay(2);
	RESET_1;
	HAL_Delay(1);
}


static void TM7705_SyncSPI() {
 	CS_0;
	TM7705_Send8Bit(0xFF);
	TM7705_Send8Bit(0xFF);
	TM7705_Send8Bit(0xFF);
	TM7705_Send8Bit(0xFF);
	CS_1;
}

 static void TM7705_Send8Bit(unsigned char _data) {
	unsigned char i;

	for(i = 0; i < 8; i++) {
		if (_data & 0x80) {
			DATA_OUT_1;
		}
		else {
			DATA_OUT_0;
		}
		CLOCK_0;
		_data <<= 1;
		CLOCK_1;
	}
}

 static unsigned char TM7705_Recive8Bit() {
	unsigned char i;
	unsigned char retc = 0;

	for (i = 0; i < 8; i++) {
		CLOCK_0;
		retc <<= 1;
		if (DATA_IN) {
			retc++;
		}
		CLOCK_1;
	}
	return retc;
}

 static void TM7705_WriteByte(unsigned char _data) {
	CS_0;
	TM7705_Send8Bit(_data);
	CS_1;
}

 static void TM7705_Write3Byte(unsigned long _data)
{
	CS_0;
	TM7705_Send8Bit((_data >> 16) & 0xFF);
	TM7705_Send8Bit((_data >> 8) & 0xFF);
	TM7705_Send8Bit(_data);
	CS_1;
}


 static unsigned char TM7705_ReadByte() {
	unsigned char read;

	CS_0;
	read = TM7705_Recive8Bit();
	CS_1;

	return read;
}

 static unsigned short TM7705_Read2Byte() {
	unsigned short read;

	CS_0;
	read = TM7705_Recive8Bit();
	read <<= 8;
	read += TM7705_Recive8Bit();
	CS_1;

	return read;
}

 static unsigned long TM7705_Read3Byte() {
	unsigned long read;

	CS_0;
	read = TM7705_Recive8Bit();
	read <<= 8;
	read += TM7705_Recive8Bit();
	read <<= 8;
	read += TM7705_Recive8Bit();
	CS_1;

	return read;
}

//	功能说明: 等待内部操作完成。 自校准时间较长，需要等待。
static void TM7705_WaitDRDY() {
	unsigned short i;

	for (i = 0; i < 8000; i++) {
		if (!DATA_READY) {
			break;
		}
	}

	if (i >= 8000) {
 		TM7705_SyncSPI();
		HAL_Delay(5);
	}
}
 void TM7705_WriteReg(unsigned char _RegID, unsigned long _RegValue) {
	unsigned char bits;

	switch (_RegID) {
		case REG_COMM:
		case REG_SETUP:
		case REG_CLOCK:
			bits = 8;
			break;

		case REG_ZERO_CH1:
		case REG_FULL_CH1:
		case REG_ZERO_CH2:
		case REG_FULL_CH2:
			bits = 24;
			break;

		case REG_DATA:
		default:
			return;
	}

	TM7705_WriteByte(_RegID | WRITE);

	if (bits == 8) {
		TM7705_WriteByte((unsigned char)_RegValue);
	}
	else { //24 bits
		TM7705_Write3Byte(_RegValue);
	}
}

 unsigned long TM7705_ReadReg(unsigned char _RegID) {
	unsigned char bits;
	unsigned long read;

	switch (_RegID) {
		case REG_COMM:
		case REG_SETUP:
		case REG_CLOCK:
			bits = 8;
			break;

		case REG_ZERO_CH1:
		case REG_FULL_CH1:
		case REG_ZERO_CH2:
		case REG_FULL_CH2:
			bits = 24;
			break;

		case REG_DATA:
		default:
			return 0xFFFFFFFF;
	}

	TM7705_WriteByte(_RegID | READ);

	if (bits == 16) {
		read = TM7705_Read2Byte();
	}
	else if (bits == 8) {
		read = TM7705_ReadByte();
	}
	else { 	// 24 bits
		read = TM7705_Read3Byte();
	}
	return read;
}

 void TM7705_CalibSelf(unsigned char _ch) {
	if (_ch == 1) {
		/* 自校准CH1 */
		TM7705_WriteByte(REG_SETUP | WRITE | CH_1);
		TM7705_WriteByte(MD_CAL_SELF | __CH1_GAIN_BIPOLAR_BUF | FSYNC_0);
		TM7705_WaitDRDY();
	}

 	else if (_ch == 2) {
 		TM7705_WriteByte(REG_SETUP | WRITE | CH_2);
		TM7705_WriteByte(MD_CAL_SELF | __CH2_GAIN_BIPOLAR_BUF | FSYNC_0);
		TM7705_WaitDRDY();
	}
}

 void TM7705_SytemCalibZero(unsigned char _ch) {
	if (_ch == 1) {
 		TM7705_WriteByte(REG_SETUP | WRITE | CH_1);
		TM7705_WriteByte(MD_CAL_ZERO | __CH1_GAIN_BIPOLAR_BUF | FSYNC_0);
		TM7705_WaitDRDY();
	}
	else if (_ch == 2) {
 		TM7705_WriteByte(REG_SETUP | WRITE | CH_2);
		TM7705_WriteByte(MD_CAL_ZERO | __CH2_GAIN_BIPOLAR_BUF | FSYNC_0);
		TM7705_WaitDRDY();
	}
}

 void TM7705_SytemCalibFull(unsigned char _ch) {
	if (_ch == 1) {
 		TM7705_WriteByte(REG_SETUP | WRITE | CH_1);
		TM7705_WriteByte(MD_CAL_FULL | __CH1_GAIN_BIPOLAR_BUF | FSYNC_0);
		TM7705_WaitDRDY();
	}
	else if (_ch == 2) {
 		TM7705_WriteByte(REG_SETUP | WRITE | CH_2);
		TM7705_WriteByte(MD_CAL_FULL | __CH2_GAIN_BIPOLAR_BUF | FSYNC_0);
		TM7705_WaitDRDY();
	}
}

uint16_t TM7705_ReadAdc(unsigned char _ch) {
	unsigned char i;
	unsigned short read = 0;

 	for (i = 0; i < 3; i++) {
		TM7705_WaitDRDY();

		if (_ch == 1) {
			TM7705_WriteByte(0x38);
		}
		else if (_ch == 2) {
			TM7705_WriteByte(0x39);
		}

		read = TM7705_Read2Byte();
	}
	return read;
}

static float calTemp(float Rt, float offset) {

	const double RTD_A = 3.9083e-3;
	const double RTD_B = -5.775e-7;

	double Z1, Z2, Z3, Z4, temp;

	Z1 = -RTD_A;
	Z2 = RTD_A * RTD_A - (4.0 * RTD_B);
	Z3 = (4.0 * RTD_B) / 1000.0;
	Z4 = 2.0 * RTD_B;

	temp = Z2 + (Z3 * Rt);
	temp = (sqrt(temp) + Z1) / Z4;

	temp += (double)offset;

	return (float)temp;
}

float TM7705_GetTemp(uint8_t channel, float offset) {
	uint16_t refRes = 2000; // ref resistor for voltage divider
	float adcValue = (float)TM7705_ReadAdc(channel);
	float tempValue = calTemp( adcValue * (float)refRes / 65535.0, offset);
	round(tempValue * 100.0) / 100.0;
	return tempValue;
}

//float ConvertRtoT(float R) {
//    float A=3.9083 * pow(10,-3);
//    float B=5775 * pow(10,-7);
//    float T;
//
//    T = (0-A + sqrt(pow(A,2) - 4 * B * (1 - R/1000))) / 2 * B;
//
//    Serial.print(T);
//    Serial.print(";");
//    T=(0-A - sqrt(pow(A,2) - 4 * B * (1 - R/1000))) / 2 * B;
//    Serial.print(T);
//    return T;
//
//}
