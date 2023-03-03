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

#define OT_HEAT		1200
#define OT_VACUUM	300
#define OT_PRESSURE 600
#define OT_RELEASE	120

#define DV_PREHEAT_CHAMBER_TEMP	60.0
#define DV_HEATING_RING_TEMP	120.0
#define DV_STEAM_GEN_TEMP		200.0
#define DV_WATER_LOAD_TIMEOUT	300

#define DV_STERIL_TEMP_134		135.4
#define DV_STERIL_TEMP_121		123.2
#define DV_STERIL_PRESS_MIN_134	290.0
#define DV_STERIL_PRESS_MAX_134	350.0
#define DV_STERIL_PRESS_MIN_121	200.0
#define DV_STERIL_PRESS_MAX_121	250.0

#define VAC_DVALUE	10
#define VAC_DTIME	20

extern osThreadId sensorsTaskHandle;

extern Sensors_s sensors;
uint8_t doorStatus;

uint8_t programStart = 0;

void StartProgramTask() {
	RTCTimerTime_s timerDoor;
	doorStatus = (HAL_GPIO_ReadPin(SW_DOOR_GPIO_Port, SW_DOOR_Pin) == GPIO_PIN_SET) ? OPEN : CLOSED;
	for (;;) {

		GetTimer(TIMER_DOOR, &timerDoor);
		if ((doorStatus == OPEN) && (timerDoor.Seconds > 10)) {
			ValveDryIntakeSet(OFF);
		}

		if (programStart){
			programStart = 0;
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
				case PROGRAM_RESUME_DRY:
					ProgramResumeDry();
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
		osDelay(300);
	}
}

void ProgramOpenDoorRelease() {

}

uint8_t ProgramResumeDry() {

//	uint8_t success = 1;
//	RTCDateTime_s clockTime;
//	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

//	PhaseResume(600);
	FanSet(ON);

	sprintf(currPhase, "rEL.");

	while (1) {
		if (sensors.PressureChamber <= pressAmbientMax) {
			break;
		}
		osDelay(200);
	}

	PhaseDry(20.0, 50.0, 180, 800);
	SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, LED_PROGRESS_NONE, 0, 0, "");

	ValveVacuumSet(OFF);
	ValveWaterSteamSet(OFF);

	PumpSteamGeneratorSet(OFF);
	PumpVacuumSet(OFF);
	PumpWaterLoadSet(OFF);

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

	FanSet(OFF);

	machineStatus.Status = STATUS_READY;

	osDelay(1000);
	if (DoorSolenoidSet(UNLOCK) != UNLOCK) {
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "Er.11");
		return 0;
	}

	PhasePreheat(0.0, 0.0);
	progRunning = 0;
	progStop = 0;

}

uint8_t ProgramUnwrapped() {

	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

	if (!PhaseBegin(600)) return 0;

	GetDateTime(&clockTime);
	PrintHeader("UNWRAPPED", sensors.T1, &clockTime);

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
		success = PhaseVacuum(1, (float)settings.FirstVacuum, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP+ 10);
		success = PhasePressure(3, 290.0, DV_STEAM_GEN_TEMP, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		SterilParam_s sterilParam = {
				.chamberTemp = DV_STERIL_TEMP_134,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = 300,
				.chamberPress = 320,
				.pressMin = 290.0,
				.pressMax = 350.0,
				.steamTemp = DV_STEAM_GEN_TEMP,
				.steamTempMin = 170.0,
				.printInterval = 30,
				.timeout = 800
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseRelease(3, OT_RELEASE);
	}

	// Dry
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseDry(20.0, 50.0, 240, 800);
	}

	// End
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
	progStop = 0;

}

