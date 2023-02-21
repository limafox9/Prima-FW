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
#include "printer.h"
#include "Inc/DB_Protocol.h"
#include <stdio.h>
#include <math.h>
#include "settings.h"
#include "modes.h"
#include "globals.h"

#define STERIL_STABILIZATION_PERIOD 30

uint8_t chamberHeaterTempSet;
uint8_t steamGeneratorTempSet;

char currPhase[8];

//extern s_Sensors Sensors;


uint8_t CheckError();

uint8_t PhaseBegin(uint8_t timeout) {

	uint8_t retVal = 1;
	RTCTimerTime_s tTimeout, tPhase;
	sprintf(currPhase, "bEG. ");

	ResetTimer(TIMER_PHASE);

	osDelay(1000);

	GetTimer(TIMER_PHASE, &tPhase);
	if (sensors.Door == OPEN) {
		lastError = ERROR_OPEN_DOOR;
		Print("DOOR ERROR *** DOOR OPEN\n");
		Beep(1, 1000, 100);
		retVal = 0;
	}

	if (DoorSolenoidSet(LOCK) != LOCK) {
		lastError = ERROR_PROTECTION_LOCK;
		Print("LOCK ERROR *** NO CONFIRMATION\n");
		Beep(1, 1000, 100);
		retVal = 0;
	}
	sensors.WaterTDS = BAT2U_GetTDSValue(1, 100);
	if (sensors.WaterTDS > settings.WaterTDSMin) {
		lastError = ERROR_WATER_HIGH_TDS;
		Print("WTDS ERROR *** WATER QUALITY\n");
		retVal = 0;
	}

	if ((sensors.WaterReservoirLevel & EMPTY) == EMPTY) {
		lastError = ERROR_WATER_LOW;
		Print("WATR ERROR *** WATER LEVEL\n");
		Beep(1, 1000, 100);
		retVal = 0;
	}
	if (sensors.WaterDumpLevel == HIGH) {
		lastError = ERROR_WATER_DUMP_HIGH;
		Print("DUMP ERROR *** WATER DUMP LEVEL\n");
		Beep(1, 1000, 100);
		retVal = 0;
	}

	if (retVal == 0) {
		PrintError("\n", &tPhase);
		return retVal;
	}

	ResetTimer(TIMER_TIMEOUT);

	machineStatus.Status = STATUS_RUNNING;

	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveVacuumSet(OFF);
	ValveWaterSteamSet(OFF);

	PumpSteamGeneratorSet(OFF);
	PumpVacuumSet(OFF);
	PumpWaterLoadSet(OFF);

	FanSet(ON);

	settings.CyclesCount++;
	EEPROM_Write(EPA_CYCLE_COUNTER, &settings.CyclesCount, 2);
	machine.ReadOnlyCyclesCount++;
	EEPROM_Write(EPA_CYCLE_COUNTER_PERMANENT, &machine.ReadOnlyCyclesCount, 2);

	uint8_t err = ERROR_POWER_OFF;
	EEPROM_Write(EPA_LAST_ERROR, &err, 1);

	while (1) {
		lastError = ERROR_PHASE_TIMEOUT;
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if ((sensors.PressureChamber >= pressAmbientMin) && (sensors.PressureChamber <= pressAmbientMax)) {
			retVal = 1;
			break;
		}
		if (tTimeout.Seconds > timeout) {
			retVal = 0;
			break;
		}
		if (abortProgram) {
			abortProgram = 0;
			lastError = ERROR_ABNORMAL_EXIT;
			retVal = 0;
			break;
		}
		osDelay(200);
	}

	return retVal;
}

