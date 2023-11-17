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
#include "system.h"
#include "usbkey.h"
#include "programs.h"

#define STERIL_STABILIZATION_PERIOD 5 // 30

#define T1_T2_STERIL_MAX_TEMP_DIFF_134 0.5
#define T1_T2_STERIL_MAX_TEMP_DIFF_121 0.5

#define T1_OVERTEMP 140.0
#define T2_OVERTEMP 140.0
#define STEAM_OVERTEMP 230.0
#define HEAT_RING_OVERTEMP 160.0
#define CHAMBER_OVERPRESS 345.0

#define STEAM_PULSE_PRESSURE	100
#define STEAM_PULSE_RISE		60
#define STEAM_PULSE_STERIL		40

#define T1T2_CHECK_ERROR_INTERVAL 1
#define T1T2_MAX_ERR_READINGS 5

uint8_t chamberHeaterTempSet;
uint8_t steamGeneratorTempSet;

uint8_t sterilRunning = 0;

char currPhase[8];

RTCTimerTime_s tCheckError;


//extern s_Sensors Sensors;


uint8_t CheckError();

uint8_t PhaseBegin(uint8_t timeout) {

	uint8_t retVal = 1;
	RTCTimerTime_s tTimeout, tPhase;
	sprintf(currPhase, "Init");

	settings.UserDryTime = GetDryTime(settings.UserDryClass, settings.UserSterilTemp, settings.UserVacuumNumber);



	RTCDateTime_s ct;
	PCF8563_GetDateTime(&ct);

	sensors.WaterTDS = WaterQualityGet();

	ResetTimer(TIMER_PHASE);

//	osDelay(1000);

	GetTimer(TIMER_PHASE, &tPhase);

	if (doorOpened == 0 && currProgram != PROGRAM_RESUME_DRY) {
		lastError = ERROR_NONE;
		PrintSize(2, 2);
		Print("DOOR ERROR *** DOOR NOT OPEN\n");
		PrintSize(1, 1);
		Beep(3, 100, 100);
		retVal = 0;

	}

	if (sensors.Door == OPEN && currProgram != PROGRAM_RESUME_DRY) {
		lastError = ERROR_NONE;
		PrintSize(2, 2);
		Print("DOOR ERROR *** DOOR OPEN\n");
		PrintSize(1, 1);
		Beep(3, 100, 100);
		retVal = 0;
	}

	sensors.WaterTDS = WaterQualityGet();
	if (settings.FunTDSActive > 0 && sensors.WaterTDS > settings.WaterTDSMin && currProgram != PROGRAM_RESUME_DRY) {
		lastError = ERROR_NONE;
//		lastError = ERROR_WATER_HIGH_TDS;
		PrintSize(2, 2);
		Print("WTDS ERROR *** WATER QUALITY\n");
		PrintSize(1, 1);
		Beep(3, 100, 100);
		retVal = 0;
	}

	if (sensors.WaterReservoirLevel == EMPTY && currProgram != PROGRAM_RESUME_DRY) {
		lastError = ERROR_NONE;
//		lastError = ERROR_WATER_LOW;
		PrintSize(2, 2);
		Print("WATR ERROR *** WATER LEVEL\n");
		PrintSize(1, 1);
		Beep(3, 100, 100);
		retVal = 0;
	}
	if (sensors.WaterDumpLevel == FULL && currProgram != PROGRAM_RESUME_DRY) {
		lastError = ERROR_NONE;
//		lastError = ERROR_WATER_DUMP_HIGH;
		PrintSize(2, 2);
		Print("DUMP ERROR *** WATER DUMP LEVEL\n");
		PrintSize(1, 1);
		Beep(3, 100, 100);
		retVal = 0;
	}
	if (retVal == 0) {
		PrintError("\n", &tPhase);
		return 0;
	} else {
		if (DoorSolenoidSet(LOCK) != LOCK) {
			lastError = ERROR_PROTECTION_LOCK;
			PrintSize(2, 2);
			Print("LOCK ERROR *** NO CONFIRMATION\n");
			PrintSize(1, 1);
			PrintError("\n", &tPhase);
			Beep(1, 1000, 100);
			return 0;
		}
	}

	ResetTimer(TIMER_TIMEOUT);

	machineStatus.Status = STATUS_RUNNING;
	osThreadResume(displayInfoTaskHandle);

//	doorOpened = 0;
	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveVacuumSet(OFF);
	ValveWaterSteamSet(OFF);

	PumpSteamGeneratorSet(OFF);
	PumpVacuumSet(OFF);
	PumpWaterLoadSet(OFF);

	FanSet(ON);

	uint8_t err = ERROR_POWER_OFF;
	EEPROM_Write(EPA_LAST_ERROR, &err, 1);

	while (1) {
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if ((sensors.PressureChamber >= pressAmbientMin) && (sensors.PressureChamber <= pressAmbientMax)) {
			retVal = 1;
			break;
		}
		if (tTimeout.Seconds > timeout) {
			lastError = ERROR_PHASE_TIMEOUT;
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

	if (currProgram != PROGRAM_RESUME_DRY && retVal != 0) {
		settings.CyclesCount++;
		sprintf(currFilename, "%02d_%02d_%02d-%02d_%02d-%05d.txt", ct.Year%100, ct.Month, ct.Day, ct.Hour, ct.Minute, settings.CyclesCount);
		EEPROM_Write(EPA_CYCLE_COUNTER, &settings.CyclesCount, 2);
		machine.ReadOnlyCyclesCount++;
		EEPROM_Write(EPA_CYCLE_COUNTER_PERMANENT, &machine.ReadOnlyCyclesCount, 2);
	}

	return retVal;
}

uint8_t PhaseResume(uint16_t timeout) {

	uint8_t retVal = 1;
	RTCTimerTime_s tTimeout, tPhase;

	lastError = ERROR_NONE;

	EEPROM_Write(EPA_LAST_ERROR, &lastError, 1);

	ValveDryIntakeSet(ON);
	ValveDrainSteamSet(ON);

	ResetTimer(TIMER_TIMEOUT);
	while (1) {
		if ((sensors.PressureChamber >= pressAmbientMin) && (sensors.PressureChamber <= pressAmbientMax)) {
			break;
		}
		lastError = CheckError();
		if (lastError > 0) {
			retVal = 0;
			break;
		}
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if (tTimeout.Seconds > timeout) {
			break;
		}
		osDelay(200);
	}

	machineStatus.Status = STATUS_READY;

//		FanSet(OFF);
	ResetTimer(TIMER_FAN);

	osDelay(1000);
	if (DoorSolenoidSet(UNLOCK) != UNLOCK) {
		DigitsWrite(3, "Er.11");
		return 0;
	}

	doorOpened = 0;
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
	static uint8_t err02;

	if (abortProgram == 1) {
		abortProgram = 0;
		retError = ERROR_ABNORMAL_EXIT;
	} else {
		if (currProgram != PROGRAM_RESUME_DRY) {
			if (sensors.TemperatureChamberHeater > HEAT_RING_OVERTEMP) {
				err02++;
			} else {
				err02 = 0;
			}

			if (sensors.Door == OPEN) {
				retError = ERROR_OPEN_DOOR;
			} else if (DoorSolenoidKeyGet() == UNLOCK) {
				retError = ERROR_PROTECTION_LOCK;
			} else if (sensors.TemperatureSteamGenerator > STEAM_OVERTEMP) {
				retError = ERROR_STEAM_OVERTEMP;
			} else if (err02 > 4) {
				retError = ERROR_RING_OVERTEMP;
			} else if ((sensors.T1 > T1_OVERTEMP) || (sensors.T2 > T2_OVERTEMP)) {
				retError = ERROR_CHAMBER_OVERTEMP;
			} else if (sensors.PressureChamber > CHAMBER_OVERPRESS) {
				retError = ERROR_OVERPRESSURE;
			}
		}
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
		osSemaphoreWait(semaphoreSensorsHandle, 100);
//		if ((sensors.PressureChamber >= sensors.PressureAmbient) && (sensors.PressureChamber <= (pressAmbientMax + 20))) {
//			ValveDryIntakeSet(ON);
//		}
		if ((sensors.PressureChamber >= pressAmbientMin) && (sensors.PressureChamber <= pressAmbientMax)) {
			GetTimer(TIMER_PHASE, &tPhase);
			PrintStepLine("EXHA ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
			ValveDryIntakeSet(OFF);
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
		osSemaphoreRelease(semaphoreSensorsHandle);
		osDelay(200);
	}

	return retVal;
}

uint8_t PhaseVacuum(uint8_t nVacuum, float press, uint8_t skipThreshold, uint16_t timeout) {

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

	if (nVacuum == 0) SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_DRY, 0, 0, "");
	else if (nVacuum == 1) SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_1STVACUUM, 0, 0, "");
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
				if (skipThreshold == 0 && sensors.PressureChamber <= settings.VacuumThreshold) {
					retVal = 1;

				} else {
					lastError = ERROR_PHASE_TIMEOUT;
					Print("VACU ERROR *** TIMEOUT\n");
					GetTimer(TIMER_PHASE, &tPhase);
					PrintStepLine("----  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
					retVal = 0;

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

	static const float pPID_KP = 100; 			// 100
	static const float pPID_KI = 60; 			// 60
	static const float pPID_KD = 40; 			// 40
	static const float pPID_TAU = 1.0; 			// 1.0
	static const float pPID_LIM_MIN = 50; 		// 50
	static const float pPID_LIM_MAX = 150; 		// 150
	static const float pPID_LIM_MIN_INT = -40; 	// -40
	static const float pPID_LIM_MAX_INT = 40; 	// 40
	static const float pSAMPLE_TIME_S = 1.0; 	// 1.0

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

	ChamberHeater_Stop();

	ValveDrainSteamSet(ON);
	osDelay(120);
	while (1) {
		float ret = PIDController_Update(&pidSteam, steamTemp, sensors.TemperatureSteamGenerator);
//		ret = ret*100;
//		float ret = PIDController_Update(&pidSteam, steamTemp, 31.0);

		if (sensors.TemperatureSteamGenerator >= steamTempMin) {
			SteamPulse(ret);
			osDelay(2500);
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

	ChamberHeater_Start();

	osDelay(100);
	ValveDrainSteamSet(OFF);
	HeaterSteamGeneratorTempSet(steamTemp);

	return retVal;

//	static const float pPID_KP = 0.5; // 1.0
//	static const float pPID_KI = 1.8; // 1.8
//	static const float pPID_KD = 0.2; // 1.0
//	static const float pPID_TAU = 1.0;
//	static const float pPID_LIM_MIN = 0.5; // 0.8
//	static const float pPID_LIM_MAX = 1.5; // 2.0
//	static const float pPID_LIM_MIN_INT = -1.5; // -1.5
//	static const float pPID_LIM_MAX_INT = 1.5; // 1.5
//	static const float pSAMPLE_TIME_S = 0.5; // 0.1
//
//	uint8_t retVal = 0;
//	RTCTimerTime_s tTimeout, tPhase;
//	char s[8];
//	sprintf(s, "PrE.%1d", nPressure);
//	sprintf(currPhase, s);
//
//	ResetTimer(TIMER_TIMEOUT);
//	ResetTimer(TIMER_PHASE);
//
//	if (nPressure == 1) SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_1STPRESSURE, 0, 0, "");
//	else if (nPressure == 2) SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_NPRESSURE, 0, 0, "");
//	else if (nPressure == 3) SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_STERILPRESSURE, 0, 0, "");
//
//	PIDController pidSteam = {pPID_KP, pPID_KI, pPID_KD, pPID_TAU, pPID_LIM_MIN, pPID_LIM_MAX, pPID_LIM_MIN_INT, pPID_LIM_MAX_INT, pSAMPLE_TIME_S};
//
//	ValveDrainSteamSet(ON);
//	osDelay(120);
//	while (1) {
//		float ret = PIDController_Update(&pidSteam, steamTemp, sensors.TemperatureSteamGenerator);
//		ret = ret*4000;
////		float ret = PIDController_Update(&pidSteam, steamTemp, 31.0);
//
//		if (sensors.TemperatureSteamGenerator >= steamTempMin) {
//			SteamPulse(50);
//			osDelay(ret);
////			Beep(1,100,100);
//		}
//
//		GetTimer(TIMER_TIMEOUT, &tTimeout);
//
//		if (sensors.PressureChamber >= press) {
//			GetTimer(TIMER_PHASE, &tPhase);
//			PrintStepLine("PRES", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
//			retVal = 1;
//			break;
//		} else if (tTimeout.Seconds > timeout) {
//			lastError = ERROR_PHASE_TIMEOUT;
//			GetTimer(TIMER_PHASE, &tPhase);
//			PrintError("PRES ERROR *** TIMEOUT\n", &tPhase);
//			retVal = 0;
//			break;
//		}
//		lastError = CheckError();
//		if (lastError > 0) {
//			retVal = 0;
//			break;
//		}
//	}
//
//	osDelay(100);
//	ValveDrainSteamSet(OFF);
//	HeaterSteamGeneratorTempSet(steamTemp);
//
//	return retVal;
}

uint8_t PhaseSteril(SterilParam_s* param) {
	static const float pPID_KP = 0.6;  // >0.8
	static const float pPID_KI = 0.3;	// >0.4
	static const float pPID_KD = 0.2;
	static const float pPID_TAU = 1.0;
	static const float pPID_LIM_MIN = 0.8; // >1.0
	static const float pPID_LIM_MAX = 1.3; // 2.0
	static const float pPID_LIM_MIN_INT = -1.0; // -1.0
	static const float pPID_LIM_MAX_INT = 1.0; // 1.0
	static const float pSAMPLE_TIME_S = 1.0;

	uint8_t retVal = 0;
	sprintf(currPhase, "rISE");

	float T1Min, T1Max, T2Min, T2Max, pMin, pMax, T1T2Diff;
	uint8_t overPress = 0;
	uint8_t timerReset = 0;

	uint8_t err09 = 0;

	T1T2Diff = (param->chamberTemp > 130.0) ? T1_T2_STERIL_MAX_TEMP_DIFF_134 : T1_T2_STERIL_MAX_TEMP_DIFF_121;

	RTCTimerTime_s tTimeout, tPhase, tInterval, tDelay, tWait;
	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);

	PIDController pidSteril = {pPID_KP, pPID_KI, pPID_KD, pPID_TAU, pPID_LIM_MIN, pPID_LIM_MAX, pPID_LIM_MIN_INT, pPID_LIM_MAX_INT, pSAMPLE_TIME_S};
	PIDController_Init(&pidSteril);

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
			SteamPulse(100);
			osDelay(ret);
		}
		if ((sensors.T1 >= param->chamberTemp) && (sensors.T1 < param->chamberTempMax)) {
			if (timerReset == 0) {
				timerReset = 1;
				ResetTimer(TIMER_3);
			}
//			if ((sensors.PressureChamber > param->pressMin) && (sensors.PressureChamber < param->pressMax)){
			if ((sensors.PressureChamber > param->pressMin) && (sensors.PressureChamber < param->pressMax)){
				GetTimer(TIMER_3, &tWait);
				if (tWait.Seconds > param->stabilizationPeriod) {
					retVal = 1;
					break;
				}
			} else {
//				ResetTimer(TIMER_3);
			}
		} else {
//			ResetTimer(TIMER_3);
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
			Print("STER ERROR *** TIMEOUT RISE\n");
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

	GetTimer(TIMER_PHASE, &tPhase);
	PrintStepLine("RISE ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);

	if (retVal) {

		sprintf(currPhase, "StEr");
//		osSemaphoreWait(semaphoreSensorsHandle, 100);
//		prevT1 = GetT1();
//		HAL_Delay(100);
//		prevT2 = GetT2();
//		osSemaphoreRelease(semaphoreSensorsHandle);
		sterilRunning = 1;


		ResetTimer(TIMER_TIMEOUT);
		ResetTimer(TIMER_PHASE);
		ResetTimer(TIMER_1);
		ResetTimer(TIMER_3);

		SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_STERILMAINTAIN, 0, 0, "");
		Print("STER  --------------------------\n");
		GetTimer(TIMER_PHASE, &tPhase);
		PrintStepLine("     ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);

		T1Min = sensors.T1;
		T1Max = sensors.T1;
		T2Min = sensors.T2;
		T2Max = sensors.T2;
		pMin = sensors.PressureChamber;
		pMax = sensors.PressureChamber;

		uint32_t currTick = HAL_GetTick();

		Beep(1, 100, 100);

		PIDController pidSteril1 = {pPID_KP, pPID_KI, pPID_KD, pPID_TAU, pPID_LIM_MIN, pPID_LIM_MAX, pPID_LIM_MIN_INT, pPID_LIM_MAX_INT, pSAMPLE_TIME_S};
		PIDController_Init(&pidSteril1);
		ResetTimer(TIMER_CHECK_ERROR);

		while (1) {

			// Exit successful on phase completion
			GetTimer(TIMER_PHASE, &tPhase);
			if (tPhase.Seconds >= param->sterilTime) {
				Beep(1, 100, 100);
				retVal = 1;
				break;
			}

			// PID Calculation and steam generation interval set
			float ret = PIDController_Update(&pidSteril1, param->chamberTemp, sensors.T1);
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

			GetTimer(TIMER_PHASE, &tCheckError);
			if (tCheckError.Seconds >= T1T2_CHECK_ERROR_INTERVAL) {
				if (fabs(sensors.T1 - sensors.T2) > T1T2Diff) {
					err09++;
				} else {
					err09 = 0;
				}

			}

			if (err09 >= T1T2_MAX_ERR_READINGS) {
				lastError = ERROR_T1T2_MISMATCH;
				Print("STER ERROR *** T1 T2 MISMATCH\n");
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
	sterilRunning = 0;

	osDelay(100);
	ValveDrainSteamSet(OFF);
	HeaterSteamGeneratorTempSet(param->steamTemp);

	Print("TIME                       %2d:%02d\n", tPhase.TimeMinute, tPhase.TimeSecond);
	Print("MIN   %5.1f  %5.1f  %5.1f\n", T1Min, T2Min, GetNormalizedPressure(pMin));
	Print("MAX   %5.1f  %5.1f  %5.1f\n", T1Max, T2Max, GetNormalizedPressure(pMax));

	return retVal;
}

uint8_t PhaseDry(float firstVacPress, float otherVacPress, uint16_t time, uint16_t timeout) {

	uint8_t retVal = 1;
	uint8_t cycle = 0;
	float currPress;
	RTCTimerTime_s tTimeout, tPhase;
	sterilRunning = 0;
	sprintf(currPhase, " dry");

	ResetTimer(TIMER_TIMEOUT);
	ResetTimer(TIMER_PHASE);
	ResetTimer(TIMER_1);

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
//					PrintStepLine("VACU  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
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
				if (sensors.PressureChamber >= (pressAmbientMin - 20)) {
					osDelay(5000);
//					PrintStepLine("INTK  ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);
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

//	if (currProgram == PROGRAM_RESUME_DRY) {
//
//		lastError = ERROR_NONE;
//		EEPROM_Write(EPA_LAST_ERROR, &lastError, 1);
//		currProgType = PROGTYPE_PROGRAM;
//		currProgram = 1;
//		ShowProgram();
//
//		return 1;
//	} else {
////		return retVal;
//	}
	return retVal;
}

uint8_t PhaseClean(uint16_t timeout) {

	uint8_t retVal = 1;
	uint8_t finish = 0;
	RTCTimerTime_s tVac, tWater, tPhase, tTimeout;
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
		lastError = CheckError();
		if (lastError > 0 && lastError != ERROR_ABNORMAL_EXIT) {
			retVal = 0;
			break;
		}
		osDelay(200);
	}
	PumpVacuumSet(OFF);

	ValveDrainSteamSet(ON);
	ValveDryIntakeSet(ON);

	ResetTimer(TIMER_TIMEOUT);
	while (1) {
		if ((sensors.PressureChamber >= pressAmbientMin) && (sensors.PressureChamber <= pressAmbientMax)) {
			break;
		}
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if (tTimeout.Seconds > timeout) {
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


	if (WaterReservoirLevelGet() != FULL) {
		PumpWaterLoadSet(ON);
		while (WaterReservoirLevelGet() != FULL) {
			GetTimer(TIMER_TIMEOUT, &tTimeout);
			if (tTimeout.Seconds > timeout) break;
			lastError = CheckError();
			if (lastError == ERROR_ABNORMAL_EXIT) {
				lastError = ERROR_NONE;
				break;
			}
			lastError = ERROR_NONE;
			osDelay(200);
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

uint8_t PhaseWait(float vacPress, uint8_t tolerance, uint16_t time) {

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
		if (sensors.PressureChamber > (vacPress + tolerance)) {
			lastError = ERROR_VACUUM_TEST_FAILED;
			Print("VAC  ERROR *** OVER PRESSURE\n");
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

	GetTimer(TIMER_PHASE, &tPhase);
	PrintStepLine("WAIT     ", sensors.T1, sensors.T2, sensors.PressureChamber, &tPhase);

	return retVal;
}

uint8_t PhaseFinish(uint8_t success, RTCDateTime_s* time, RTCTimerTime_s* timer, uint16_t timeout) {

	uint8_t retVal = 1;
	RTCTimerTime_s tTimeout;
	sprintf(currPhase, "FIn.");

	ValveVacuumSet(OFF);
	ValveWaterSteamSet(OFF);

	PumpSteamGeneratorSet(OFF);
	PumpVacuumSet(OFF);
	PumpWaterLoadSet(OFF);

	HeaterChamberTempSet(60);
	HeaterSteamGeneratorTempSet(90);

	ResetTimer(TIMER_FAN);


	if (success) {
		PrintFooter("PASS", time, timer);

//		uint8_t err = ERROR_NONE;
		lastError = ERROR_NONE;
		EEPROM_Write(EPA_LAST_ERROR, &lastError, 1);

		ValveDryIntakeSet(ON);
		ValveDrainSteamSet(ON);

		ResetTimer(TIMER_TIMEOUT);
		while (1) {
			if ((sensors.PressureChamber >= pressAmbientMin) && (sensors.PressureChamber <= pressAmbientMax)) {
				break;
			}
			lastError = CheckError();
			if (lastError > 0) {
				retVal = 0;
				break;
			}
			GetTimer(TIMER_TIMEOUT, &tTimeout);
			if (tTimeout.Seconds > timeout) {
				retVal = 0;
				break;
			}
			osDelay(200);
		}

		machineStatus.Status = STATUS_PASS;

	//		FanSet(OFF);
		ResetTimer(TIMER_FAN);

		osDelay(1000);
		if (DoorSolenoidSet(UNLOCK) != UNLOCK) {
//			DigitsWrite(3, "Er.11");
			return 11;
		}

		osDelay(3000);
		Beep(3, 100, 100);
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "PASS");

	} else {
		char s[8];
		sprintf(s,"Er.%02d", lastError);
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, s);
		machineStatus.Status = STATUS_ERROR;

		EEPROM_Write(EPA_LAST_ERROR, &lastError, 1);
		PrintFooter("FAIL", time, timer);
		Beep(1, 1000, 100);
		if (lastError == ERROR_ABNORMAL_EXIT) {
			currProgram = PROGRAM_RESUME_DRY;
			machineStatus.Status == STATUS_READY;
			machineStatus.Show = SHOW_PROGRAM;
			programStart = 1;
		}
	}
	osThreadSuspend(displayInfoTaskHandle);

	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);

	PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
	ResetTimer(TIMER_NIGHT_CYCLE);

	doorOpened = 0;

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

