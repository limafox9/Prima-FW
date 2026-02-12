/*==============================================================================

Filename: program_steps.h

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



#ifndef INC_PHASES_H_
#define INC_PHASES_H_

#include <stdio.h>
#include "time.h"

typedef struct SterilPID_s {
	float PID_KP;
	float PID_KI;
	float PID_KD;
	float PID_TAU;
	float PID_LIM_MIN;
	float PID_LIM_MAX;
	float PID_LIM_MIN_INT;
	float PID_LIM_MAX_INT;
	float SAMPLE_TIME_S;
} SterilPID_s;

typedef struct SterilParam_s {
	float chamberTemp;
	float chamberTempMin;
	float chamberTempMax;
	uint16_t sterilTime;
	float chamberPress;
	float pressMin;
	float pressMax;
	float steamTemp;
	float steamTempMin;
	float T1T2MaxDiff;
	uint16_t settleTime;
	uint16_t printInterval;
	uint16_t stabilizationPeriod;
	uint16_t timeout;
} SterilParam_s;

extern char currPhase[8];
extern uint8_t sterilRunning;

uint8_t PhaseBegin(uint8_t timeout);
uint8_t PhaseResume(uint16_t timeout);
uint8_t PhasePreheat(float chamberTemp, float steamGenTemp);
uint8_t PhaseHeatUp(float ringTemp, float steamGenTemp, float chamberTemp, float chamberPress, uint16_t timeout);
uint8_t PhaseVacuum(uint8_t nVacuum, float press, uint8_t skipThreshold, uint16_t timeout);
uint8_t PhaseVacuumHold(float vacPress, uint8_t tolerance, uint16_t time, uint16_t timeout);
uint8_t PhasePressure(uint8_t nPressure, float press, float steamTemp, float steamTempMin, uint16_t timeout);
uint8_t PhasePressureTest(float steamTemp, float steamTempMin);
uint8_t PhaseRelease(uint8_t nRelease, uint16_t timeout);
uint8_t PhaseSteril(SterilParam_s* param, SterilPID_s* pPID);
uint8_t PhaseDry(float firstVacPress, float otherVacPress, uint16_t time, uint16_t timeout);
uint8_t PhaseFinish(uint8_t success, RTCDateTime_s* time, RTCTimerTime_s* timer, uint16_t timeout);
uint8_t PhaseWait(float vacPress, uint8_t tolerance, uint16_t time);
uint8_t PhaseWaterLoad(uint16_t timeout);
void PhaseTestPID(SterilParam_s* param);

#endif /* INC_PHASES_H_ */
