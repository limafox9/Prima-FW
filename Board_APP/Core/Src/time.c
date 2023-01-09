/*==============================================================================

Filename: time.c

Created on: Jan 7, 2023

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


#include "PCF8563.h"
#include "time.h"


uint32_t timers[8] = {0};

static uint32_t GetSafeDeltaTick(uint32_t startTick, uint32_t endTick);


void ClockInit(I2C_HandleTypeDef* hi2c) {

	PCF8563_Init(hi2c);

	for (int i = 0; i < 8; i++) {
		ResetTimer(i);
	}
}

void GetDateTime(RTCDateTime_s* dateTime) {

	PCF8563_GetDateTime(dateTime);
}

void SetDateTime(RTCDateTime_s* dateTime) {

	PCF8563_SetDateTime(dateTime);
}

void GetTimer(uint8_t timer, RTCTimerTime_s* timerTime) {

	timerTime->Ticks = GetSafeDeltaTick(timers[timer], HAL_GetTick());
	timerTime->Seconds = timerTime->Ticks / 1000;
	timerTime->TimeSecond = (timerTime->Ticks / 1000) % 60;
	timerTime->TimeMinute = (timerTime->Ticks / 1000 / 60) % 60;
	timerTime->TimeHour = timerTime->Ticks / 1000 / 60 / 60;
}

void ResetTimer(uint8_t timer) {

	timers[timer] = HAL_GetTick();
}

uint32_t GetSafeDeltaTick(uint32_t startTick, uint32_t endTick) {

	uint32_t deltaTick;

	// In case of HAL_GetTick() overflow (~49 days). Counter restarts from 0.
	if (endTick < startTick) {
		deltaTick = UINT32_MAX - startTick + endTick;
	} else {
		deltaTick = endTick - startTick;
	}

	return deltaTick;
}