uint8_t PhaseResume(uint8_t timeout) {

	uint8_t retVal = 1;
	RTCTimerTime_s tTimeout, tPhase;
	sprintf(currPhase, "rES. ");

	ResetTimer(TIMER_PHASE);
	ResetTimer(TIMER_TIMEOUT);

	osDelay(1000);

	GetTimer(TIMER_PHASE, &tPhase);
	if ((sensors.Door == OPEN) || (sensors.Solenoid == UNLOCK)) {
		return 0;
	}

	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveVacuumSet(OFF);
	ValveWaterSteamSet(OFF);

	PumpSteamGeneratorSet(OFF);
	PumpVacuumSet(OFF);
	PumpWaterLoadSet(OFF);

	FanSet(ON);

	while (1) {
		if ((sensors.PressureChamber >= pressAmbientMin) && (sensors.PressureChamber <= pressAmbientMax)) {
			retVal = 1;
			break;
		}
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if (tTimeout.Seconds > timeout) {
			lastError = ERROR_PHASE_TIMEOUT;
			Print("HEAT ERROR *** TIMEOUT\n");
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

uint8_t PhaseHeatUp(float ringTemp, float steamGenTemp, float chamberTemp, float chamberPress, uint16_t timeout) {

	uint8_t retVal = 0;
	RTCTimerTime_s tTimeout, tPhase, tWait;
	sprintf(currPhase, "hEAt");

	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);


	SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_HEATING, 0, 0, "");

	HeaterChamberTempSet(ringTemp);
	HeaterSteamGeneratorTempSet(steamGenTemp);

	ValveDrainSteamSet(ON);

	while (1) {
		if ((sensors.TemperatureChamberHeater >= ringTemp) && \
			(sensors.TemperatureSteamGenerator >= steamGenTemp) && \
			(sensors.T1 >= chamberTemp)) {
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("HEAT", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
			retVal = 1;
			break;
		}
		if ((sensors.TemperatureSteamGenerator >= steamGenTemp) && (sensors.T1 < chamberTemp)) {
			GetTimer(TIMER_1, &tWait);
			if (tWait.Seconds > 5) {
				SteamPulse(200);
				ResetTimer(TIMER_1);
			}
		}
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if (tTimeout.Seconds > timeout) {
			lastError = ERROR_PHASE_TIMEOUT;
			Print("HEAT ERROR *** TIMEOUT\n");
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("----  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
			retVal = 0;
			break;
		}
		lastError = CheckError();
		if (lastError > 0) {
			retVal = 0;
			break;
		}
		osDelay(200);
	}

	ValveDrainSteamSet(OFF);

	return retVal;
}

uint8_t CheckError() {

	uint8_t retError = ERROR_NONE;

	if (abortProgram == 1) {
		abortProgram = 0;
		retError = ERROR_ABNORMAL_EXIT;
	} else if ((machineStatus.Status == STATUS_RUNNING) && (sensors.Door == OPEN)) {
		retError = ERROR_OPEN_DOOR;
	} else if (sensors.TemperatureSteamGenerator > 230.0) {
		retError = ERROR_STEAM_OVERTEMP;
	} else if (sensors.TemperatureChamberHeater > 150.0) {
		retError = ERROR_RING_OVERTEMP;
	} else if ((sensors.T1 > 138.0) || (sensors.T2 > 138.0)) {
		retError = ERROR_CHAMBER_OVERTEMP;
	} else if (sensors.PressureChamber > 345.0) {
		retError = ERROR_OVERPRESSURE;
	}

	return retError;
}

uint8_t PhaseRelease(uint8_t nRelease, uint16_t timeout) {

	uint8_t retVal = 1;
	RTCTimerTime_s tTimeout, tPhase;
	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);
	sprintf(currPhase, "rEL. ");

	if (nRelease == 1) SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_NVACUUM, 0, 0, "");
	else if (nRelease == 2) SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_NRELEASE, 0, 0, "");
	else if (nRelease == 3) SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_STERILRELEASE, 0, 0, "");

	// Valves
	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveWaterSteamSet(OFF);
	ValveVacuumSet(OFF);
	HAL_Delay(100);

	while (1) {
		if ((sensors.PressureChamber >= pressAmbientMin) && (sensors.PressureChamber <= pressAmbientMax)) {
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("EXHA ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
			retVal = 1;
			break;
		}
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if (tTimeout.Seconds > timeout) {
			lastError = ERROR_NO_PRESSURE_RELEASE;
			Print("EXHA ERROR *** TIMEOUT\n");
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("----  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
			retVal = 0;
			break;
		}
		lastError = CheckError();
		if (lastError > 0) {
			retVal = 0;
			break;
		}
		osDelay(200);
	}

	return retVal;
}

uint8_t PhaseVacuum(uint8_t nVacuum, float press, uint16_t timeout) {

	uint8_t retVal = 0;
	RTCTimerTime_s tTimeout, tPhase, tExt;
	char s[8];
	sprintf(s, "UAC.%1d", nVacuum);
	sprintf(currPhase, s);

	float pressInit = sensors.PressureChamber;

	const float pressDeltaValue = 10;
	const uint16_t deltaTime = 20;

	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);
	ResetTimer(TIMER_3);

	if (nVacuum == 1) SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_1STVACUUM, 0, 0, "");
	else if (nVacuum == 2) SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_NVACUUM, 0, 0, "");
	else if (nVacuum == 3) SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_STERILVACUUM, 0, 0, "");

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
		if (sensors.PressureChamber <= press) {
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("VACU", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
			retVal = 1;
			break;
		}
		GetTimer(TIMER_3, &tExt);
//		if ((abs(pressInit - sensors.PressureChamber) < pressDeltaValue) && (tExt.Seconds > deltaTime)) {
//			lastError = ERROR_TEMP_PRESS_MISMATCH;
//			Print("VACU ERROR *** DELTA TIMEOUT\n");
//			GetTimer(TIMER_PHASE, &tPhase);
//			PrintStepLine("----  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
//			retVal = 0;
//			break;
//		}
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if (tTimeout.Seconds > timeout) {
			if (sensors.PressureChamber > settings.VacuumThreshold) {
				lastError = ERROR_PHASE_TIMEOUT;
				Print("VACU ERROR *** TIMEOUT\n");
				GetTimer(TIMER_PHASE, &tPhase);
				PrintStepLine("----  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
				retVal = 0;
			} else {
				retVal = 1;
			}
			break;
		}
		lastError = CheckError();
		if (lastError > 0) {
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
//	static const float pPID_KP = 500.0;
//	static const float pPID_KI = 2000.0;
//	static const float pPID_KD = 10.0;
//	static const float pPID_TAU = 1.0;
//	static const float pPID_LIM_MIN = 3000;
//	static const float pPID_LIM_MAX = 8000;
//	static const float pPID_LIM_MIN_INT = -1000;
//	static const float pPID_LIM_MAX_INT = 1000;
//	static const float pSAMPLE_TIME_S = 0.5;

//	static const float pPID_KP = 0.4;
//	static const float pPID_KI = 0.3;
//	static const float pPID_KD = 0.4;
//	static const float pPID_TAU = 0.1;
//	static const float pPID_LIM_MIN = 0.87;
//	static const float pPID_LIM_MAX = 2.0;
//	static const float pPID_LIM_MIN_INT = -1;
//	static const float pPID_LIM_MAX_INT = 1;
//	static const float pSAMPLE_TIME_S = 0.25;

	static const float pPID_KP = 0.5; // 1.0
	static const float pPID_KI = 1.8; // 1.8
	static const float pPID_KD = 0.2; // 1.0
	static const float pPID_TAU = 1.0;
	static const float pPID_LIM_MIN = 0.5; // 0.8
	static const float pPID_LIM_MAX = 1.5; // 2.0
	static const float pPID_LIM_MIN_INT = -1.5; // -1.5
	static const float pPID_LIM_MAX_INT = 1.5; // 1.5
	static const float pSAMPLE_TIME_S = 0.5; // 0.1

	uint8_t retVal = 0;
	RTCTimerTime_s tTimeout, tPhase;
	char s[8];
	sprintf(s, "PrE.%1d", nPressure);
	sprintf(currPhase, s);

	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);

	if (nPressure == 1) SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_1STPRESSURE, 0, 0, "");
	else if (nPressure == 2) SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_NPRESSURE, 0, 0, "");
	else if (nPressure == 3) SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_STERILPRESSURE, 0, 0, "");

	PIDController pidSteam = {pPID_KP, pPID_KI, pPID_KD, pPID_TAU, pPID_LIM_MIN, pPID_LIM_MAX, pPID_LIM_MIN_INT, pPID_LIM_MAX_INT, pSAMPLE_TIME_S};

	ValveDrainSteamSet(ON);
	osDelay(120);
	while (1) {
		float ret = PIDController_Update(&pidSteam, steamTemp, sensors.TemperatureSteamGenerator);
		ret = ret*4000;
//		float ret = PIDController_Update(&pidSteam, steamTemp, 31.0);

		if (sensors.TemperatureSteamGenerator >= steamTempMin) {
			SteamPulse(50);
			osDelay(ret);
//			Beep(1,100,100);
		}

		GetTimer(TIMER_TIMEOUT, &tTimeout);

		if (sensors.PressureChamber >= press) {
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("PRES", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
			retVal = 1;
			break;
		} else if (tTimeout.Seconds > timeout) {
			lastError = ERROR_PHASE_TIMEOUT;
			GetTimer(TIMER_PHASE, &tPhase);
			PrintError("PRES ERROR *** TIMEOUT\n", &tPhase);
			retVal = 0;
			break;
		}
		lastError = CheckError();
		if (lastError > 0) {
			retVal = 0;
			break;
		}
	}

	osDelay(100);
	ValveDrainSteamSet(OFF);
	HeaterSteamGeneratorTempSet(steamTemp);

	return retVal;
}

