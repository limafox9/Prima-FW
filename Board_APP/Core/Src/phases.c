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
#include "cmsis_os.h"
#include "panel_serial.h"
#include "printer.h"
#include <stdio.h>

uint8_t chamberHeaterTempSet;
uint8_t steamGeneratorTempSet;

extern s_Sensors Sensors;


uint8_t PhasePreheat(float ringTemp, float steamGenTemp) {

	HeaterChamberTempSet(ringTemp);
	HeaterSteamGeneratorTempSet(steamGenTemp);
}

uint8_t PhaseHeatUp(float ringTemp, float steamGenTemp, float chamberTemp, uint16_t timeout) {

	uint8_t retVal = 0;
	RTCTimerTime_s tTimeout;

	ResetTimer(TIMER_TIMEOUT);

	HeaterChamberTempSet(ringTemp);
	HeaterSteamGeneratorTempSet(steamGenTemp);

	while (1) {
		if ((Sensors.TemperatureChamberHeater >= ringTemp) && \
			(Sensors.TemperatureSteamGenerator >= steamGenTemp) && \
			(Sensors.T1 >= chamberTemp)) {
			retVal = 1;
			break;
		}
		if ((Sensors.TemperatureSteamGenerator >= steamGenTemp) && (Sensors.T1 < chamberTemp)) {
			ValveVacuumSet(ON);
			ValveDrainSteamSet(ON);
			SteamPulse(150);
			osDelay(5000);
			ValveDrainSteamSet(OFF);
			ValveVacuumSet(OFF);
		}
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if (tTimeout.Seconds > timeout) {
			retVal = 0;
			break;
		}
		osDelay(200);
	}
	return retVal;
}

uint8_t PhaseRelease(float press, uint16_t timeout) {

	uint8_t retVal = 1;
	RTCTimerTime_s tTimeout;
	ResetTimer(1);

	// Valves
	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveWaterSteamSet(OFF);
	ValveVacuumSet(OFF);
	HAL_Delay(100);

	while (1) {
		if (Sensors.PressureChamber <= press) {
			retVal = 1;
			break;
		}
		GetTimer(1, &tTimeout);
		if (tTimeout.Seconds > timeout) {
			retVal = 1;
			break;
		}
		osDelay(200);
	}

	return retVal;
}

uint8_t PhaseVacuum(float press, uint16_t timeout) {

	uint8_t retVal = 0;
	RTCTimerTime_s tTimeout;
	ResetTimer(TIMER_TIMEOUT);

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
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if (tTimeout.Seconds > timeout) {
			retVal = 0;
			break;
		}
		osDelay(200);
	}
	PumpVacuumSet(OFF);
	osDelay(100);
	ValveVacuumSet(OFF);

	return retVal;

}

uint8_t PhasePressure(float press, float steamTemp, float steamTempMin, uint16_t timeout) {
	static const float pPID_KP = 500.0;
	static const float pPID_KI = 2000.0;
	static const float pPID_KD = 10.0;
	static const float pPID_TAU = 1.0;
	static const float pPID_LIM_MIN = 3000;
	static const float pPID_LIM_MAX = 8000;
	static const float pPID_LIM_MIN_INT = -1000;
	static const float pPID_LIM_MAX_INT = 1000;
	static const float pSAMPLE_TIME_S = 0.5;

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
			SteamPulse(150);
			osDelay(ret);
//			Beep(1,100,100);
		}

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

uint8_t PhaseSteril(float chamberTemp, float chamberTempMin, float chamberTempMax, uint16_t sterilTime, float pressMin, float pressMax, float steamTemp, float steamTempMin, uint16_t timeout) {
	static const float pPID_KP = 500.0;
	static const float pPID_KI = 2000.0;
	static const float pPID_KD = 10.0;
	static const float pPID_TAU = 1.0;
	static const float pPID_LIM_MIN = 2000;
	static const float pPID_LIM_MAX = 8000;
	static const float pPID_LIM_MIN_INT = -1000;
	static const float pPID_LIM_MAX_INT = 1000;
	static const float pSAMPLE_TIME_S = 0.5;

	uint8_t retVal = 0;

	float T1Min, T1Max, T2Min, T2Max, pMin, pMax;

	RTCTimerTime_s tTimeout, tPhase, tInterval;
	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);

	PIDController pidSteam = {pPID_KP, pPID_KI, pPID_KD, pPID_TAU, pPID_LIM_MIN, pPID_LIM_MAX, pPID_LIM_MIN_INT, pPID_LIM_MAX_INT, pSAMPLE_TIME_S};

	ValveDrainSteamSet(ON);
	osDelay(100);

