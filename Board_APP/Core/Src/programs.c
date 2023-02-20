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
#define DV_WATER_LOAD_TIMEOUT	300

#define VAC_DVALUE	10
#define VAC_DTIME	20

extern osThreadId sensorsTaskHandle;

extern Sensors_s sensors;


uint8_t programStart = 0;

void StartProgramTask() {

	for (;;) {
		if (programStart){
			programStart = 0;
			if (currProgType == PROGTYPE_PROGRAM) {
				switch (currProgram) {
				case 0:
					ProgramUnwrapped();
					break;
				case 1:
					ProgramWrapped();
					break;
				case 2:
					ProgramPrion();
					break;
				case 3:
					ProgramGentle();
					break;
				case 4:
					//ProgramUser();
					break;
				case 5:
					ProgramTestSteril();
					break;
				case 99:
					ProgramResumeDry();
					break;
				}
			} else {
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

uint8_t ProgramResumeDry() {

//	uint8_t success = 1;
//	RTCDateTime_s clockTime;
//	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

//	PhaseResume(600);
	FanSet(ON);

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

	lastError = ERROR_NONE;
	EEPROM_Write(EPA_LAST_ERROR, &lastError, 1);

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
		success = PhaseHeatUp(DV_HEATING_RING_TEMP, 200.0, DV_PREHEAT_CHAMBER_TEMP, 160.0, OT_HEAT);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 200.0);
		success = PhaseRelease(1, OT_RELEASE);
	}

	ResetTimer(TIMER_CYCLE);

	// Sterilization
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 200);
		success = PhaseVacuum(1, 20.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 220);
		success = PhasePressure(3, 290.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 220);
		SterilParam_s sterilParam = {
				.chamberTemp = 135.2,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = 300,
				.chamberPress = 320,
				.pressMin = 290.0,
				.pressMax = 350.0,
				.steamTemp = 200.0,
				.steamTempMin = 170.0,
				.printInterval = 30,
				.timeout = 800
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 200);
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
		success = PhaseHeatUp(DV_HEATING_RING_TEMP, 200.0, DV_PREHEAT_CHAMBER_TEMP, 100.0, OT_HEAT);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 200.0);
		success = PhaseRelease(1, OT_RELEASE);
	}

	ResetTimer(TIMER_CYCLE);

	// First vacuum
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 210.0);
		success = PhaseVacuum(1, 20.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 220.0);
		success = PhasePressure(1, 170.0, 200.0, 170.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 200.0);
		success = PhaseRelease(1, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < 2; i++) {
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, 210.0);
			success = PhaseVacuum(2, 30.0, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, 220.0);
			success = PhasePressure(2, 170.0, 200.0, 170.0, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, 200.0);
			success = PhaseRelease(2, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 210.0);
		success = PhaseVacuum(3, 30.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 220.0);
		success = PhasePressure(3, 290.0, 200.0, 170.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 220.0);
		SterilParam_s sterilParam = {
				.chamberTemp = 135.6,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = 300,
				.chamberPress = 320,
				.pressMin = 290.0,
				.pressMax = 350.0,
				.steamTemp = 200.0,
				.steamTempMin = 170.0,
				.printInterval = 30,
				.timeout = 800
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 200.0);
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
		success = PhaseHeatUp(DV_HEATING_RING_TEMP, 200.0, DV_PREHEAT_CHAMBER_TEMP, 160.0, OT_HEAT);
	}

	ResetTimer(TIMER_CYCLE);

	// First vacuum
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 210);
		success = PhaseVacuum(1, 20.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 220);
		success = PhasePressure(1, 170.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 200);
		success = PhaseRelease(1, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < 2; i++) {
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, 210.0);
			success = PhaseVacuum(2, 30.0, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, 220.0);
			success = PhasePressure(2, 170.0, 200.0, 180.0, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, 200.0);
			success = PhaseRelease(2, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, 30.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 220);
		success = PhasePressure(3, 290.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 220);
		SterilParam_s sterilParam = {
				.chamberTemp = 135.6,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = 1080,
				.chamberPress = 320,
				.pressMin = 290.0,
				.pressMax = 350.0,
				.steamTemp = 200.0,
				.steamTempMin = 170.0,
				.printInterval = 120,
				.timeout = 1500
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 200);
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
		success = PhaseHeatUp(DV_HEATING_RING_TEMP, 200.0, DV_PREHEAT_CHAMBER_TEMP, 160.0, OT_HEAT);
	}

	ResetTimer(TIMER_CYCLE);

	// First vacuum
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 210);
		success = PhaseVacuum(1, 20.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 220);
		success = PhasePressure(1, 170.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 200);
		success = PhaseRelease(1, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < 2; i++) {
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, 210.0);
			success = PhaseVacuum(2, 30.0, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, 220.0);
			success = PhasePressure(2, 170.0, 200.0, 180.0, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, 200.0);
			success = PhaseRelease(2, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, 30.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 220);
		success = PhasePressure(3, 190.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 220);
		SterilParam_s sterilParam = {
				.chamberTemp = 123.2,
				.chamberTempMin = 121.0,
				.chamberTempMax = 125.0,
				.sterilTime = 1200,
				.chamberPress = 320,
				.pressMin = 200.0,
				.pressMax = 250.0,
				.steamTemp = 200.0,
				.steamTempMin = 170.0,
				.printInterval = 120,
				.timeout = 1800
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 200);
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
		success = PhaseHeatUp(DV_HEATING_RING_TEMP, 200.0, DV_PREHEAT_CHAMBER_TEMP, 160.0, OT_HEAT);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 200.0);
		success = PhaseRelease(1, OT_RELEASE);
	}

	ResetTimer(TIMER_CYCLE);

	// First vacuum
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 210);
		success = PhaseVacuum(1, 20.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 220);
		success = PhasePressure(1, 170.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 200);
		success = PhaseRelease(1, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < 1; i++) {
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, 210.0);
			success = PhaseVacuum(2, 30.0, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, 220.0);
			success = PhasePressure(2, 170.0, 200.0, 180.0, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(DV_HEATING_RING_TEMP, 200.0);
			success = PhaseRelease(2, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, 20.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 220);
		success = PhasePressure(3, 290.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 220);
		SterilParam_s sterilParam = {
				.chamberTemp = 135.5,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = 230, // 210
				.chamberPress = 320,
				.pressMin = 290.0,
				.pressMax = 350.0,
				.steamTemp = 200.0,
				.steamTempMin = 170.0,
				.printInterval = 30,
				.timeout = 600
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(DV_HEATING_RING_TEMP, 200);
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

	PhaseWaterLoad(6000);

	// End
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
//	PhaseFinish(success, &clockTime, &tCycle);
	PhasePreheat(0.0, 0.0);

	progRunning = 0;
	progStop = 0;

}

uint8_t ProgramTestSteril() {

	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

	if (!PhaseBegin(600)) return 0;

	GetDateTime(&clockTime);
	PrintHeader("QUICK CYCLE", sensors.T1, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(130.0, 200.0, 0.0, 160.0, OT_HEAT);
	}

	ResetTimer(TIMER_CYCLE);

	// Sterilization
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseVacuum(1, 20.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(3, 290.0, 200.0, 50.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(130, 220);
		SterilParam_s sterilParam = {
				.chamberTemp = 135.6,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = 210,
				.chamberPress = 320,
				.pressMin = 290.0,
				.pressMax = 350.0,
				.steamTemp = 200.0,
				.steamTempMin = 170.0,
				.printInterval = 30,
				.timeout = 800
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(3, OT_RELEASE);
	}

	// Dry
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseDry(20.0, 50.0, 30, 800);
	}

	// End
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
	progStop = 0;

}