uint8_t PhaseSteril(SterilParam_s* param) {
	static const float pPID_KP = 0.6;  // >0.8
	static const float pPID_KI = 0.3;	// >0.4
	static const float pPID_KD = 0.2;
	static const float pPID_TAU = 1.0;
	static const float pPID_LIM_MIN = 1.0; // >1.1
	static const float pPID_LIM_MAX = 2.0;
	static const float pPID_LIM_MIN_INT = -1.0; // -1.0
	static const float pPID_LIM_MAX_INT = 1.0; // 1.0
	static const float pSAMPLE_TIME_S = 1.0;

	uint8_t retVal = 0;
	sprintf(currPhase, "rISE");

	float T1Min, T1Max, T2Min, T2Max, pMin, pMax;
	uint8_t overPress = 0;

	RTCTimerTime_s tTimeout, tPhase, tInterval, tDelay, tWait;
	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);

	SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_STERILMAINTAIN, 0, 0, "");
	Print("STER  --------------------------\n");

	PIDController pidSteril = {pPID_KP, pPID_KI, pPID_KD, pPID_TAU, pPID_LIM_MIN, pPID_LIM_MAX, pPID_LIM_MIN_INT, pPID_LIM_MAX_INT, pSAMPLE_TIME_S};

	ValveDrainSteamSet(ON);
	osDelay(100);

	ResetTimer(TIMER_1);

	while (1) {
//		float ret = PIDController_Update(&pidSteril, param->chamberTemp, sensors.T1);
		float ret = PIDController_Update(&pidSteril, param->chamberTemp, sensors.T1);
//		float ret = PIDController_Update(&pidSteril, param->chamberPress, sensors.PressureChamber);
		ret = (1/ret)*4000;
//		if ((Sensors.T1 < param->chamberTemp) && (Sensors.TemperatureSteamGenerator >= param->steamTempMin)) {
		if ((sensors.TemperatureSteamGenerator >= param->steamTempMin) && (sensors.T1 < param->chamberTemp)) {
			SteamPulse(50);
			osDelay(ret);
		}
		if ((sensors.T1 > param->chamberTempMin) && (sensors.T1 < param->chamberTempMax)) {
//			if ((sensors.PressureChamber > param->pressMin) && (sensors.PressureChamber < param->pressMax)){
			if ((sensors.PressureChamber > 310.0) && (sensors.PressureChamber < 340)){
				GetTimer(TIMER_3, &tWait);
				if (tWait.Seconds > STERIL_STABILIZATION_PERIOD) {
					retVal = 1;
					break;
				}
			} else {
				ResetTimer(TIMER_3);
			}
		} else {
			ResetTimer(TIMER_3);
		}
		if ((sensors.PressureChamber > param->pressMax) && (overPress == 0)) {
			overPress = 1;
			Print("STER WARNING *** T1 PRESSURE\n");
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("----  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
		}
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if (tTimeout.Seconds > param->timeout) {
			lastError = ERROR_PHASE_TIMEOUT;
			Print("STER ERROR *** TIMEOUT CLIMB\n");
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("----  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
			retVal = 0;
			break;
		}
		lastError = CheckError();
		if (lastError > 0) {
			retVal = 0;
			break;
		}
		osDelay(100);
	}

	if (retVal) {

		sprintf(currPhase, "StEr");

		ResetTimer(TIMER_PHASE);
		ResetTimer(TIMER_1);
		ResetTimer(TIMER_3);

		T1Min = sensors.T1;
		T1Max = sensors.T1;
		T2Min = sensors.T2;
		T2Max = sensors.T2;
		pMin = sensors.PressureChamber;
		pMax = sensors.PressureChamber;

		uint32_t currTick = HAL_GetTick();

		Beep(1, 100, 100);

		while (1) {

			// Exit successful on phase completion
			GetTimer(TIMER_PHASE, &tPhase);
			if (tPhase.Seconds >= param->sterilTime) {
				Beep(1, 100, 100);
				retVal = 1;
				break;
			}

			// PID Calculation and steam generation interval set
			float ret = PIDController_Update(&pidSteril, param->chamberTemp, sensors.T1);
//			float ret = PIDController_Update(&pidSteril, param->chamberPress, sensors.PressureChamber);
			ret = (1/ret)*4000;
			if ((sensors.T1 <= param->chamberTemp) && (HAL_GetTick() > (currTick + ret))) {
				if (sensors.TemperatureSteamGenerator >= param->steamTempMin) {
					SteamPulse(30);
					currTick = HAL_GetTick();
				}
			}

			// Delay before counting phase (steril) time. Wait to settle temp.
//			GetTimer(TIMER_3, &tDelay);
//			if (tDelay.Seconds < 20) {
//				ResetTimer(TIMER_PHASE);
//			}

			// Check for errors
			GetTimer(TIMER_TIMEOUT, &tTimeout);
			if (sensors.T1 <= param->chamberTempMin) {
				lastError = ERROR_MAINTAIN_PRESSTEMP;
				Print("STER ERROR *** UNDERTEMPERATURE\n");
				GetTimer(TIMER_PHASE, &tPhase);
				PrintStepLine("----  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
				retVal = 0;
				break;
			} else if (sensors.PressureChamber >= param->pressMax) {
				lastError = ERROR_MAINTAIN_PRESSTEMP;
				Print("STER ERROR *** OVERPRESSURE\n");
				GetTimer(TIMER_PHASE, &tPhase);
				PrintStepLine("----  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
				retVal = 0;
				break;
			} else if (sensors.PressureChamber <= param->pressMin) {
				lastError = ERROR_MAINTAIN_PRESSTEMP;
				Print("STER ERROR *** UNDERPRESSURE\n");
				GetTimer(TIMER_PHASE, &tPhase);
				PrintStepLine("----  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
				retVal = 0;
				break;
			} else if (tTimeout.Seconds > param->timeout) {
				lastError = ERROR_PHASE_TIMEOUT;
				Print("STER ERROR *** TIMEOUT MAINTAIN\n");
				GetTimer(TIMER_PHASE, &tPhase);
				PrintStepLine("----  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
				retVal = 0;
				break;
			}

			T1Min = (sensors.T1 < T1Min) ? sensors.T1 : T1Min;
			T1Max = (sensors.T1 > T1Max) ? sensors.T1 : T1Max;
			T2Min = (sensors.T2 < T2Min) ? sensors.T2 : T2Min;
			T2Max = (sensors.T2 > T2Max) ? sensors.T2 : T2Max;
			pMin = (sensors.PressureChamber < pMin) ? sensors.PressureChamber : pMin;
			pMax = (sensors.PressureChamber > pMax) ? sensors.PressureChamber : pMax;

			GetTimer(TIMER_1, &tInterval);
			if (tInterval.Seconds >= param->printInterval) {
				PrintStepLine("     ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
				ResetTimer(TIMER_1);
			}

			lastError = CheckError();
			if (lastError > 0) {
				retVal = 0;
				break;
			}
			osDelay(200);
		}
	}

	osDelay(100);
	ValveDrainSteamSet(OFF);
	HeaterSteamGeneratorTempSet(param->steamTemp);

	Print("TIME                       %2d:%02d\n", tPhase.TimeMinute, tPhase.TimeSecond);
	Print("MIN   %5.1f  %5.1f  %5.1f\n", T1Min, T2Min, pMin);
	Print("MAX   %5.1f  %5.1f  %5.1f\n", T1Max, T2Max, pMax);

	return retVal;
}

uint8_t PhaseDry(float firstVacPress, float otherVacPress, uint16_t time, uint16_t timeout) {

	uint8_t retVal = 1;
	uint8_t cycle = 0;
	float currPress;
	RTCTimerTime_s tTimeout, tPhase;
	sprintf(currPhase, "dry");

	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);

	SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_DRY, 0, 0, "");
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
				if (sensors.PressureChamber <= currPress) {
					PrintStepLine("VACU  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
					retVal = 1;
					break;
				}
				if (tTimeout.Seconds > timeout) {
					lastError = ERROR_PHASE_TIMEOUT;
					Print("DRY  ERROR *** TIMEOUT VACUUM\n");
					GetTimer(TIMER_PHASE, &tPhase);
					PrintStepLine("----  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
					retVal = 0;
					break;
				}
				if (tPhase.Seconds > time) {
					retVal = 1;
					break;
				}
				lastError = CheckError();
				if (lastError > 0) {
					retVal = 0;
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
				if ((sensors.PressureChamber >= pressAmbientMin) && (sensors.PressureChamber <= pressAmbientMax)) {
					osDelay(5000);
					PrintStepLine("INTK  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
					retVal = 1;
					break;
				}
				if (tTimeout.Seconds > timeout) {
					lastError = ERROR_PHASE_TIMEOUT;
					Print("DRY  ERROR *** TIMEOUT INTAKE\n");
					GetTimer(TIMER_PHASE, &tPhase);
					PrintStepLine("----  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
					retVal = 0;
					break;
				}
				if (tPhase.Seconds > time) {
					retVal = 1;
					break;
				}
				lastError = CheckError();
				if (lastError > 0) {
					retVal = 0;
					break;
				}
				osDelay(200);
			}
		}

		if ((tTimeout.Seconds > timeout) || (tPhase.Seconds > time) || (retVal == 0)) break;
	}

	ValveDryIntakeSet(OFF);
	PumpVacuumSet(OFF);
	ValveVacuumSet(OFF);

	GetTimer(TIMER_PHASE, &tPhase);
	Print("TIME                       %2d:%02d\n", tPhase.TimeMinute, tPhase.TimeSecond);

	return retVal;
}

uint8_t PhaseClean() {

	uint8_t retVal = 1;
	uint8_t finish = 0;
	RTCTimerTime_s tVac, tWater, tPhase;
	sprintf(currPhase, "UASH");

	ResetTimer(TIMER_PHASE);
	ResetTimer(TIMER_3);

	SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_DRY, 0, 0, "");

	ValveDrainSteamSet(OFF);
	ValveWaterSteamSet(ON);
	ValveDryIntakeSet(OFF);
	ValveVacuumSet(ON);

	while (1) {
		PumpVacuumSet(ON);
		ResetTimer(TIMER_2);
		if (!finish) {
			while (1) {
				PumpSteamGeneratorSet(ON);
				GetTimer(TIMER_2, &tWater);
				if (tWater.Seconds > 30) break;
				osDelay(50);
			}
			ResetTimer(TIMER_2);
			while (1) {
				PumpSteamGeneratorSet(OFF);
				GetTimer(TIMER_2, &tWater);
				if (tWater.Seconds > 5) break;
				osDelay(50);
			}
			ResetTimer(TIMER_2);
			while (1) {
				PumpSteamGeneratorSet(ON);
				GetTimer(TIMER_2, &tWater);
				if (tWater.Seconds > 30) {
					PumpSteamGeneratorSet(OFF);
					finish = 1;
					break;
				}
				osDelay(50);
			}
			osDelay(30000);
			ValveDryIntakeSet(ON);
			osDelay(10000);
			ValveDryIntakeSet(OFF);
			osDelay(50000);
			ValveDryIntakeSet(ON);
			osDelay(10000);
			ValveDryIntakeSet(OFF);
		}
		GetTimer(TIMER_3, &tVac);
		if (tVac.Seconds > 230) break;
		osDelay(50);
	}
	PumpVacuumSet(OFF);

	ValveDrainSteamSet(ON);
	ValveDryIntakeSet(ON);

	while (1) {
		if ((sensors.PressureChamber >= pressAmbientMin) && (sensors.PressureChamber <= pressAmbientMax)) {
			break;
		}
		osDelay(200);
	}

	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveVacuumSet(OFF);
	ValveWaterSteamSet(OFF);

	GetTimer(TIMER_PHASE, &tPhase);
	Print("TIME                       %2d:%02d\n", tPhase.TimeMinute, tPhase.TimeSecond);

	return retVal;
}

uint8_t PhaseWaterLoad(uint16_t timeout) {

	uint8_t retVal = 1;
	uint8_t finish = 0;
	RTCTimerTime_s tVac, tWater, tPhase, tTimeout;
	sprintf(currPhase, "UALd");

	ResetTimer(TIMER_PHASE);
	ResetTimer(TIMER_TIMEOUT);

	ValveDrainSteamSet(OFF);
	ValveWaterSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveVacuumSet(OFF);

	if ((WaterReservoirLevelGet() & FULL) != FULL) {
		PumpWaterLoadSet(ON);
		while ((WaterReservoirLevelGet() & FULL) != FULL) {
			GetTimer(TIMER_TIMEOUT, &tTimeout);
			if (tTimeout.Seconds > timeout) break;
			osDelay(100);
		}
	}

	PumpWaterLoadSet(OFF);

	return retVal;
}

uint8_t PhaseVacuumHold(float vacPress, uint8_t tolerance, uint16_t time, uint16_t timeout) {

	uint8_t retVal = 1;
	uint8_t cycle = 0;
	float currPress;
	RTCTimerTime_s tTimeout, tPhase;
	sprintf(currPhase, "U.HLd");

	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);

	SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_1STVACUUM, 0, 0, "");
//	Print("VAC   --------------------------\n");

	ValveDrainSteamSet(ON);
	ValveWaterSteamSet(OFF);

	ValveDryIntakeSet(OFF);
	ValveVacuumSet(OFF);

	while (1) {
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		GetTimer(TIMER_PHASE, &tPhase);
		if (sensors.PressureChamber > (vacPress + tolerance)) {
			lastError = ERROR_VACUUM_TEST_FAILED;
			Print("VAC  ERROR *** OVER PRESSURE\n");
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("----  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
			retVal = 0;
			break;
		} else if (tTimeout.Seconds > timeout) {
			lastError = ERROR_PHASE_TIMEOUT;
			Print("VAC  ERROR *** TIMEOUT\n");
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("----  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
			retVal = 0;
			break;
		} else if (tPhase.Seconds > time) {
			retVal = 1;
			break;
		}
		lastError = CheckError();
		if (lastError > 0) {
			retVal = 0;
			break;
		}
		osDelay(200);
	}

	GetTimer(TIMER_PHASE, &tPhase);
	PrintStepLine("TEST     ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);

	return retVal;
}

uint8_t PhaseWait(uint16_t time) {

	uint8_t retVal = 1;
	uint8_t cycle = 0;
	float currPress;
	RTCTimerTime_s tPhase;
	ResetTimer(TIMER_PHASE);

	SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_1STVACUUM, 0, 0, "");
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
		lastError = CheckError();
		if (lastError > 0) {
			retVal = 0;
			break;
		}
		osDelay(200);
	}

	GetTimer(TIMER_PHASE, &tPhase);
	PrintStepLine("WAIT     ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);

	return retVal;
}

uint8_t PhaseFinish(uint8_t success, RTCDateTime_s* time, RTCTimerTime_s* timer) {

	uint8_t retVal = 1;
	sprintf(currPhase, "FIn.");

	ValveVacuumSet(OFF);
	ValveWaterSteamSet(OFF);

	PumpSteamGeneratorSet(OFF);
	PumpVacuumSet(OFF);
	PumpWaterLoadSet(OFF);

	HeaterChamberTempSet(60);
	HeaterSteamGeneratorTempSet(90);

	if (success) {
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "PASS");

//		uint8_t err = ERROR_NONE;
		lastError = ERROR_NONE;
		EEPROM_Write(EPA_LAST_ERROR, &lastError, 1);

		PrintFooter("PASS", time, timer);
		ValveDryIntakeSet(ON);
		ValveDrainSteamSet(ON);

		while (1) {
			if ((sensors.PressureChamber >= pressAmbientMin) && (sensors.PressureChamber <= pressAmbientMax)) {
				break;
			}
			lastError = CheckError();
			if (lastError > 0) {
				retVal = 0;
				break;
			}
			osDelay(200);
		}

		machineStatus.Status = STATUS_READY;

		FanSet(OFF);

		osDelay(1000);
		if (DoorSolenoidSet(UNLOCK) != UNLOCK) {
			SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "Er.11");
			return 0;
		}
		Beep(3, 100, 100);

	} else {
		char s[8];
		sprintf(s,"Er.%02d", lastError);
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, s);

		EEPROM_Write(EPA_LAST_ERROR, &lastError, 1);

		PrintFooter("FAIL", time, timer);
		Beep(1, 1000, 100);
	}

	if (lastError != ERROR_NONE) machineStatus.Status = STATUS_ERROR;

	PhasePreheat(0.0, 0.0);

	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);

	return retVal;
}

extern UART_HandleTypeDef huart1;

void PhaseTestPID(SterilParam_s* param) {
	static const float pPID_KP = 0.5;
	static const float pPID_KI = 0.8;
	static const float pPID_KD = 0.2;
	static const float pPID_TAU = 1.0;
	static const float pPID_LIM_MIN = 0.5;
	static const float pPID_LIM_MAX = 2.0;
	static const float pPID_LIM_MIN_INT = -1.5;
	static const float pPID_LIM_MAX_INT = 1.5;
	static const float pSAMPLE_TIME_S = 1.0;

	PIDController pidSteam = {pPID_KP, pPID_KI, pPID_KD, pPID_TAU, pPID_LIM_MIN, pPID_LIM_MAX, pPID_LIM_MIN_INT, pPID_LIM_MAX_INT, pSAMPLE_TIME_S};
	PIDController_Init(&pidSteam);

	while (1) {
		float ret = PIDController_Update(&pidSteam, param->chamberTemp, sensors.T1);
//		if ((Sensors.T1 < param->chamberTemp) && (Sensors.TemperatureSteamGenerator >= param->steamTempMin)) {
//			SteamPulse(50);
//			osDelay(ret);
//		}
		char* s[20];
		sprintf(s,"%f\n", (1/ret)*4000);
		HAL_UART_Transmit(&huart1, s, strlen(s), 500);

		osDelay(300);
	}
}