//	while (1) {
//		float ret = PIDController_Update(&pidSteam, steamTemp, Sensors.TemperatureSteamGenerator);
//		if ((Sensors.TemperatureSteamGenerator >= steamTempMin) && (Sensors.PressureChamber < pressMin)) {
//			SteamPulse(200);
//			osDelay(ret);
//		}
//		GetTimer(1, &t);
//		if (Sensors.PressureChamber >= pressMin) {
//			retVal = 1;
//			break;
//		} else if (t.Seconds > timeout) {
//			retVal = 0;
//			break;
//		}
//		osDelay(100);
//	}

	ResetTimer(TIMER_1);

	while (1) {
		float ret = PIDController_Update(&pidSteam, steamTemp, Sensors.TemperatureSteamGenerator);
		if (Sensors.T1 < chamberTempMin) {
			if (Sensors.TemperatureSteamGenerator >= steamTempMin) {
				SteamPulse(50);
				osDelay(ret);
			}
		}
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if ((Sensors.T1 >= chamberTempMin) && (Sensors.T1 <= chamberTempMax)) {
			if ((Sensors.PressureChamber >= pressMin) && (Sensors.PressureChamber <= pressMax)) {
				Beep(1, 100, 100);
				ResetTimer(2);
				retVal = 1;
				break;
			}
		} else if (tTimeout.Seconds > timeout) {
			retVal = 0;
			break;
		}
		osDelay(100);
	}

	if (retVal) {
		while (1) {
			float ret = PIDController_Update(&pidSteam, steamTemp, Sensors.TemperatureSteamGenerator);
			if (Sensors.T1 <= chamberTemp) {
				if (Sensors.TemperatureSteamGenerator >= steamTempMin) {
					SteamPulse(20);
					osDelay(ret);
				}
			}
			GetTimer(TIMER_TIMEOUT, &tTimeout);
			GetTimer(TIMER_PHASE, &tPhase);
			if ((Sensors.T1 <= chamberTempMin) || (Sensors.T1 >= chamberTempMax)) {
				retVal = 0;
				break;
			} else if (tTimeout.Seconds > timeout) {
				retVal = 0;
				break;
			} else if (tPhase.Seconds >= sterilTime) {
				Beep(1, 100, 100);
				retVal = 1;
				break;
			}
			T1Min = (Sensors.T1 < T1Min) ? Sensors.T1 : T1Min;
			T1Max = (Sensors.T1 > T1Max) ? Sensors.T1 : T1Max;
			T2Min = (Sensors.T2 < T2Min) ? Sensors.T2 : T2Min;
			T2Max = (Sensors.T2 > T2Max) ? Sensors.T2 : T2Max;
			pMin = (Sensors.PressureChamber < pMin) ? Sensors.PressureChamber : pMin;
			pMax = (Sensors.PressureChamber > pMax) ? Sensors.PressureChamber : pMax;
			GetTimer(TIMER_1, &tInterval);
			if (tInterval.Seconds >= 30) {
				Print("       %03.1f %03.1f %03.1f   %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
				ResetTimer(TIMER_1);
			}
			osDelay(100);
		}
	}

	osDelay(100);
	ValveDrainSteamSet(OFF);
	HeaterSteamGeneratorTempSet(steamTemp);

	Print("  TIME                %01d:%01d\n", tPhase.TimeMinute, tPhase.TimeSecond);
	Print("   MIN %03.1f %03.1f %03.1f\n", T1Min, T2Min, pMin);
	Print("   MAX %03.1f %03.1f %03.1f\n", T1Max, T2Max, pMax);
	Print("--------------------------------\n");

	return retVal;
}

uint8_t PhaseDry(float firstVacPress, float otherVacPress, float releasePress, uint16_t time, uint16_t timeout) {

	uint8_t retVal = 1;
	uint8_t cycle = 0;
	float currPress;
	RTCTimerTime_s t, t2;
	ResetTimer(1);
	ResetTimer(2);

	ValveDrainSteamSet(OFF);
	ValveWaterSteamSet(OFF);

	ValveDryIntakeSet(OFF);
	ValveVacuumSet(ON);
	PumpVacuumSet(ON);


	while (1) {
		cycle++;
		if (retVal) {
			while (1) {
				GetTimer(1, &t);
				GetTimer(2, &t2);
				currPress = (cycle <= 1) ? firstVacPress : otherVacPress;
				if (Sensors.PressureChamber <= currPress) {
					retVal = 1;
					break;
				} else if (t.Seconds > timeout) {
					retVal = 0;
					break;
				} else if (t2.Seconds > time) {
					retVal = 1;
					break;
				}
				osDelay(100);
			}
		}

		if (retVal) {
			ValveDryIntakeSet(ON);
			while (1) {
				GetTimer(1, &t);
				GetTimer(2, &t2);
				if (Sensors.PressureChamber >= releasePress) {
					retVal = 1;
					break;
				} else if (t.Seconds > timeout) {
					retVal = 0;
					break;
				} else if (t2.Seconds > time) {
					retVal = 1;
					break;
				}
				osDelay(100);
			}
		}

		osDelay(100);
		ValveDryIntakeSet(OFF);

		GetTimer(1, &t);
		GetTimer(2, &t2);
		if (t.Seconds > timeout) {
			retVal = 0;
			break;
		} else if (t2.Seconds > time) {
			retVal = 1;
			break;
		}
	}

	ValveDryIntakeSet(OFF);
	PumpVacuumSet(OFF);
	ValveVacuumSet(OFF);

	return retVal;
}

uint8_t PhaseFinish() {
	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveVacuumSet(OFF);
	ValveWaterSteamSet(OFF);

	PumpSteamGeneratorSet(OFF);
	PumpVacuumSet(OFF);
	PumpWaterLoadSet(OFF);

	FanSet(OFF);

	HeaterChamberTempSet(0.0);
	HeaterSteamGeneratorTempSet(0.0);

	return 1;
}


