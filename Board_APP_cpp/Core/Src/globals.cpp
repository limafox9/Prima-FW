/*==============================================================================

Filename: globals.cpp

Created on: Jan 6, 2023

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


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "globals.h"
#include "class_RTClock.h"
#include "class_machine.h"


extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;
extern machine_c M;

machine_c M;


void PinWrite(GPIOPin_s pin, bool status) {
	status ? HAL_GPIO_WritePin(pin.Port, pin.Pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(pin.Port, pin.Pin, GPIO_PIN_RESET);
}

bool PinRead(GPIOPin_s pin) {
	bool status;
	GPIO_PinState r = HAL_GPIO_ReadPin(pin.Port, pin.Pin);
	r == GPIO_PIN_SET ? status = true : status = false;
	return status;
}

void PinToggle(GPIOPin_s pin, uint8_t status) {
	HAL_GPIO_TogglePin(pin.Port, pin.Pin);
}

void Log(LOG_LEVEL logLevel, uint8_t logTimeStamp, const char* format, ...) {

	if (LOG_ACTIVE && logLevel >= LOG_LEVEL_ACTIVE) {
		RTCDateTime_s r;
		RTCTimerTime_s t;
		char* s = (char*)malloc(128 * sizeof(char));
		memset(s,'\0', 1);
		char buffer[256];

		if ((logTimeStamp & LOG_ELAPSED_RESET) == LOG_ELAPSED_RESET) {
			M.RTClock.ResetTimer(1);
		}
		if ((logTimeStamp & LOG_DATE) == LOG_DATE) {
			M.RTClock.GetDateTime(&r);
			sprintf(buffer, "%02d-%02d-%02d ", r.Year, r.Month, r.Day);
			strcat(s, buffer);
		}
		if ((logTimeStamp & LOG_TIME) == LOG_TIME) {
			M.RTClock.GetDateTime(&r);
			sprintf(buffer, "%02d:%02d:%02d ", r.Hour, r.Minute, r.Second);
			strcat(s, buffer);
		}
		if ((logTimeStamp & LOG_ELAPSED) == LOG_ELAPSED) {
			M.RTClock.GetTimer(1, &t);
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

		free(s);
	}
}

//void HAL_I2C_SlaveRxCpltCallback (I2C_HandleTypeDef * hi2c) {
//	if (hi2c->Instance == hi2c1.Instance) {
//		PCF8563_CalculateDateTime(&r1);
//		Log(LOG_LEVEL_RELEASE, "Date/Time: %02d-%02d-%02d %02d:%02d:%02d", r1.Year, r1.Month, r1.Day, r1.Hour, r1.Minute, r1.Second);
//
//	}
//
//	PCF8563_ReceiveDateTimeDMA();
//}
