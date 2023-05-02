/*==============================================================================

Filename: time.h

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


#ifndef INC_TIME_H_
#define INC_TIME_H_

#include "PCF8563.h"
#include <stdint.h>
#include "main.h"


typedef struct RTCTimerTime_s {
	uint16_t	TimeHour;
	uint16_t	Hours;	// Total hours elapsed
	uint32_t	Minutes;	// Total minutes elapsed
	uint32_t	Seconds;	// Total seconds elapsed
	uint8_t		TimeMinute;	// Minutes in 0..59 format
	uint8_t		TimeSecond;	// Seconds in 0..59 format
	uint32_t	Ticks;		// Total Ticks elapsed
} RTCTimerTime_s;

typedef enum {
	TIMER_POWER_ON,
	TIMER_LOG,
	TIMER_CYCLE,
	TIMER_PHASE,
	TIMER_DISPLAYINFO,
	TIMER_FAN,
	TIMER_DOOR,
	TIMER_TIMEOUT,
	TIMER_NIGHT_CYCLE,
	TIMER_SENSOR_MONITOR,
	TIMER_CHECK_ERROR,
	TIMER_1,
	TIMER_2,
	TIMER_3,
	TIMER_4,
	TIMER_5,
} RTCTimer_e;

extern uint16_t yearSet;

void ClockInit(I2C_HandleTypeDef* hi2c);
void SetDateTime(RTCDateTime_s* dateTime);
void GetDateTime(RTCDateTime_s* dateTime);
void GetTimer(uint8_t timer, RTCTimerTime_s* timerTime);
void ResetTimer(uint8_t timer);

//uint32_t GetSafeDeltaTick(uint32_t starTick, uint32_t endTick);


#endif /* INC_TIME_H_ */
