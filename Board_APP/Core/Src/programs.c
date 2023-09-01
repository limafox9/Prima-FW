/*==============================================================================

Filename: programs.c

Created on: 28 gen 2023

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

#include "programs.h"

#include "phases.h"
#include "globals.h"
#include "Inc/DB_Protocol.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "time.h"
#include "sensors.h"
#include "printer.h"
#include "modes.h"
#include "settings.h"
#include "system.h"

#define OT_HEAT		1200
#define OT_VACUUM	300
#define OT_PRESSURE 600
#define OT_RELEASE	120

#define DV_PREHEAT_CHAMBER_TEMP	60.0
#define DV_HEATING_RING_TEMP	110.0
#define DV_STEAM_GEN_TEMP		200.0
#define DV_STEAM_GEN_TEMP_MIN	170.0
#define DV_STEAM_GEN_TEMP_PRESS	220.0
#define DV_WATER_LOAD_TIMEOUT	300

#define DV_STERIL_TEMP_134		135.5
#define DV_STERIL_TEMP_134_USER_0 134.7
#define DV_STERIL_TEMP_121		123.2
#define DV_STERIL_PRESS_MIN_134	308.0
#define DV_STERIL_PRESS_MAX_134	350.0
#define DV_STERIL_PRESS_MIN_121	200.0
#define DV_STERIL_PRESS_MAX_121	250.0

#define VAC_DVALUE	10
#define VAC_DTIME	20

#define DRY_FIRST_VAC	30.0
#define DRY_OTHER_VAC	50.0

#define FAN_OFF_DELAY	600

#define POWER_TEST_ON_TIME 3000
#define POWER_TEST_PAUSE_TIME 1000


#define STERIL_STABILIZATION_PERIOD 5
#define STERIL_STABILIZATION_PERIOD_HELIX 30

extern osThreadId sensorsTaskHandle;

extern Sensors_s sensors;
uint8_t doorStatus;

uint8_t programStart = 0;

void StartProgramTask() {
	RTCTimerTime_s timerDoor, timerFan, timerNCycle;
	doorStatus = (HAL_GPIO_ReadPin(SW_DOOR_GPIO_Port, SW_DOOR_Pin) == GPIO_PIN_SET) ? OPEN : CLOSED;
	for (;;) {

		GetTimer(TIMER_DOOR, &timerDoor);
		if ((doorStatus == OPEN) && (timerDoor.Seconds > 10)) {
			ValveDryIntakeSet(OFF);
		}

		GetTimer(TIMER_FAN, &timerFan);
		if ((machineStatus.Status != STATUS_RUNNING) && (timerFan.Seconds > FAN_OFF_DELAY)) {
			FanSet(OFF);
		}

		if (settings.FunNightCycleTime > 0) {
			GetTimer(TIMER_NIGHT_CYCLE, &timerNCycle);
			if ((machineStatus.Status != STATUS_RUNNING) && (timerNCycle.Minutes > settings.FunNightCycleTime)) {
				PhasePreheat(0.0, 0.0);
			} else if (machineStatus.Status == STATUS_RUNNING) {
				ResetTimer(TIMER_NIGHT_CYCLE);
			}
		}

		if (programStart){
			programStart = 0;
			sterilRunning = 0;
			if (currProgram == PROGRAM_RESUME_DRY) {
				ProgramResumeDry();
			} else {
				if (currProgType == PROGTYPE_PROGRAM) {
					switch (currProgram) {
					case PROGRAM_UNWRAPPED:
						ProgramUnwrapped();
						break;
					case PROGRAM_WRAPPED:
						ProgramWrapped();
						break;
					case PROGRAM_PRION:
						ProgramPrion();
						break;
					case PROGRAM_GENTLE:
						ProgramGentle();
						break;
					case PROGRAM_USER:
						ProgramUser();
						break;
					case PROGRAM_STERIL_TEST:
	//					ProgramTestSteril();
						break;
					}
				} else if (currProgType == PROGTYPE_TEST) {
					switch (currTest) {
					case 0:
						ProgramHelixTest();
						break;
					case 1:
						ProgramVacuumTest();
						break;
					case 2:
						ProgramWash();
						break;
					case 3:
						ProgramWaterLoad();
						break;
					}
				}
			}
		}
		sterilRunning = 0;
		osDelay(300);
	}
}

void ProgramOpenDoorRelease() {

}

uint8_t ProgramResumeDry() {

	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase, tTimeout;

	ResetTimer(TIMER_TIMEOUT);

	ProgressLedsOff();
	DigitsWrite(3,"----");

	progRunning = 1;
	progStop = 0;

//	if (!PhaseBegin(600)) return 0;
	sprintf(currPhase, "rEL.");
	osThreadResume(displayInfoTaskHandle);

	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveVacuumSet(OFF);
	ValveWaterSteamSet(OFF);

	PumpSteamGeneratorSet(OFF);
	PumpVacuumSet(OFF);
	PumpWaterLoadSet(OFF);

	while (1) {
		GetTimer(TIMER_TIMEOUT, &tTimeout);
		if (sensors.PressureChamber <= pressAmbientMax) {
			success = 1;
			break;
		}
		if (tTimeout.Seconds > OT_RELEASE) {
			lastError = ERROR_PHASE_TIMEOUT;
			success = 0;
			break;
		}
		if (abortProgram) {
			abortProgram = 0;
			lastError = ERROR_NONE;
			success = 0;
			break;
		}
		osDelay(200);
	}

//	PhaseResume(600);
	FanSet(ON);


//	while (1) {
//		if (sensors.PressureChamber <= pressAmbientMax) {
//			break;
//		}
//		osDelay(200);
//	}

	PhaseDry(DRY_FIRST_VAC, DRY_OTHER_VAC, 180, 800);
//	SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_NONE, 0, 0, "");


	ValveVacuumSet(OFF);
	ValveWaterSteamSet(OFF);

	PumpSteamGeneratorSet(OFF);
	PumpVacuumSet(OFF);
	PumpWaterLoadSet(OFF);

	ValveDrainSteamSet(ON);
	ValveDryIntakeSet(ON);

	while (1) {
		if ((sensors.PressureChamber >= (pressAmbientMin - 10)) && (sensors.PressureChamber <= pressAmbientMax)) {
			break;
		}
		osDelay(200);
	}

	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);

//	FanSet(OFF);
	ResetTimer(TIMER_FAN);

	osDelay(1000);
	if (DoorSolenoidSet(UNLOCK) != UNLOCK) {
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "Er.11");
		return 0;
	}

	machineStatus.Status = STATUS_READY;
	machineStatus.Show = SHOW_PROGRAM;
	lastError = ERROR_NONE;
	EEPROM_Write(EPA_LAST_ERROR, &lastError, 1);
	progRunning = 0;
	progStop = 0;
	currProgType = PROGTYPE_PROGRAM;
	currProgram = 1;
	ShowProgram();

	PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
	osThreadSuspend(displayInfoTaskHandle);

}

uint8_t ProgramUnwrapped() {

	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase, tWait;

	progRunning = 1;
	progStop = 0;

	if (!PhaseBegin(600)) return 0;

	uint16_t sterilTime = 0;
	uint16_t dryTime = 0;
	sterilTime = 240;
	dryTime = GetDryTime(settings.FunDryClass, STERILTEMP_134, 1) * 60;

	GetDateTime(&clockTime);
	PrintHeader("UNWRAPPED", sensors.T1, dryTime, sterilTime, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP, DV_PREHEAT_CHAMBER_TEMP, 160.0, OT_HEAT);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseRelease(1, OT_RELEASE);
	}

	ResetTimer(TIMER_CYCLE);

	// Sterilization
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseVacuum(1, (float)settings.FirstVacuum, 0, OT_VACUUM);
	}
//	if (settings.UserVacuumNumber < 3) {
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(3, 200.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
	}
	if (success) {
		ValveDrainSteamSet(OFF);
		ResetTimer(TIMER_5);
		do {
			GetTimer(TIMER_5, &tWait);
			lastError = CheckError();
			if (lastError > 0) {
				break;
			}
			osDelay(200);
		} while (tWait.Seconds < 20);
		ValveDrainSteamSet(ON);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(3, 290.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		SterilParam_s sterilParam = {
				.chamberTemp = DV_STERIL_TEMP_134,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = sterilTime,
				.chamberPress = 320,
				.pressMin = DV_STERIL_PRESS_MIN_134,
				.pressMax = DV_STERIL_PRESS_MAX_134,
				.steamTemp = DV_STEAM_GEN_TEMP,
				.steamTempMin = 170.0,
				.stabilizationPeriod = STERIL_STABILIZATION_PERIOD,
				.printInterval = 30,
				.timeout = 800
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
		success = PhaseRelease(3, OT_RELEASE);
	}

	// Dry
	if (success) {
		success = PhaseDry(DRY_FIRST_VAC, DRY_OTHER_VAC, dryTime, dryTime * 1.5);
	}

	// End
	PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
	osThreadSuspend(displayInfoTaskHandle);

	progStop = 0;

}

uint8_t ProgramWrapped() {
	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

	if (!PhaseBegin(600)) return 0;

	uint16_t sterilTime = 0;
	uint16_t dryTime = 0;
	sterilTime = 300;
	dryTime = GetDryTime(settings.FunDryClass, STERILTEMP_134, settings.VacNumber) * 60;

	GetDateTime(&clockTime);
	PrintHeader("WRAPPED", sensors.T1, dryTime, sterilTime, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP, DV_PREHEAT_CHAMBER_TEMP, 100.0, OT_HEAT);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseRelease(1, OT_RELEASE);
	}

	ResetTimer(TIMER_CYCLE);

	// First vacuum
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseVacuum(1, (float)settings.FirstVacuum, 0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(1, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseRelease(1, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < (settings.VacNumber - 2); i++) {  // -2 for 1st cycle and steril cycle
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseVacuum(2, (float)settings.OtherVacuum, 0, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
			success = PhasePressure(2, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseRelease(2, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 210.0);
		success = PhaseVacuum(3, (float)settings.OtherVacuum, 0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(3, 290.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		SterilParam_s sterilParam = {
				.chamberTemp = DV_STERIL_TEMP_134,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = sterilTime,
				.chamberPress = 320,
				.pressMin = DV_STERIL_PRESS_MIN_134,
				.pressMax = DV_STERIL_PRESS_MAX_134,
				.steamTemp = DV_STEAM_GEN_TEMP,
				.steamTempMin = 170.0,
				.stabilizationPeriod = STERIL_STABILIZATION_PERIOD,
				.printInterval = 30,
				.timeout = 800
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
		success = PhaseRelease(3, OT_RELEASE);
	}

	// Dry
	if (success) {
		success = PhaseDry(DRY_FIRST_VAC, DRY_OTHER_VAC, dryTime, dryTime * 1.5);
	}

	// End
	PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
	osThreadSuspend(displayInfoTaskHandle);

	progStop = 0;

}

uint8_t ProgramPrion() {
	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

	FanSet(ON);
	SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_HEATING, 0, 0, "");

	if (!PhaseBegin(600)) return 0;

	uint16_t sterilTime = 0;
	uint16_t dryTime = 0;
	sterilTime = 1080;
	dryTime = GetDryTime(settings.FunDryClass, STERILTEMP_134, settings.VacNumber) * 60;

	GetDateTime(&clockTime);
	PrintHeader("PRION", sensors.T1, dryTime, sterilTime, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP, DV_PREHEAT_CHAMBER_TEMP, 160.0, OT_HEAT);
	}

	ResetTimer(TIMER_CYCLE);

	// First vacuum
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseVacuum(1, (float)settings.FirstVacuum, 0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(1, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseRelease(1, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < (settings.VacNumber - 2); i++) {  // -2 for 1st cycle and steril cycle
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseVacuum(2, (float)settings.OtherVacuum, 0, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
			success = PhasePressure(2, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseRelease(2, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, (float)settings.OtherVacuum, 0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(3, 290.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		SterilParam_s sterilParam = {
				.chamberTemp = DV_STERIL_TEMP_134,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = sterilTime,
				.chamberPress = 320,
				.pressMin = DV_STERIL_PRESS_MIN_134,
				.pressMax = DV_STERIL_PRESS_MAX_134,
				.steamTemp = DV_STEAM_GEN_TEMP,
				.steamTempMin = 170.0,
				.stabilizationPeriod = STERIL_STABILIZATION_PERIOD,
				.printInterval = 120,
				.timeout = 1500
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
		success = PhaseRelease(3, OT_RELEASE);
	}

	// Dry
	if (success) {
		success = PhaseDry(DRY_FIRST_VAC, DRY_OTHER_VAC, dryTime, dryTime * 1.5);
	}

	// End
	PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
	osThreadSuspend(displayInfoTaskHandle);

	progStop = 0;

}

uint8_t ProgramGentle() {
	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

	if (!PhaseBegin(600)) return 0;

	uint16_t sterilTime = 0;
	uint16_t dryTime = 0;
	sterilTime = 1200;
	dryTime = GetDryTime(settings.FunDryClass, STERILTEMP_121, settings.VacNumber) * 60;

	GetDateTime(&clockTime);
	PrintHeader("GENTLE", sensors.T1, dryTime, sterilTime, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP, DV_PREHEAT_CHAMBER_TEMP, 160.0, OT_HEAT);
	}

	ResetTimer(TIMER_CYCLE);

	// First vacuum
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseVacuum(1, (float)settings.FirstVacuum, 0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(1, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseRelease(1, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < (settings.VacNumber - 2); i++) {  // -2 for 1st cycle and steril cycle
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseVacuum(2, (float)settings.OtherVacuum, 0, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
			success = PhasePressure(2, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseRelease(2, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, (float)settings.OtherVacuum, 0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(3, 190.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		SterilParam_s sterilParam = {
				.chamberTemp = DV_STERIL_TEMP_121,
				.chamberTempMin = 121.0,
				.chamberTempMax = 125.0,
				.sterilTime = sterilTime,
				.chamberPress = 220,
				.pressMin = DV_STERIL_PRESS_MIN_121,
				.pressMax = DV_STERIL_PRESS_MAX_121,
				.steamTemp = DV_STEAM_GEN_TEMP,
				.steamTempMin = 170.0,
				.stabilizationPeriod = STERIL_STABILIZATION_PERIOD,
				.printInterval = 120,
				.timeout = 1800
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
		success = PhaseRelease(3,  OT_RELEASE);
	}

	// Dry
	if (success) {
		success = PhaseDry(DRY_FIRST_VAC, DRY_OTHER_VAC, dryTime, dryTime * 1.5);
	}

	// End
	PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
	osThreadSuspend(displayInfoTaskHandle);

	progStop = 0;

}

uint8_t ProgramUser() {
	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase, tWait;

	progRunning = 1;
	progStop = 0;

	if (!PhaseBegin(600)) return 0;

	uint16_t sterilTime = 0;
	uint16_t dryTime = 0;
	sterilTime = settings.UserSterilTime * 60;
	dryTime = GetDryTime(settings.UserDryClass, settings.UserSterilTemp, settings.UserVacuumNumber) * 60;

	GetDateTime(&clockTime);
	PrintHeader("USER", sensors.T1, dryTime, sterilTime, &clockTime);

	float cTemp;
	cTemp = (settings.UserSterilTemp == STERILTEMP_134) ? DV_STERIL_TEMP_134 : DV_STERIL_TEMP_121;
	cTemp = (settings.UserVacuumNumber == 0 && settings.UserSterilTemp == STERILTEMP_134) ? DV_STERIL_TEMP_134_USER_0 : cTemp;


	// Heat up
	if (success) {
		success = PhaseHeatUp(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP, DV_PREHEAT_CHAMBER_TEMP, 100.0, OT_HEAT);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseRelease(1, OT_RELEASE);
	}

	ResetTimer(TIMER_CYCLE);

	// First vacuum
	if (settings.UserVacuumNumber == 0) {
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseVacuum(1, 50.0, 0, OT_VACUUM);
		}
	}
	if (settings.UserVacuumNumber > 0) {
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseVacuum(1, (float)settings.FirstVacuum, 0, OT_VACUUM);
		}
	}
	if (settings.UserVacuumNumber >= 3) {
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
			success = PhasePressure(1, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseRelease(1, OT_RELEASE);
		}
	}

	// Other vacuum
	if (settings.UserVacuumNumber >= 3) {
		for (int i = 0; i < (settings.UserVacuumNumber - 2); i++) {  // -2 for 1st cycle and steril cycle
			if (success) {
				PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
				success = PhaseVacuum(2, (float)settings.OtherVacuum, 0, OT_VACUUM);
			}
			if (success) {
				PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
				success = PhasePressure(2, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
			}
			if (success) {
				PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
				success = PhaseRelease(2, OT_RELEASE);
			}
		}
	}

	// Sterilization
	if (settings.UserVacuumNumber >= 3) {
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseVacuum(3, (float)settings.OtherVacuum, 0, OT_VACUUM);
		}
	}
	if (settings.UserVacuumNumber < 3) {
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
			success = PhasePressure(3, (settings.UserSterilTemp == STERILTEMP_134) ? 200.0 : 180.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
		}
		if (success) {
			ValveDrainSteamSet(OFF);
			ResetTimer(TIMER_5);
			do {
				GetTimer(TIMER_5, &tWait);
				lastError = CheckError();
				if (lastError > 0) {
					break;
				}
				osDelay(200);
			} while (tWait.Seconds < 25);
			ValveDrainSteamSet(ON);
		}
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(3, (settings.UserSterilTemp == STERILTEMP_134) ? 290.0 : 190.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		SterilParam_s sterilParam = {
				.chamberTemp = cTemp,
				.chamberTempMin = (settings.UserSterilTemp == STERILTEMP_134) ? 134.0 : 121.0,
				.chamberTempMax = (settings.UserSterilTemp == STERILTEMP_134) ? 136.0 : 125.0,
				.sterilTime = sterilTime,
				.chamberPress = (settings.UserSterilTemp == STERILTEMP_134) ? 320.0 : 220.0,
				.pressMin = (settings.UserSterilTemp == STERILTEMP_134) ? DV_STERIL_PRESS_MIN_134 : DV_STERIL_PRESS_MIN_121,
				.pressMax = (settings.UserSterilTemp == STERILTEMP_134) ? DV_STERIL_PRESS_MAX_134 : DV_STERIL_PRESS_MAX_121,
				.steamTemp = DV_STEAM_GEN_TEMP,
				.steamTempMin = 170.0,
				.stabilizationPeriod = STERIL_STABILIZATION_PERIOD,
				.printInterval = 30,
				.timeout = settings.UserSterilTime * 60 * 1.5,
		};

		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
		success = PhaseRelease(3, OT_RELEASE);
	}

	// Dry
	if (success) {
		if (dryTime > 0) {
			success = PhaseDry(DRY_FIRST_VAC, DRY_OTHER_VAC, dryTime, dryTime * 1.5);
		} else {
		PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
		success = PhaseVacuum(0, 50.0, 0, 300);
		}
	}

	// End
	PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
	osThreadSuspend(displayInfoTaskHandle);

	progStop = 0;

}

uint8_t ProgramHelixTest() {
	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

	if (!PhaseBegin(600)) return 0;

	uint16_t sterilTime = 0;
	uint16_t dryTime = 0;
	sterilTime = 210;
	dryTime = GetDryTime(settings.FunDryClass, STERILTEMP_134, 3) * 60;
	dryTime = 120;

	GetDateTime(&clockTime);
	PrintHeader("HELIX TEST", sensors.T1, dryTime, sterilTime, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP, DV_PREHEAT_CHAMBER_TEMP, 160.0, OT_HEAT);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseRelease(1, OT_RELEASE);
	}

	ResetTimer(TIMER_CYCLE);

	// First vacuum
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseVacuum(1, (float)settings.FirstVacuum, 0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(1, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseRelease(1, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < 1; i++) {
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseVacuum(2, (float)settings.OtherVacuum, 0, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
			success = PhasePressure(2, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseRelease(2, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, (float)settings.OtherVacuum, 0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(3, 290.0, DV_STEAM_GEN_TEMP_PRESS, DV_STEAM_GEN_TEMP_MIN, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		SterilParam_s sterilParam = {
				.chamberTemp = 135.5,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = sterilTime, // 210
				.chamberPress = 320,
				.pressMin = 290.0,
				.pressMax = 350.0,
				.steamTemp = DV_STEAM_GEN_TEMP,
				.steamTempMin = 170.0,
				.stabilizationPeriod = STERIL_STABILIZATION_PERIOD_HELIX,
				.printInterval = 30,
				.timeout = 600
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
		success = PhaseRelease(3, OT_RELEASE);
	}

	// Dry
	if (success) {
		success = PhaseDry(DRY_FIRST_VAC, DRY_OTHER_VAC, dryTime, dryTime * 1.5);
	}

	// End
	PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
	osThreadSuspend(displayInfoTaskHandle);

	progStop = 0;

}

uint8_t ProgramVacuumTest() {

	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

//	if (!PhaseBegin(600)) return 0;

	ResetTimer(TIMER_CYCLE);

	PhasePreheat(1, 1);

	if (!PhaseBegin(600)) return 0;

	GetDateTime(&clockTime);
	PrintHeader("VACUUM TEST", sensors.T1, 0, 0, &clockTime);

	if (success) {
		success = PhaseVacuum(1, 20.0, 1, OT_VACUUM);
	}
	if (success) {
		success = PhaseWait(sensors.PressureChamber, 5.0, 240);
	}
	if (success) {
		success = PhaseVacuumHold(sensors.PressureChamber, 2.0, 240, 600);
	}
//	if (success) {
//		success = PhaseRelease(1, OT_RELEASE);
//	}
//	machineStatus.Status = STATUS_READY;

	ResetTimer(TIMER_FAN);

	// End
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);

//	if (!success) PrintFooter("FAIL", &clockTime, &tCycle);

//		uint8_t err = ERROR_NONE;
//	lastError = ERROR_NONE;
//
//	EEPROM_Write(EPA_LAST_ERROR, &lastError, 1);

	if (lastError == ERROR_VACUUM_TEST_FAILED) {

		ValveDryIntakeSet(ON);
		ValveDrainSteamSet(ON);

		while (1) {
			if ((sensors.PressureChamber >= pressAmbientMin) && (sensors.PressureChamber <= pressAmbientMax)) {
				break;
			}
//			lastError = CheckError();
//			if (lastError > 0) {
//				success = 0;
//				break;
//			}
			osDelay(200);
		}
		ResetTimer(TIMER_FAN);

//		osDelay(1000);
//		if (DoorSolenoidSet(UNLOCK) != UNLOCK) {
//			DigitsWrite(3, "Er.11");
//			return 0;
//		}

		Beep(1, 1000, 100);

	}

	PhaseFinish(success, &clockTime, &tCycle);

	PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);

	progRunning = 0;
	osThreadSuspend(displayInfoTaskHandle);
	progStop = 0;

}

uint8_t PowerTest() {

	char val[4];
	uint8_t tCount = 0;

	osSemaphoreWait(semaphoreSensorsHandle, 100);

	machineStatus.Show = SHOW_FUNCTIONS;
	PWMPower_Stop();
	FanSet(OFF);

	PrintPowerTestHeader();

	tCount++;
	Print("%2d - NO LOAD\n", tCount);

	DigitsWrite(2, "FAn");
	FanSet(ON);
	osDelay(POWER_TEST_ON_TIME);
//	osDelay(500);
//	sprintf(val,"%d", CurrentGet());
//	DigitsWrite(1, val);
	FanSet(OFF);
	osDelay(POWER_TEST_PAUSE_TIME);
	tCount++;
	Print("%2d - FAN\n", tCount);

	DigitsWrite(2, "H St");
	HAL_GPIO_WritePin(HEATER_STEAM_GEN_GPIO_Port, HEATER_STEAM_GEN_Pin, ON);
	osDelay(POWER_TEST_ON_TIME);
//	osDelay(500);
//	sprintf(val,"%d", CurrentGet());
//	DigitsWrite(1, val);
	HAL_GPIO_WritePin(HEATER_STEAM_GEN_GPIO_Port, HEATER_STEAM_GEN_Pin, OFF);
	osDelay(POWER_TEST_PAUSE_TIME);
	tCount++;
	Print("%2d - STEAM GENERATOR\n", tCount);

	DigitsWrite(2, "H rI");
	HAL_GPIO_WritePin(HEATER_RING_GPIO_Port, HEATER_RING_Pin, ON);
	osDelay(POWER_TEST_ON_TIME);
//	osDelay(500);
//	sprintf(val,"%d", CurrentGet());
//	DigitsWrite(1, val);
	HAL_GPIO_WritePin(HEATER_RING_GPIO_Port, HEATER_RING_Pin, OFF);
	osDelay(POWER_TEST_PAUSE_TIME);
	tCount++;
	Print("%2d - HEATING RING\n", tCount);

	DigitsWrite(2, "U UA");
	ValveVacuumSet(ON);
	osDelay(POWER_TEST_ON_TIME);
//	osDelay(500);
//	sprintf(val,"%d", CurrentGet());
//	DigitsWrite(1, val);
	ValveVacuumSet(OFF);
	osDelay(POWER_TEST_PAUSE_TIME);
	tCount++;
	Print("%2d - VACUUM VALVE\n", tCount);

	DigitsWrite(2, "U dr");
	ValveDrainSteamSet(ON);
	osDelay(POWER_TEST_ON_TIME);
	ValveDrainSteamSet(OFF);
	osDelay(POWER_TEST_PAUSE_TIME);
	tCount++;
	Print("%2d - DRAIN STEAM VALVE\n", tCount);

	DigitsWrite(2, "U Ut");
	ValveWaterSteamSet(ON);
	osDelay(POWER_TEST_ON_TIME);
	ValveWaterSteamSet(OFF);
	osDelay(POWER_TEST_PAUSE_TIME);
	tCount++;
	Print("%2d - WATER STEAM VALVE\n", tCount);

	DigitsWrite(2, "U In");
	ValveDryIntakeSet(ON);
	osDelay(POWER_TEST_ON_TIME);
	ValveDryIntakeSet(OFF);
	osDelay(POWER_TEST_PAUSE_TIME);
	tCount++;
	Print("%2d - INTAKE FRONT VALVE\n", tCount);

	DigitsWrite(2, "P Ut");
	PumpWaterLoadSet(ON);
	osDelay(POWER_TEST_ON_TIME);
	PumpWaterLoadSet(OFF);
	osDelay(POWER_TEST_PAUSE_TIME);
	tCount++;
	Print("%2d - WATER LOAD PUMP\n", tCount);

	DigitsWrite(2, "P UA");
	ValveVacuumSet(ON);
	PumpVacuumSet(ON);
	osDelay(POWER_TEST_ON_TIME);
	PumpVacuumSet(OFF);
	ValveVacuumSet(OFF);
	osDelay(POWER_TEST_PAUSE_TIME);
	tCount++;
	Print("%2d - VACUUM VALVE\n", tCount);

	DigitsWrite(2, "P St");
	ValveWaterSteamSet(ON);
	PumpSteamGeneratorSet(ON);
	osDelay(POWER_TEST_ON_TIME);
	PumpSteamGeneratorSet(OFF);
	ValveWaterSteamSet(OFF);
	osDelay(POWER_TEST_PAUSE_TIME);
	tCount++;
	Print("%2d - WATER STEAM VALVE\n", tCount);

	DigitsWrite(2, "SOLE");
	HAL_GPIO_WritePin(SOLENOID_V1_GPIO_Port, SOLENOID_V1_Pin, OFF);
	HAL_GPIO_WritePin(SOLENOID_V2_GPIO_Port, SOLENOID_V2_Pin, OFF);
	// LOCK
	HAL_GPIO_WritePin(SOLENOID_V2_GPIO_Port, SOLENOID_V2_Pin, ON);
	osDelay(POWER_TEST_ON_TIME);

	HAL_GPIO_WritePin(SOLENOID_V1_GPIO_Port, SOLENOID_V1_Pin, OFF);
	HAL_GPIO_WritePin(SOLENOID_V2_GPIO_Port, SOLENOID_V2_Pin, OFF);
	// UNLOCK
	HAL_GPIO_WritePin(SOLENOID_V1_GPIO_Port, SOLENOID_V1_Pin, ON);
	osDelay(POWER_TEST_ON_TIME);
	// All off afetr LOCK/UNLOCK impulse
	HAL_GPIO_WritePin(SOLENOID_V1_GPIO_Port, SOLENOID_V1_Pin, OFF);
	HAL_GPIO_WritePin(SOLENOID_V2_GPIO_Port, SOLENOID_V2_Pin, OFF);
	tCount++;
	Print("%2d - LOCK/UNLOCK SOLENOID\n", tCount);

	DigitsWrite(2, "----");
	PWMPower_Start();
	osSemaphoreRelease(semaphoreSensorsHandle);
	osThreadResume(displayInfoTaskHandle);

	Print("\n");
	Print("******************************************\n");
	Print("\n\n\n\n\n");

}

uint8_t ProgramWash() {

	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

	ResetTimer(TIMER_CYCLE);

	if (!PhaseBegin(600)) return 0;

	GetDateTime(&clockTime);
	PrintHeader("CLEANING PROGRAM", sensors.T1, 0, 0, &clockTime);

	PhaseClean();

	// End
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	machineStatus.Status = STATUS_READY;

	ResetTimer(TIMER_FAN);

	progRunning = 0;
	osThreadSuspend(displayInfoTaskHandle);
	EEPROM_Write(EPA_LAST_CLEAN_CYCLE, &settings.CyclesCount, 2);
	doorOpened = 0;

	progStop = 0;

}

uint8_t ProgramWaterLoad() {

	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	if (!settings.FunWaterLoadActive) {
		Beep(2, 100, 100);
		return;
	}
	machineStatus.Status = STATUS_RUNNING;
	progRunning = 1;
	osThreadResume(displayInfoTaskHandle);

	progStop = 0;

//	if (!PhaseBegin(600)) return 0;

	ResetTimer(TIMER_CYCLE);

//	if (!PhaseBegin(600)) return 0;

//	GetDateTime(&clockTime);
//	PrintHeader("CLEANING PROGRAM", sensors.T1, &clockTime);

	PhaseWaterLoad(300);

	// End
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
//	PhaseFinish(success, &clockTime, &tCycle);
	PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
	machineStatus.Status = STATUS_READY;

	progRunning = 0;
	osThreadSuspend(displayInfoTaskHandle);
	progStop = 0;

}

//uint8_t ProgramTestSteril() {
//
//	uint8_t success = 1;
//	RTCDateTime_s clockTime;
//	RTCTimerTime_s tCycle, tPhase;
//
//	progRunning = 1;
//	progStop = 0;
//
//	if (!PhaseBegin(600)) return 0;
//
//	GetDateTime(&clockTime);
//	PrintHeader("QUICK CYCLE", sensors.T1, &clockTime);
//
//	// Heat up
//	if (success) {
//		success = PhaseHeatUp(130.0, 200.0, 0.0, 160.0, OT_HEAT);
//	}
//
//	ResetTimer(TIMER_CYCLE);
//
//	// Sterilization
//	if (success) {
//		PhasePreheat(130, 200);
//		success = PhaseVacuum(1, 20.0, OT_VACUUM);
//	}
//	if (success) {
//		PhasePreheat(130, 220);
//		success = PhasePressure(3, 290.0, 200.0, 50.0, OT_PRESSURE);
//	}
//	if (success) {
//		PhasePreheat(130, 220);
//		SterilParam_s sterilParam = {
//				.chamberTemp = DV_STERIL_TEMP_134,
//				.chamberTempMin = 134.0,
//				.chamberTempMax = 136.0,
//				.sterilTime = 210,
//				.chamberPress = 320,
//				.pressMin = 290.0,
//				.pressMax = 350.0,
//				.steamTemp = 200.0,
//				.steamTempMin = 170.0,
//				.printInterval = 30,
//				.timeout = 800
//		};
//		success = PhaseSteril(&sterilParam);
//	}
//	if (success) {
//		PhasePreheat(130, 200);
//		success = PhaseRelease(3, OT_RELEASE);
//	}
//
//	// Dry
//	if (success) {
//		PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);
//		success = PhaseDry(20.0, 50.0, 30, 800);
//	}
//
//	// End
//	GetTimer(TIMER_CYCLE, &tCycle);
//	GetDateTime(&clockTime);
//	PhaseFinish(success, &clockTime, &tCycle);
//
//	progRunning = 0;
//	progStop = 0;
//
//}
