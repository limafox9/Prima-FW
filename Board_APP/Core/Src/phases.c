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
#include "Inc/DB_Protocol.h"
#include <stdio.h>

uint8_t chamberHeaterTempSet;
uint8_t steamGeneratorTempSet;

extern s_Sensors Sensors;


uint8_t PhaseBegin(uint8_t timeout) {

	uint8_t retVal = 1;
	RTCTimerTime_s tTimeout, tPhase;

	ResetTimer(TIMER_TIMEOUT);

	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveVacuumSet(OFF);
	ValveWaterSteamSet(OFF);

	PumpSteamGeneratorSet(OFF);
	PumpVacuumSet(OFF);
	PumpWaterLoadSet(OFF);

	while (1) {
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if ((Sensors.PressureChamber >= 90) && (Sensors.PressureChamber <= 110)) {
			retVal = 1;
			break;
		}
		if (tTimeout.Seconds > timeout) {
				retVal = 0;
				break;
		}
		osDelay(200);
	}

	return retVal;
}

uint8_t PhasePreheat(float ringTemp, float steamGenTemp) {

	HeaterChamberTempSet(ringTemp);
	HeaterSteamGeneratorTempSet(steamGenTemp);
}

uint8_t PhaseHeatUp(float ringTemp, float steamGenTemp, float chamberTemp, uint16_t timeout) {

	uint8_t retVal = 0;
	RTCTimerTime_s tTimeout, tPhase;

	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);

	SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_HEATING, 0, 0, "");

	HeaterChamberTempSet(ringTemp);
	HeaterSteamGeneratorTempSet(steamGenTemp);

	while (1) {
		if ((Sensors.TemperatureChamberHeater >= ringTemp) && \
			(Sensors.TemperatureSteamGenerator >= steamGenTemp) && \
			(Sensors.T1 >= chamberTemp)) {
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("HEAT", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
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
			Print("HEAT ERROR *** TIMEOUT");
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("----  ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
			retVal = 0;
			break;
		}
		osDelay(200);
	}
	return retVal;
}

uint8_t PhaseRelease(uint8_t nRelease, float press, uint16_t timeout) {

	uint8_t retVal = 1;
	RTCTimerTime_s tTimeout, tPhase;
	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);

	if (nRelease == 1) SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_NVACUUM, 0, 0, "");
	else if (nRelease == 2) SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_NRELEASE, 0, 0, "");
	else if (nRelease == 3) SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_STERILRELEASE, 0, 0, "");

	// Valves
	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveWaterSteamSet(OFF);
	ValveVacuumSet(OFF);
	HAL_Delay(100);

	while (1) {
		if (Sensors.PressureChamber <= press) {
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("EXHA ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
			retVal = 1;
			break;
		}
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if (tTimeout.Seconds > timeout) {
			Print("EXHA ERROR *** TIMEOUT");
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("----  ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
			retVal = 0;
			break;
		}
		osDelay(200);
	}

	return retVal;
}

uint8_t PhaseVacuum(uint8_t nVacuum, float press, uint16_t timeout) {

	uint8_t retVal = 0;
	RTCTimerTime_s tTimeout, tPhase;
	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);

	if (nVacuum == 1) SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_1STVACUUM, 0, 0, "");
	else if (nVacuum == 2) SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_NVACUUM, 0, 0, "");
	else if (nVacuum == 3) SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_STERILVACUUM, 0, 0, "");

	// Valves
	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveWaterSteamSet(OFF);
	ValveVacuumSet(ON);
	osDelay(50);

	// Pumps
	PumpVacuumSet(ON);
	PumpSteamGeneratorSet(OFF);
	PumpWaterLoadSet(OFF);

	while (1) {
		if (Sensors.PressureChamber <= press) {
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("VACU", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
			retVal = 1;
			break;
		}
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if (tTimeout.Seconds > timeout) {
			Print("VACU ERROR *** TIMEOUT");
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("----  ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
			retVal = 0;
			break;
		}
		osDelay(200);
	}
	ValveDrainSteamSet(ON);
	PumpVacuumSet(OFF);
	osDelay(100);
	ValveVacuumSet(OFF);

	return retVal;
}

uint8_t PhasePressure(uint8_t nPressure, float press, float steamTemp, float steamTempMin, uint16_t timeout) {
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
	RTCTimerTime_s tTimeout, tPhase;
	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);

	if (nPressure == 1) SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_1STPRESSURE, 0, 0, "");
	else if (nPressure == 2) SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_NPRESSURE, 0, 0, "");
	else if (nPressure == 3) SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_STERILPRESSURE, 0, 0, "");

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

		GetTimer(TIMER_TIMEOUT, &tTimeout);

		if (Sensors.PressureChamber >= press) {
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("PRES", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
			retVal = 1;
			break;
		} else if (tTimeout.Seconds > timeout) {
			Print("PRES ERROR *** TIMEOUT");
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("----  ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
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

	SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_STERILMAINTAIN, 0, 0, "");
	Print("STER  --------------------------\n");

	PIDController pidSteam = {pPID_KP, pPID_KI, pPID_KD, pPID_TAU, pPID_LIM_MIN, pPID_LIM_MAX, pPID_LIM_MIN_INT, pPID_LIM_MAX_INT, pSAMPLE_TIME_S};

	ValveDrainSteamSet(ON);
	osDelay(100);

	ResetTimer(TIMER_1);

	while (1) {
		float ret = PIDController_Update(&pidSteam, steamTemp, Sensors.TemperatureSteamGenerator);
		if ((Sensors.T1 < chamberTemp) && (Sensors.TemperatureSteamGenerator >= steamTempMin)) {
			SteamPulse(50);
			osDelay(ret);
		}
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if (Sensors.T1 >= chamberTemp) {
			if ((Sensors.PressureChamber >= pressMin) && (Sensors.PressureChamber <= pressMax)) {
				retVal = 1;
				break;
			}
		} else if (tTimeout.Seconds > timeout) {
			Print("STER ERROR *** TIMEOUT CLIMB");
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("----  ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
			retVal = 0;
			break;
		}
		osDelay(100);
	}

	if (retVal) {

		Beep(1, 100, 100);
		ResetTimer(TIMER_PHASE);
		T1Min = Sensors.T1;
		T1Max = Sensors.T1;
		T2Min = Sensors.T2;
		T2Max = Sensors.T2;
		pMin = Sensors.PressureChamber;
		pMax = Sensors.PressureChamber;

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
			if (Sensors.T1 <= chamberTempMin) {
				Print("STER ERROR *** UNDER TEMPERATURE");
				GetTimer(TIMER_PHASE, &tPhase);
				PrintStepLine("----  ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
				retVal = 0;
				break;
			} else if (Sensors.T1 >= chamberTempMax) {
				Print("STER ERROR *** OVER TEMPERATURE");
				GetTimer(TIMER_PHASE, &tPhase);
				PrintStepLine("----  ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
				retVal = 0;
				break;
			} else if (tTimeout.Seconds > timeout) {
				Print("STER ERROR *** TIMEOUT MAINTAIN");
				GetTimer(TIMER_PHASE, &tPhase);
				PrintStepLine("----  ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
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
				PrintStepLine("     ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
				ResetTimer(TIMER_1);
			}
			osDelay(200);
		}
	}

	osDelay(100);
	ValveDrainSteamSet(OFF);
	HeaterSteamGeneratorTempSet(steamTemp);

	Print("TIME                       %2d:%02d\n", tPhase.TimeMinute, tPhase.TimeSecond);
	Print("MIN   %5.1f  %5.1f  %5.1f\n", T1Min, T2Min, pMin);
	Print("MAX   %5.1f  %5.1f  %5.1f\n", T1Max, T2Max, pMax);

	return retVal;
}

uint8_t PhaseDry(float firstVacPress, float otherVacPress, float releasePress, uint16_t time, uint16_t timeout) {

	uint8_t retVal = 1;
	uint8_t cycle = 0;
	float currPress;
	RTCTimerTime_s tTimeout, tPhase;
	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);

	SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_DRY, 0, 0, "");
	Print("DRY   --------------------------\n");

	ValveDrainSteamSet(OFF);
	ValveWaterSteamSet(OFF);

	ValveDryIntakeSet(OFF);
	ValveVacuumSet(ON);
	PumpVacuumSet(ON);

	while (1) {
		cycle++;
		if (retVal) {
			ValveDryIntakeSet(OFF);
			while (1) {
				GetTimer(TIMER_TIMEOUT, &tTimeout);
				GetTimer(TIMER_PHASE, &tPhase);
				currPress = (cycle <= 1) ? firstVacPress : otherVacPress;
				if (Sensors.PressureChamber <= currPress) {
					PrintStepLine("VACU  ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
					retVal = 1;
					break;
				} else if (tTimeout.Seconds > timeout) {
					Print("DRY  ERROR *** TIMEOUT VACUUM");
					GetTimer(TIMER_PHASE, &tPhase);
					PrintStepLine("----  ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
					retVal = 0;
					break;
				} else if (tPhase.Seconds > time) {
					retVal = 1;
					break;
				}
				osDelay(200);
			}
		}

		if (retVal) {
			ValveDryIntakeSet(ON);
			while (1) {
				GetTimer(TIMER_TIMEOUT, &tTimeout);
				GetTimer(TIMER_PHASE, &tPhase);
				if (Sensors.PressureChamber >= releasePress) {
					osDelay(5000);
					PrintStepLine("INTK  ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
					retVal = 1;
					break;
				} else if (tTimeout.Seconds > timeout) {
					Print("DRY  ERROR *** TIMEOUT INTAKE");
					GetTimer(TIMER_PHASE, &tPhase);
					PrintStepLine("----  ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
					retVal = 0;
					break;
				} else if (tPhase.Seconds > time) {
					retVal = 1;
					break;
				}
				osDelay(200);
			}
		}

		if ((tTimeout.Seconds > timeout) || (tPhase.Seconds > time)) break;
	}

	ValveDryIntakeSet(OFF);
	PumpVacuumSet(OFF);
	ValveVacuumSet(OFF);

	GetTimer(TIMER_PHASE, &tPhase);
	Print("TIME                       %2d:%02d\n", tPhase.TimeMinute, tPhase.TimeSecond);

	return retVal;
}

uint8_t PhaseVacuumHold(float vacPress, uint8_t tolerance, uint16_t time, uint16_t timeout) {

	uint8_t retVal = 1;
	uint8_t cycle = 0;
	float currPress;
	RTCTimerTime_s tTimeout, tPhase;
	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);

	SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_1STVACUUM, 0, 0, "");
//	Print("VAC   --------------------------\n");

	ValveDrainSteamSet(ON);
	ValveWaterSteamSet(OFF);

	ValveDryIntakeSet(OFF);
	ValveVacuumSet(OFF);

	while (1) {
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		GetTimer(TIMER_PHASE, &tPhase);
		if (Sensors.PressureChamber > (vacPress + tolerance)) {
			Print("VAC  ERROR *** OVER PRESSURE");
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("----  ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
			retVal = 0;
			break;
		} else if (tTimeout.Seconds > timeout) {
			Print("VAC  ERROR *** TIMEOUT");
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("----  ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);
			retVal = 0;
			break;
		} else if (tPhase.Seconds > time) {
			retVal = 1;
			break;
		}
		osDelay(200);
	}

	GetTimer(TIMER_PHASE, &tPhase);
	PrintStepLine("TEST     ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);

	return retVal;
}

uint8_t PhaseWait(uint16_t time) {

	uint8_t retVal = 1;
	uint8_t cycle = 0;
	float currPress;
	RTCTimerTime_s tPhase;
	ResetTimer(TIMER_PHASE);

	SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_1STVACUUM, 0, 0, "");
//	Print("VAC   --------------------------\n");

	ValveDrainSteamSet(ON);
	ValveWaterSteamSet(OFF);

	ValveDryIntakeSet(OFF);
	ValveVacuumSet(OFF);

	while (1) {
		GetTimer(TIMER_PHASE, &tPhase);
		if (tPhase.Seconds > time) {
			retVal = 1;
			break;
		}
		osDelay(200);
	}

	GetTimer(TIMER_PHASE, &tPhase);
	PrintStepLine("WAIT     ", Sensors.T1, Sensors.T2, Sensors.PressureChamber, &tPhase);

	return retVal;
}

uint8_t PhaseFinish(uint8_t success, RTCDateTime_s* time, RTCTimerTime_s* timer) {
	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveVacuumSet(OFF);
	ValveWaterSteamSet(OFF);

	PumpSteamGeneratorSet(OFF);
	PumpVacuumSet(OFF);
	PumpWaterLoadSet(OFF);

	HeaterChamberTempSet(60);
	HeaterSteamGeneratorTempSet(90);

	while (1) {
		if ((Sensors.PressureChamber >= 90) && (Sensors.PressureChamber <= 110)) {
			break;
		}
		osDelay(200);
	}

	if (success) {
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "PASS");
		PrintFooter("PASS", time, timer);
		Beep(3, 100, 100);
	} else {
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "FAIL");
		PrintFooter("FAIL", time, timer);
		Beep(1, 1000, 100);
	}

	return 1;
}


