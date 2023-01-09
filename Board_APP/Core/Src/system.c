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


extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart3_rx;

uint32_t UID[3] = {0};
uint16_t machineSerial;

osThreadId keysTaskHandle;
osThreadId ledsTaskHandle;
osThreadId sensorsTaskHandle;

uint8_t UART_rxBuffer[UART_DISPLAY_BUFFER_SIZE] = {0};

RTCDateTime_s dt;

void SysInit() {

	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "\n\n****************************************");
	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "PRIMA Board APP v%d.%d.%d", FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION);

	// Get MCU unique ID 96 bit
	UID[0] = *(uint32_t*)(UID_BASE );
	UID[1] = *(uint32_t*)(UID_BASE + 0x04);
	UID[2] = *(uint32_t*)(UID_BASE + 0x08);
	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "UID:    %08X %08X %08X", UID[0], UID[1], UID[2]);

	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "SERIAL: %08X %08X %08X", UID[0], UID[1], UID[2]);

	// Clock initialization and get system date/time
	ClockInit(&hi2c1);
	GetDateTime(&dt);
	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "TIME:   %02d-%02d-%02d %02d:%02d\n", dt.Year, dt.Month, dt.Day, dt.Hour, dt.Minute);

	Log(LOG_LEVEL_DEBUG, LOG_TIME_NONE, LOG_OUT_SERIAL, "Kernel Started");

	// Set actuators in safe (disconnected) mode
	ActuatorsSafeStatus();
	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "Actuators initialized in safe mode");

	// Create threads
	osThreadDef(keysTask, StartKeysTask, osPriorityNormal, 1, 128);
	keysTaskHandle = osThreadCreate(osThread(keysTask), NULL);

	osThreadDef(ledsTask, StartLedsTask, osPriorityNormal, 1, 128);
	ledsTaskHandle = osThreadCreate(osThread(ledsTask), NULL);

	osThreadDef(sensorsTask, StartSensorsTask, osPriorityNormal, 1, 1024);
	sensorsTaskHandle = osThreadCreate(osThread(sensorsTask), NULL);

	Log(LOG_LEVEL_DEBUG, LOG_TIME_NONE, LOG_OUT_SERIAL, "Threads started");

	// Display communication init and Start receiving DMA display UART
	PanelSerialInit(&huart3);
	HAL_UART_Receive_DMA(&huart3, UART_rxBuffer, UART_DISPLAY_BUFFER_SIZE);
	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);

	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "System initialized\n");

	Beep(3, 100, 100);
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

		HAL_UART_Transmit(&huart1, (uint8_t*)s, strlen(s), 100);
		HAL_UART_Transmit(&huart1, (uint8_t*)"\n", 1, 100);
	}
}

// Display DMA UART callback for receiving data
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == huart3.Instance) {
		PanelSerialReceive(UART_rxBuffer, UART_DISPLAY_BUFFER_SIZE);
	}

	HAL_UART_Receive_DMA(huart, UART_rxBuffer, UART_DISPLAY_BUFFER_SIZE);
	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
}

