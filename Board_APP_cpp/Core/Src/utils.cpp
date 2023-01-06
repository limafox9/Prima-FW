/*==============================================================================

Filename: utils.cpp

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


#include "utils.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "globals.h"
#include "RTC.h"

extern UART_HandleTypeDef huart1;


void Log(LOG_LEVEL logLevel, uint8_t logTimeStamp, const char* format, ...) {

	static uint32_t lastTickCount = HAL_GetTick();

	if (LOG_ACTIVE && logLevel >= LOG_LEVEL_ACTIVE) {
		RTCDateTime r;
		char* s = (char*)malloc(128 * sizeof(char));
		memset(s,'\0', 1);
		char buffer[256];

		if ((logTimeStamp & LOG_ELAPSED_RESET) == LOG_ELAPSED_RESET) {
			lastTickCount = HAL_GetTick();
		}
		if ((logTimeStamp & LOG_DATE) == LOG_DATE) {
			PCF8563_GetDateTime(&r);
			sprintf(buffer, "%02d-%02d-%02d ", r.YearShort, r.Month, r.Day);
			strcat(s, buffer);
		}
		if ((logTimeStamp & LOG_TIME) == LOG_TIME) {
			PCF8563_GetDateTime(&r);
			sprintf(buffer, "%02d:%02d:%02d ", r.Hour, r.Minute, r.Second);
			strcat(s, buffer);
		}
		if ((logTimeStamp & LOG_ELAPSED) == LOG_ELAPSED) {
			uint32_t currTickCount = HAL_GetTick();
			uint32_t elapsedTickCount = GetSafeDeltaTick(lastTickCount, HAL_GetTick());
			uint16_t sec = (elapsedTickCount / 1000) % 60;
			uint16_t min = (sec / 60) % 60;
			uint16_t hrs = min / 60;
			sprintf(buffer, "%3d:%02d:%02d ", hrs, min, sec);
			strcat(s, buffer);
			lastTickCount = currTickCount;
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

void PinWrite(gpioPin_s pin, bool status) {
	status ? HAL_GPIO_WritePin(pin.Port, pin.Pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(pin.Port, pin.Pin, GPIO_PIN_RESET);
}

bool PinRead(gpioPin_s pin) {
	bool status;
	GPIO_PinState r = HAL_GPIO_ReadPin(pin.Port, pin.Pin);
	r == GPIO_PIN_SET ? status = true : status = false;
	return status;
}

void PinToggle(gpioPin_s pin, uint8_t status) {
	HAL_GPIO_TogglePin(pin.Port, pin.Pin);
}

uint32_t GetSafeDeltaTick(uint32_t startTick, uint32_t endTick) {

	uint32_t deltaTick;

	// In case of HAL_GetTick() overflow (~49 days). Counter restarts from 0.
	if (endTick < startTick) {
		deltaTick = MAX_VALUE(startTick) - startTick + endTick;
	} else {
		deltaTick = endTick - startTick;
	}

	return deltaTick;
}



