/*==============================================================================

Filename: phases.c

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

#include "phases.h"
#include "time.h"
#include "sensors.h"
#include "actuators.h"
#include "pid.h"
#include <stdio.h>

uint8_t chamberHeaterTempSet;
uint8_t steamGeneratorTempSet;

extern s_Sensors Sensors;


uint8_t PhasePreheat(uint8_t chamberTemp, uint8_t steamGenTemp) {

	HeaterChamberTempSet(chamberTemp);
	HeaterSteamGeneratorTempSet(steamGenTemp);
}

uint8_t PhaseHeatUp(uint8_t chamberTemp, uint8_t steamGenTemp, uint16_t timeout) {

	uint8_t retVal = 0;
	RTCTimerTime_s t;
	ResetTimer(1);
	FanSet(ON);
	HeaterChamberTempSet(chamberTemp);
	HeaterSteamGeneratorTempSet(steamGenTemp);

	while (1) {
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
	FanSet(OFF);
	return retVal;
}

uint8_t PhaseVacuum(float press, uint16_t timeout) {

	uint8_t retVal = 0;
	RTCTimerTime_s t;
	ResetTimer(1);

	// Valves
	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveWaterSteamSet(OFF);
	ValveVacuumSet(ON);
	osDelay(100);

	// Pumps
	PumpVacuumSet(ON);
	PumpSteamGeneratorSet(OFF);
	PumpWaterLoadSet(OFF);

	while (1) {
		if (Sensors.PressureChamber <= press) {
			retVal = 1;
			break;
		}
		GetTimer(1, &t);
		if (t.Seconds > timeout) {
			retVal = 0;
			break;
		}
	}
	PumpVacuumSet(OFF);
	osDelay(100);
	ValveVacuumSet(OFF);

	return retVal;

}

uint8_t PhasePressure(float press, float steamTemp, float steamTempMin, uint16_t timeout) {
	static const float pPID_KP = 1000.0;
	static const float pPID_KI = 800.0;
	static const float pPID_KD = 100.0;
	static const float pPID_TAU = 1.0;
	static const float pPID_LIM_MIN = 3000;
	static const float pPID_LIM_MAX = 8000;
	static const float pPID_LIM_MIN_INT = -800;
	static const float pPID_LIM_MAX_INT = 800;
	static const float pSAMPLE_TIME_S = 1;

	uint8_t retVal = 0;
	RTCTimerTime_s t;
	ResetTimer(1);

	PIDController pidSteam = {pPID_KP, pPID_KI, pPID_KD, pPID_TAU, pPID_LIM_MIN, pPID_LIM_MAX, pPID_LIM_MIN_INT, pPID_LIM_MAX_INT, pSAMPLE_TIME_S};

	ValveDrainSteamSet(ON);
	osDelay(100);
	while (1) {
		float ret = PIDController_Update(&pidSteam, steamTemp, Sensors.TemperatureSteamGenerator);
//		float ret = PIDController_Update(&pidSteam, steamTemp, 31.0);

		if (Sensors.TemperatureSteamGenerator >= steamTempMin) {
			ValveWaterSteamSet(ON);
			osDelay(100);
			PumpSteamGeneratorSet(ON);
			osDelay(250);
			PumpSteamGeneratorSet(OFF);
			osDelay(100);
			ValveWaterSteamSet(OFF);
			HAL_Delay(ret);
//			Beep(1,100,100);
		}

		osDelay(100);
		GetTimer(1, &t);

		if (Sensors.PressureChamber >= press) {
			retVal = 1;
			break;
		} else if (t.Seconds > timeout) {
			retVal = 0;
			break;
		}
	}

	osDelay(100);
	ValveDrainSteamSet(OFF);
	HeaterSteamGeneratorTempSet(steamTemp);

	return retVal;
}


