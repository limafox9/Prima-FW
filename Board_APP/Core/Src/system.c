/*==============================================================================

Filename: system.c

Created on: Dec 17, 2022

Original work: Copyright (c) 2022 Luca Favenzi
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

#include "system.h"

#include <stdio.h>
#include <stdarg.h>

#include "globals.h"
#include "cmsis_os.h"
#include "settings.h"
#include "time.h"
#include "actuators.h"
#include "keys.h"
#include "leds.h"
#include "sensors.h"
#include "serial.h"
#include "eeprom.h"
#include "TM7705.h"
#include "BAT2U.h"
#include "BMP280.h"

#define UART_MONITOR huart1
#define UART_DISPLAY huart3

#define RTD_A 3.9083e-3
#define RTD_B -5.775e-7

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern SPI_HandleTypeDef hspi1;

uint32_t UID[3] = {0};
uint16_t machineSerial;

uint8_t UART_rxBuffer[UART_DISPLAY_BUFFER_SIZE] = {0};

RTCDateTime_s dt;

float calTemp(float Rt) {
	float Z1, Z2, Z3, Z4, temp;

	Z1 = -RTD_A;
	Z2 = RTD_A * RTD_A - (4 * RTD_B);
	Z3 = (4 * RTD_B) / 1000;
	Z4 = 2 * RTD_B;

	temp = Z2 + (Z3 * Rt);
	temp = (sqrt(temp) + Z1) / Z4;

	return temp;
}

void SysInit() {

	SerialInit(&UART_MONITOR, 115200);
	SerialInit(&UART_DISPLAY, 115200);


	BMP280_HandleTypedef bmp280;
	float pressure, temperature, humidity;
	uint16_t size;
	uint8_t Data[256];

	bmp280_init_default_params(&bmp280.params);
	bmp280.addr = BMP280_I2C_ADDRESS_0;
//	bmp280.i2c = &hi2c2;

	while (!bmp280_init(&bmp280, &bmp280.params)) {
			size = sprintf((char *)Data, "BMP280 initialization failed\n");
			SerialSend(&UART_MONITOR, Data, size);
			HAL_Delay(2000);
	}
	bool bme280p = bmp280.id == BME280_CHIP_ID;
	size = sprintf((char *)Data, "BMP280: found %s\n", bme280p ? "BME280" : "BMP280");
	SerialSend(&UART_MONITOR, Data, size);




	char s[32];
	uint16_t refRes = WaterTDSGet();
	sprintf(s,"%d\n", refRes);
	SerialSend(&UART_MONITOR, s, 32);


//
//	TM7705_Init(&hspi1);
//
//	Beep(2, 100, 100);
//
//	uint16_t val0, val1;
//
//	char s[32];
//	double res;
//	uint16_t refRes = 2000;
//	float tmp0, tmp1;
//
//	while (1) {
//		val0 = TM7705_GetValue(0);
////		val1 = TM7705_GetValue(CHANNEL_1);
//
//		res = ((double)val0) * refRes / 65535;
//		tmp0 = calTemp(res);
//
////		res = ((double)val1) * refRes / 65535;
////		tmp1 = calTemp(res);
//
//		sprintf(s,"%d - %0.2f - %0.2f\n", val0, tmp0, tmp0);
//		SerialSend(&UART_MONITOR, s, 32);
//		osDelay(500);
//	}

	//	SerialSend(&UART_MONITOR, "MAMMA\n", 6);

//
	//	uint16_t n = 0x5678;
	////	char a[6] = "ABCDEF";
	//	EEPROM_Write(2,0,&n,2);
//	EEPROM_Write(page, offset, data, size)3,0,a,4);
//
//	EEPROM_EraseAll();










	//===============================================================================
//	uint8_t n[32]= {[0 ... 31] = 0x08};
//	EEPROM_Write(2,&n,21);
//	float u = 3.14667;
//	EEPROM_Write_Float(12, u);
//
//
//	uint16_t pp = 0x4523;
//	EEPROM_Write(255,&pp,2);
//
//	SerialSend(&UART_MONITOR, "\n", 1);
//
//	EEPROM_EraseAll();
//
//	char s[10];
//	char r[8];
//	for (int p = 0; p<32; p++) {
//		EEPROM_Read(p*8,r,8);
//		for (int b = 0; b<8; b++) {
//			sprintf(s,"%02X ", r[b]);
//			SerialSend(&UART_MONITOR, s, 3);
//		}
//		SerialSend(&UART_MONITOR, "\n", 1);
//	}
//
//	char z[10];
//	float y;
//	sprintf(z,"%0.6f\n", EEPROM_Read_Float(12));
//	SerialSend(&UART_MONITOR, z, 10);
	//===============================================================================








//	EEPROM_Read(10, &r, 2);
//	sprintf(s,"%2X \n", r);
//	SerialSend(&UART_MONITOR, s, 4);

//	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "\n\n****************************************");
//	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "PRIMA Board APP v%d.%d.%d", FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION);
//
//	// Get MCU unique ID 96 bit
//	UID[0] = *(uint32_t*)(UID_BASE );
//	UID[1] = *(uint32_t*)(UID_BASE + 0x04);
//	UID[2] = *(uint32_t*)(UID_BASE + 0x08);
//	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "UID:    %08X %08X %08X", UID[0], UID[1], UID[2]);
//
//	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "SERIAL: %08X %08X %08X", UID[0], UID[1], UID[2]);
//
//	// Clock initialization and get system date/time
//	ClockInit(&hi2c1);
//	GetDateTime(&dt);
//	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "TIME:   %02d-%02d-%02d %02d:%02d\n", dt.Year, dt.Month, dt.Day, dt.Hour, dt.Minute);
//
//	Log(LOG_LEVEL_DEBUG, LOG_TIME_NONE, LOG_OUT_SERIAL, "Kernel Started");
//
//	// Set actuators in safe (disconnected) mode
//	ActuatorsSafeStatus();
//	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "Actuators initialized in safe mode");
//
//	// Create threads
//
//	Log(LOG_LEVEL_DEBUG, LOG_TIME_NONE, LOG_OUT_SERIAL, "Threads started");
//
//	// Display communication init and Start receiving DMA display UART
////	PanelSerialInit(&huart3);
////	HAL_UART_Receive_DMA(&huart3, UART_rxBuffer, UART_DISPLAY_BUFFER_SIZE);
////	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
//
//	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "System initialized\n");

}

void Beep(uint8_t hits, uint16_t onDelay, uint16_t offDelay) {
	for (uint8_t i = 0; i < hits; i++) {
		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, ON);
		osDelay(onDelay);
		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, OFF);
		osDelay(onDelay);
	}
}

void Log(e_LogLevel logLevel, e_LogTimeStamp logTimeStamp, e_LogOutput logOutput, const char* format, ...) {

	if (*(int8_t*)GetSetting("LOG_ACTIVE") && logLevel >= *(int8_t*)GetSetting("LOG_ACTIVE_LEVEL")) {
		RTCDateTime_s r;
		RTCTimerTime_s t;
		char s[256] = {'\0'};
		char buffer[256];

		if ((logTimeStamp & LOG_TIME_ELAPSED_RESET) == LOG_TIME_ELAPSED_RESET) {
			ResetTimer(1);
		}
		if ((logTimeStamp & LOG_TIME_DATE) == LOG_TIME_DATE) {
			GetDateTime(&r);
			sprintf(buffer, "%02d-%02d-%02d ", r.Year, r.Month, r.Day);
			strcat(s, buffer);
		}
		if ((logTimeStamp & LOG_TIME_TIME) == LOG_TIME_TIME) {
			GetDateTime(&r);
			sprintf(buffer, "%02d:%02d:%02d ", r.Hour, r.Minute, r.Second);
			strcat(s, buffer);
		}
		if ((logTimeStamp & LOG_TIME_ELAPSED) == LOG_TIME_ELAPSED) {
			GetTimer(1, &t);
			sprintf(buffer, "%3d:%02d:%02d ", t.TimeHour, t.TimeMinute, t.TimeSecond);
			strcat(s, buffer);
		}

		va_list args;
		va_start (args, format);
		vsprintf (buffer,format, args);
		va_end (args);
		strcat(s, buffer);

//		HAL_UART_Transmit(&huart1, (uint8_t*)s, strlen(s), 100);
//		HAL_UART_Transmit(&huart1, (uint8_t*)"\n", 1, 100);

	}
}


