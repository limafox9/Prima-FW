/*==============================================================================

Filename: program_steps.c

Created on: 17 gen 2023

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

#include "program_steps.h"
#include "time.h"
#include "sensors.h"
#include "actuators.h"
#include <stdio.h>

uint8_t chamberHeaterTempSet;
uint8_t steamGeneratorTempSet;

extern s_Sensors Sensors;

uint8_t Step_Preheat(uint8_t chamberTemp, uint8_t steamGenTemp, uint16_t timeout) {

	uint8_t retVal = 0;
	RTCTimerTime_s t;
	ResetTimer(1);
	FanSet(ON);
	while (1) {
		Sensors.TemperatureChamberHeater < chamberTemp ? HeaterChamberSet(ON) : HeaterChamberSet(OFF);
		Sensors.TemperatureSteamGenerator < steamGenTemp ? HeaterSteamGeneratorSet(ON) : HeaterSteamGeneratorSet(OFF);
		if ((Sensors.TemperatureChamberHeater >= chamberTemp) && (Sensors.TemperatureSteamGenerator >= steamGenTemp)) {
			retVal = 1;
			break;
		}
		GetTimer(1, &t);
		if (t.Seconds > timeout) {
			retVal = 0;
			break;
		}
		osDelay(500);
	}

	HeaterChamberSet(OFF);
	HeaterSteamGeneratorSet(OFF);
	FanSet(OFF);
}



