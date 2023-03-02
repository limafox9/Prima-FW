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

/*
	TM7705 驱动模块
*/

/*
	接线方法： STC89C52开发板，请使用杜邦线连接。

    TM7705模块   STC89C52开发板
      VCC   ------  5.0V (3.3V)
      GND   ------  GND
      CS    ------  P1.0
      RST   ------  P1.1
      DIN   ------  P1.2
	  SCK   ------  P1.3
      DOUT  ------  P1.4
      DRDY  ------  P1.5
*/

//#include "delay.h"
//#include <reg52.h>
#include <stdio.h>
#include "tm7705.h"
#include "stm32f1xx_hal.h"
#include "main.h"

#define SOFT_SPI // C51 IO 口模拟 SPI 接口

/* 通道1和通道2的增益,输入缓冲，极性 */
#define __CH1_GAIN_BIPOLAR_BUF	(GAIN_1 | UNIPOLAR | BUF_EN)
#define __CH2_GAIN_BIPOLAR_BUF	(GAIN_1 | UNIPOLAR | BUF_EN)

#ifdef SOFT_SPI		/* 软件SPI */

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

	/* 定义GPIO端口 */
//	sbit CS     = P1^0;
//	sbit RESET  = P1^1;
//	sbit DIN    = P1^2;
//	sbit SCK    = P1^3;
//	sbit DOUT   = P1^4;
//	sbit DRDY	= P1^5;
//
//	/* 定义口线置0和置1的宏 */
//	#define CS_0()		CS = 0
//	#define CS_1()		CS = 1
//
//	#define RESET_0()	RESET = 0
//	#define RESET_1()	RESET = 1
//
//	#define DI_0()		DIN = 0
//	#define DI_1()		DIN = 1
//
//	#define SCK_0()		SCK = 0
//	#define SCK_1()		SCK = 1
//
//	#define DO_IS_HIGH()	(DOUT != 0)
//
//	#define DRDY_IS_LOW()	(DRDY == 0)
#endif

/* Communication register bit definition */
enum {
	/* register selection RS2 RS1 RS0  */
	REG_COMM	= 0x00,	/* 通信寄存器 */
	REG_SETUP	= 0x10,	/* 设置寄存器 */
	REG_CLOCK	= 0x20,	/* 时钟寄存器 */
	REG_DATA	= 0x30,	/* 数据寄存器 */
	REG_ZERO_CH1	= 0x60,	/* CH1 偏移寄存器 */
	REG_FULL_CH1	= 0x70,	/* CH1 满量程寄存器 */
	REG_ZERO_CH2	= 0x61,	/* CH2 偏移寄存器 */
	REG_FULL_CH2	= 0x71,	/* CH2 满量程寄存器 */

	/* 读写操作 */
	WRITE 		= 0x00,	/* 写操作 */
	READ 		= 0x08,	/* 读操作 */

	/* 通道 */
	CH_1		= 0,	/* AIN1+  AIN1- */
	CH_2		= 1,	/* AIN2+  AIN2- */
	CH_3		= 2,	/* AIN1-  AIN1- */
	CH_4		= 3		/* AIN1-  AIN2- */
};

/* 设置寄存器bit定义 */
enum {
	MD_NORMAL		= (0 << 6),	/* 正常模式 */
	MD_CAL_SELF		= (1 << 6),	/* 自校准模式 */
	MD_CAL_ZERO		= (2 << 6),	/* 校准0刻度模式 */
	MD_CAL_FULL		= (3 << 6),	/* 校准满刻度模式 */

	GAIN_1			= (0 << 3),	/* 增益 */
	GAIN_2			= (1 << 3),	/* 增益 */
	GAIN_4			= (2 << 3),	/* 增益 */
	GAIN_8			= (3 << 3),	/* 增益 */
	GAIN_16			= (4 << 3),	/* 增益 */
	GAIN_32			= (5 << 3),	/* 增益 */
	GAIN_64			= (6 << 3),	/* 增益 */
	GAIN_128		= (7 << 3),	/* 增益 */

	/* 无论双极性还是单极性都不改变任何输入信号的状态，它只改变输出数据的代码和转换函数上的校准点 */
	BIPOLAR			= (0 << 2),	/* 双极性输入 */
	UNIPOLAR		= (1 << 2),	/* 单极性输入 */