uint8_t ProgramWrapped() {
	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

	if (!PhaseBegin(600)) return 0;

	GetDateTime(&clockTime);
	PrintHeader("WRAPPED", sensors.T1, &clockTime);

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
		success = PhaseVacuum(1, (float)settings.FirstVacuum, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(1, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP, 170.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseRelease(1, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < (settings.VacNumber - 2); i++) {  // -2 for 1st cycle and steril cycle
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseVacuum(2, (float)settings.OtherVacuum, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
			success = PhasePressure(2, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP, 170.0, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseRelease(2, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 210.0);
		success = PhaseVacuum(3, (float)settings.OtherVacuum, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(3, 290.0, DV_STEAM_GEN_TEMP, 170.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		SterilParam_s sterilParam = {
				.chamberTemp = DV_STERIL_TEMP_134,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = 300,
				.chamberPress = 320,
				.pressMin = 290.0,
				.pressMax = 350.0,
				.steamTemp = DV_STEAM_GEN_TEMP,
				.steamTempMin = 170.0,
				.printInterval = 30,
				.timeout = 800
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseRelease(3, OT_RELEASE);
	}

	// Dry
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseDry(20.0, 50.0, 540, 800);
	}

	// End
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
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

	GetDateTime(&clockTime);
	PrintHeader("PRION", sensors.T1, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP, DV_PREHEAT_CHAMBER_TEMP, 160.0, OT_HEAT);
	}

	ResetTimer(TIMER_CYCLE);

	// First vacuum
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseVacuum(1, (float)settings.FirstVacuum, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(1, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseRelease(1, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < (settings.VacNumber - 2); i++) {  // -2 for 1st cycle and steril cycle
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseVacuum(2, (float)settings.OtherVacuum, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
			success = PhasePressure(2, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP, 180.0, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseRelease(2, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, (float)settings.OtherVacuum, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(3, 290.0, DV_STEAM_GEN_TEMP, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		SterilParam_s sterilParam = {
				.chamberTemp = DV_STERIL_TEMP_134,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = 1080,
				.chamberPress = 320,
				.pressMin = 290.0,
				.pressMax = 350.0,
				.steamTemp = DV_STEAM_GEN_TEMP,
				.steamTempMin = 170.0,
				.printInterval = 120,
				.timeout = 1500
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseRelease(3, OT_RELEASE);
	}

	// Dry
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseDry(20.0, 50.0, 540, 600);
	}

	// End
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
	progStop = 0;

}

uint8_t ProgramGentle() {
	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

	if (!PhaseBegin(600)) return 0;

	GetDateTime(&clockTime);
	PrintHeader("GENTLE", sensors.T1, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP, DV_PREHEAT_CHAMBER_TEMP, 160.0, OT_HEAT);
	}

	ResetTimer(TIMER_CYCLE);

	// First vacuum
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseVacuum(1, (float)settings.FirstVacuum, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(1, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseRelease(1, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < (settings.VacNumber - 2); i++) {  // -2 for 1st cycle and steril cycle
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseVacuum(2, (float)settings.OtherVacuum, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
			success = PhasePressure(2, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP, 180.0, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseRelease(2, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, (float)settings.OtherVacuum, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(3, 190.0, DV_STEAM_GEN_TEMP, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		SterilParam_s sterilParam = {
				.chamberTemp = DV_STERIL_TEMP_121,
				.chamberTempMin = 121.0,
				.chamberTempMax = 125.0,
				.sterilTime = 1200,
				.chamberPress = 220,
				.pressMin = 200.0,
				.pressMax = 250.0,
				.steamTemp = DV_STEAM_GEN_TEMP,
				.steamTempMin = 170.0,
				.printInterval = 120,
				.timeout = 1800
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseRelease(3,  OT_RELEASE);
	}

	// Dry
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseDry(20.0, 50.0, 540, 600);
	}

	// End
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
	progStop = 0;

}

uint8_t ProgramUser() {
	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

	if (!PhaseBegin(600)) return 0;

	GetDateTime(&clockTime);
	PrintHeader("USER", sensors.T1, &clockTime);

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
	if (settings.UserVacuumNumber > 0) {
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseVacuum(1, (float)settings.FirstVacuum, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
			success = PhasePressure(1, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP, 170.0, OT_PRESSURE);
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
				success = PhaseVacuum(2, (float)settings.OtherVacuum, OT_VACUUM);
			}
			if (success) {
				PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
				success = PhasePressure(2, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP, 170.0, OT_PRESSURE);
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
			success = PhaseVacuum(3, (float)settings.OtherVacuum, OT_VACUUM);
		}
	}
	if (settings.UserVacuumNumber < 3) {
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
			success = PhasePressure(3, (settings.UserSterilTemp == STERILTEMP_134) ? 200.0 : 180.0, DV_STEAM_GEN_TEMP, 170.0, OT_PRESSURE);
		}
		ValveDrainSteamSet(OFF);
		osDelay(20000);
		ValveDrainSteamSet(ON);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(3, (settings.UserSterilTemp == STERILTEMP_134) ? 290.0 : 190.0, DV_STEAM_GEN_TEMP, 170.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		SterilParam_s sterilParam = {
				.chamberTemp = (settings.UserSterilTemp == STERILTEMP_134) ? DV_STERIL_TEMP_134 : DV_STERIL_TEMP_121,
				.chamberTempMin = (settings.UserSterilTemp == STERILTEMP_134) ? 134.0 : 121.0,
				.chamberTempMax = (settings.UserSterilTemp == STERILTEMP_134) ? 136.0 : 125.0,
				.sterilTime = settings.UserSterilTime * 60,
				.chamberPress = (settings.UserSterilTemp == STERILTEMP_134) ? 320.0 : 220.0,
				.pressMin = (settings.UserSterilTemp == STERILTEMP_134) ? DV_STERIL_PRESS_MIN_134 : DV_STERIL_PRESS_MIN_121,
				.pressMax = (settings.UserSterilTemp == STERILTEMP_134) ? DV_STERIL_PRESS_MAX_134 : DV_STERIL_PRESS_MAX_121,
				.steamTemp = DV_STEAM_GEN_TEMP,
				.steamTempMin = 170.0,
				.printInterval = 30,
				.timeout = settings.UserSterilTime * 60 * 1.5,
		};

		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseRelease(3, OT_RELEASE);
	}

	// Dry
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseDry(20.0, 50.0, settings.UserDryTime * 60, settings.UserDryTime * 60 * 1.5);
	}

	// End
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
	progStop = 0;

}

uint8_t ProgramHelixTest() {
	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

	if (!PhaseBegin(600)) return 0;

	GetDateTime(&clockTime);
	PrintHeader("HELIX TEST", sensors.T1, &clockTime);

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
		success = PhaseVacuum(1, (float)settings.FirstVacuum, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(1, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
		success = PhaseRelease(1, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < 1; i++) {
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseVacuum(2, (float)settings.OtherVacuum, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
			success = PhasePressure(2, (float)settings.OtherPressure * 10.0, DV_STEAM_GEN_TEMP, 180.0, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP);
			success = PhaseRelease(2, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, (float)settings.OtherVacuum, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		success = PhasePressure(3, 290.0, DV_STEAM_GEN_TEMP, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, DV_STEAM_GEN_TEMP + 10);
		SterilParam_s sterilParam = {
				.chamberTemp = DV_STERIL_TEMP_134,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = 210, // 210
				.chamberPress = 320,
				.pressMin = 290.0,
				.pressMax = 350.0,
				.steamTemp = DV_STEAM_GEN_TEMP,
				.steamTempMin = 170.0,
				.printInterval = 30,
				.timeout = 600
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseRelease(3, OT_RELEASE);
	}

	// Dry
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseDry(20.0, 50.0, 300, 600);
	}

	// End
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
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

	if (!PhaseBegin(600)) return 0;

	GetDateTime(&clockTime);
	PrintHeader("VACUUM TEST", sensors.T1, &clockTime);

	if (success) {
		success = PhaseVacuum(1, 20.0, OT_VACUUM);
	}
	if (success) {
		success = PhaseWait(240);
	}
	if (success) {
		success = PhaseVacuumHold(18.0, 20.0, 240, 600);
	}
	if (success) {
		success = PhaseRelease(1, OT_RELEASE);
	}

	// End
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
	progStop = 0;

}

uint8_t ProgramWash() {

	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

//	if (!PhaseBegin(600)) return 0;

	ResetTimer(TIMER_CYCLE);

	if (!PhaseBegin(600)) return 0;

	GetDateTime(&clockTime);
	PrintHeader("CLEANING PROGRAM", sensors.T1, &clockTime);

	PhaseClean();

	// End
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
	progStop = 0;

}

uint8_t ProgramWaterLoad() {

	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
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
	PhasePreheat(0.0, 0.0);

	progRunning = 0;
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
//		PhasePreheat(60, 90);
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