	BUF_NO			= (0 << 1),	/* 输入无缓冲（内部缓冲器不启用) */
	BUF_EN			= (1 << 1),	/* 输入有缓冲 (启用内部缓冲器) */

	FSYNC_0			= 0,
	FSYNC_1			= 1		/* 不启用 */
};


/* 时钟寄存器bit定义 */
enum {
	CLKDIS_0	= 0x00,		/* 时钟输出使能 （当外接晶振时，必须使能才能振荡） */
	CLKDIS_1	= 0x10,		/* 时钟禁止 （当外部提供时钟时，设置该位可以禁止MCK_OUT引脚输出时钟以省电 */

	/*
		2.4576MHz（CLKDIV=0 ）或为 4.9152MHz （CLKDIV=1 ），CLK 应置 “0”。
		1MHz （CLKDIV=0 ）或 2MHz   （CLKDIV=1 ），CLK 该位应置  “1”
	*/
	CLK_4_9152M = 0x08,
	CLK_2_4576M = 0x00,
	CLK_1M 		= 0x04,
	CLK_2M 		= 0x0C,

	FS_50HZ		= 0x00,
	FS_60HZ		= 0x01,
	FS_250HZ	= 0x02,
	FS_500HZ	= 0x04,

	/*
		四十九、电子秤应用中提高TM7705 精度的方法
			当使用主时钟为 2.4576MHz 时，强烈建议将时钟寄存器设为 84H,此时数据输出更新率为10Hz,即每0.1S 输出一个新数据。
			当使用主时钟为 1MHz 时，强烈建议将时钟寄存器设为80H, 此时数据输出更新率为4Hz, 即每0.25S 输出一个新数据
	*/
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

//初始化
void TM7705_Init() {
	TM7705_ResetHard();		/* 硬件复位 */
	HAL_Delay(5);

	TM7705_SyncSPI();		/* 同步SPI接口时序 */
	HAL_Delay(5);

	/* 配置时钟寄存器 */
	/* 先写通信寄存器，下一步是写时钟寄存器 */
	TM7705_WriteByte(REG_CLOCK | WRITE | CH_1);
	/* 使能外部晶振，刷新速率50Hz */
	TM7705_WriteByte(CLKDIS_0 | CLK_4_9152M | FS_50HZ);
	//TM7705_WriteByte(CLKDIS_0 | CLK_4_9152M | FS_500HZ);	/* 刷新速率500Hz */

	/* 每次上电进行一次自校准 */
	TM7705_CalibSelf(1);	/* 内部自校准 CH1 */
	//TM7705_CalibSelf(2);	/* 内部自校准 CH2 */
	HAL_Delay(5);
}

//硬件复位
static void TM7705_ResetHard() {
	RESET_1;
	HAL_Delay(1);
	RESET_0;
	HAL_Delay(2);
	RESET_1;
	HAL_Delay(1);
}

//同步SPI时序
static void TM7705_SyncSPI() {
	/* AD7705串行接口失步后将其复位。复位后要延时500us再访问 */
	CS_0;
	TM7705_Send8Bit(0xFF);
	TM7705_Send8Bit(0xFF);
	TM7705_Send8Bit(0xFF);
	TM7705_Send8Bit(0xFF);
	CS_1;
}

//发送8个bit数据，不带片选CS
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

//接收8个bit数据，不带片选CS
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

//发送1个byte数据，带片选CS
static void TM7705_WriteByte(unsigned char _data) {
	CS_0;
	TM7705_Send8Bit(_data);
	CS_1;
}

//发送3个byte数据，带片选CS
static void TM7705_Write3Byte(unsigned long _data)
{
	CS_0;
	TM7705_Send8Bit((_data >> 16) & 0xFF);
	TM7705_Send8Bit((_data >> 8) & 0xFF);
	TM7705_Send8Bit(_data);
	CS_1;
}


// 功能说明: 从AD芯片读取一个字（8位）
static unsigned char TM7705_ReadByte() {
	unsigned char read;

	CS_0;
	read = TM7705_Recive8Bit();
	CS_1;

	return read;
}

// 功能说明: 读2字节数据
static unsigned short TM7705_Read2Byte() {
	unsigned short read;

	CS_0;
	read = TM7705_Recive8Bit();
	read <<= 8;
	read += TM7705_Recive8Bit();
	CS_1;

	return read;
}

// 功能说明: 读3字节数据
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
		//printf("TM7705_WaitDRDY() 芯片应答超时 ...\r\n"); /* 调试语句. 用语排错 */
		//printf("重新同步SPI接口时序\r\n");	/* 调试语句. 用语排错 */
		//printf("restart SPI bus\r\n");
		TM7705_SyncSPI();  /* 同步SPI接口时序 */
		HAL_Delay(5);
	}
}

/*
*********************************************************************************************************
*	功能说明: 写指定的寄存器
*	形    参:  _RegID : 寄存器ID
*			  _RegValue : 寄存器值。 对于8位的寄存器，取32位形参的低8bit
*	返 回 值: 无
*********************************************************************************************************
*/
void TM7705_WriteReg(unsigned char _RegID, unsigned long _RegValue) {
	unsigned char bits;

	switch (_RegID) {
		case REG_COMM:		/* 通信寄存器 */
		case REG_SETUP:		/* 设置寄存器 8bit */
		case REG_CLOCK:		/* 时钟寄存器 8bit */
			bits = 8;
			break;

		case REG_ZERO_CH1:	/* CH1 偏移寄存器 24bit */
		case REG_FULL_CH1:	/* CH1 满量程寄存器 24bit */
		case REG_ZERO_CH2:	/* CH2 偏移寄存器 24bit */
		case REG_FULL_CH2:	/* CH2 满量程寄存器 24bit*/
			bits = 24;
			break;

		case REG_DATA:		/* 数据寄存器 16bit */
		default:
			return;
	}

	TM7705_WriteByte(_RegID | WRITE);	/* 写通信寄存器, 指定下一步是写操作，并指定写哪个寄存器 */

	if (bits == 8) {
		TM7705_WriteByte((unsigned char)_RegValue);
	}
	else { //24 bits
		TM7705_Write3Byte(_RegValue);
	}
}

/*
*********************************************************************************************************
*	功能说明: 读指定的寄存器
*	形    参:  _RegID : 寄存器ID
*			  _RegValue : 寄存器值。 对于8位的寄存器，取32位形参的低8bit
*	返 回 值: 读到的寄存器值。 对于8位的寄存器，取32位形参的低8bit
*********************************************************************************************************
*/
unsigned long TM7705_ReadReg(unsigned char _RegID) {
	unsigned char bits;
	unsigned long read;

	switch (_RegID) {
		case REG_COMM:		/* 通信寄存器 */
		case REG_SETUP:		/* 设置寄存器 8bit */
		case REG_CLOCK:		/* 时钟寄存器 8bit */
			bits = 8;
			break;

		case REG_ZERO_CH1:	/* CH1 偏移寄存器 24bit */
		case REG_FULL_CH1:	/* CH1 满量程寄存器 24bit */
		case REG_ZERO_CH2:	/* CH2 偏移寄存器 24bit */
		case REG_FULL_CH2:	/* CH2 满量程寄存器 24bit*/
			bits = 24;
			break;

		case REG_DATA:		/* 数据寄存器 16bit */
		default:
			return 0xFFFFFFFF;
	}

	TM7705_WriteByte(_RegID | READ);	/* 写通信寄存器, 指定下一步是写操作，并指定写哪个寄存器 */

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

/*
*********************************************************************************************************
*	函 数 名: TM7705_CalibSelf
*	功能说明: 启动自校准. 内部自动短接AIN+ AIN-校准0位，内部短接到Vref 校准满位。此函数执行过程较长，
*			  实测约 180ms
*	形    参:  _ch : ADC通道，1或2
*	返 回 值: 无
*********************************************************************************************************
*/
void TM7705_CalibSelf(unsigned char _ch) {
	if (_ch == 1) {
		/* 自校准CH1 */
		TM7705_WriteByte(REG_SETUP | WRITE | CH_1);	/* 写通信寄存器，下一步是写设置寄存器，通道1 */
		TM7705_WriteByte(MD_CAL_SELF | __CH1_GAIN_BIPOLAR_BUF | FSYNC_0);/* 启动自校准 */
		TM7705_WaitDRDY();	/* 等待内部操作完成 --- 时间较长，约180ms */
	}

 	else if (_ch == 2) {
		/* 自校准CH2 */
		TM7705_WriteByte(REG_SETUP | WRITE | CH_2);	/* 写通信寄存器，下一步是写设置寄存器，通道2 */
		TM7705_WriteByte(MD_CAL_SELF | __CH2_GAIN_BIPOLAR_BUF | FSYNC_0);	/* 启动自校准 */
		TM7705_WaitDRDY();	/* 等待内部操作完成  --- 时间较长，约180ms */
	}
}

/*
*********************************************************************************************************
*	函 数 名: TM7705_SytemCalibZero
*	功能说明: 启动系统校准零位. 请将AIN+ AIN-短接后，执行该函数。校准应该由主程序控制并保存校准参数。
*			 执行完毕后。可以通过 TM7705_ReadReg(REG_ZERO_CH1) 和  TM7705_ReadReg(REG_ZERO_CH2) 读取校准参数。
*	形    参: _ch : ADC通道，1或2
*	返 回 值: 无
*********************************************************************************************************
*/
void TM7705_SytemCalibZero(unsigned char _ch) {
	if (_ch == 1) {
		/* 校准CH1 */
		TM7705_WriteByte(REG_SETUP | WRITE | CH_1);	/* 写通信寄存器，下一步是写设置寄存器，通道1 */
		TM7705_WriteByte(MD_CAL_ZERO | __CH1_GAIN_BIPOLAR_BUF | FSYNC_0);/* 启动自校准 */
		TM7705_WaitDRDY();	/* 等待内部操作完成 */
	}
	else if (_ch == 2) {
		/* 校准CH2 */
		TM7705_WriteByte(REG_SETUP | WRITE | CH_2);	/* 写通信寄存器，下一步是写设置寄存器，通道2 */
		TM7705_WriteByte(MD_CAL_ZERO | __CH2_GAIN_BIPOLAR_BUF | FSYNC_0);	/* 启动自校准 */
		TM7705_WaitDRDY();	/* 等待内部操作完成 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: TM7705_SytemCalibFull
*	功能说明: 启动系统校准满位. 请将AIN+ AIN-接最大输入电压源，执行该函数。校准应该由主程序控制并保存校准参数。
*			 执行完毕后。可以通过 TM7705_ReadReg(REG_FULL_CH1) 和  TM7705_ReadReg(REG_FULL_CH2) 读取校准参数。
*	形    参:  _ch : ADC通道，1或2
*	返 回 值: 无
*********************************************************************************************************
*/
void TM7705_SytemCalibFull(unsigned char _ch) {
	if (_ch == 1) {
		/* 校准CH1 */
		TM7705_WriteByte(REG_SETUP | WRITE | CH_1);	/* 写通信寄存器，下一步是写设置寄存器，通道1 */
		TM7705_WriteByte(MD_CAL_FULL | __CH1_GAIN_BIPOLAR_BUF | FSYNC_0);/* 启动自校准 */
		TM7705_WaitDRDY();	/* 等待内部操作完成 */
	}
	else if (_ch == 2) {
		/* 校准CH2 */
		TM7705_WriteByte(REG_SETUP | WRITE | CH_2);	/* 写通信寄存器，下一步是写设置寄存器，通道2 */
		TM7705_WriteByte(MD_CAL_FULL | __CH2_GAIN_BIPOLAR_BUF | FSYNC_0);	/* 启动自校准 */
		TM7705_WaitDRDY();	/* 等待内部操作完成 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: TM7705_ReadAdc
*	功能说明: 读通道1或2的ADC数据
*	形    参:  _ch : ADC通道，1或2
*	返 回 值: 0 ~ 65535
*********************************************************************************************************
*/
unsigned short TM7705_ReadAdc(unsigned char _ch) {
	unsigned char i;
	unsigned short read = 0;

	/* 为了避免通道切换造成读数失效，读3次 */
	for (i = 0; i < 3; i++) {
		TM7705_WaitDRDY();		/* 等待DRDY口线为0 */

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

float TM7705_GetTemp(uint8_t channel, float offset) {
	uint16_t refRes = 2000; // ref resistor for voltage divider
	return calTemp((float)TM7705_ReadAdc(channel) * (float)refRes / 65535.0, offset);
}

