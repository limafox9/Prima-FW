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
#include "panel_serial.h"
#include "Inc/DB_Protocol.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "time.h"
#include "sensors.h"
#include "printer.h"

#define OT_HEAT		1200
#define OT_VACUUM	240
#define OT_PRESSURE 600
#define OT_RELEASE	120

extern osThreadId sensorsTaskHandle;

extern s_Sensors Sensors;

uint8_t currProgram = 1;
uint8_t currTest = 1;
uint8_t currProgType = 1; // 1 = Program, 0 = Test

uint8_t progRunning = 0;
uint8_t progStop = 0;

void SetProgram() {
	if (!progRunning) {
		if (currProgType == 1) currProgram++;
		currProgType = 1;
		if (currProgram > PROGRAMS_N) currProgram = 1;
		char s[8];
		sprintf(s,"Pr. %d", currProgram);
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, s);
	}
}

void SetTest() {
	if (!progRunning) {
		if (currProgType == 0) currTest++;
		currProgType = 0;
		if (currTest > TESTS_N) currTest = 1;
		char s[8];
		sprintf(s,"tE. %d", currTest);
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, s);
	}
}

void StartProgram() {
	if (progRunning) {
//		progStop = 1;
	} else {
		if (currProgType) {
			switch (currProgram) {
			case 1:
				ProgramUnwrapped();
				break;
			case 2:
				ProgramWrapped();
				break;
			case 3:
				ProgramPrion();
				break;
			case 4:
				ProgramGentle();
				break;
			}
		} else {
			switch (currTest) {
			case 1:
				ProgramHelixTest();
				break;
			case 2:
				ProgramVacuumTest();
				break;
			}
		}
	}
}

uint8_t ProgramUnwrapped() {

	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

	FanSet(ON);
	SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_HEATING, 0, 0, "");

	GetDateTime(&clockTime);
	PrintHeader("UNWRAPPED", Sensors.T1, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(130.0, 200.0, 50.0, OT_HEAT);
	}

	ResetTimer(TIMER_CYCLE);

	// Sterilization
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseVacuum(1, 18.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(3, 290.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(130, 190);
		SterilParam_s sterilParam = {
				.chamberTemp = 135.2,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = 300,
				.pressMin = 310.0,
				.pressMax = 340.0,
				.steamTemp = 190.0,
				.steamTempMin = 170.0,
				.printInterval = 30,
				.timeout = 800
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(3, 110.0, OT_RELEASE);
	}

	// Dry
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseDry(20.0, 50.0, 90.0, 540, 800);
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

	FanSet(ON);
	SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_HEATING, 0, 0, "");

	GetDateTime(&clockTime);
	PrintHeader("WRAPPED", Sensors.T1, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(130.0, 200.0, 50.0, OT_HEAT);
	}

	ResetTimer(TIMER_CYCLE);

	// First vacuum
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseVacuum(1, 18.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(1, 170.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(1, 110.0, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < 2; i++) {
		if (success) {
			PhasePreheat(130.0, 200.0);
			success = PhaseVacuum(2, 30.0, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(130.0, 220.0);
			success = PhasePressure(2, 170.0, 200.0, 180.0, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(130.0, 200.0);
			success = PhaseRelease(2, 110.0, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, 30.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(3, 290.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(130, 190);
		SterilParam_s sterilParam = {
				.chamberTemp = 135.2,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = 300,
				.pressMin = 310.0,
				.pressMax = 340.0,
				.steamTemp = 190.0,
				.steamTempMin = 170.0,
				.printInterval = 30,
				.timeout = 800
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(3, 110.0, OT_RELEASE);
	}

	// Dry
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseDry(20.0, 50.0, 90.0, 540, 800);
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
	SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_HEATING, 0, 0, "");

	GetDateTime(&clockTime);
	PrintHeader("PRION", Sensors.T1, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(130.0, 200.0, 50.0, OT_HEAT);
	}

	ResetTimer(TIMER_CYCLE);

	// First vacuum
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseVacuum(1, 18.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(1, 170.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(1, 110.0, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < 2; i++) {
		if (success) {
			PhasePreheat(130.0, 200.0);
			success = PhaseVacuum(2, 30.0, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(130.0, 220.0);
			success = PhasePressure(2, 170.0, 200.0, 180.0, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(130.0, 200.0);
			success = PhaseRelease(2, 110.0, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, 30.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(3, 290.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(130, 190);
		SterilParam_s sterilParam = {
				.chamberTemp = 135.2,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = 1080,
				.pressMin = 310.0,
				.pressMax = 350.0,
				.steamTemp = 190.0,
				.steamTempMin = 170.0,
				.printInterval = 120,
				.timeout = 1500
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(3, 110.0, OT_RELEASE);
	}

	// Dry
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseDry(20.0, 50.0, 90.0, 540, 600);
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

	FanSet(ON);
	SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_HEATING, 0, 0, "");

	GetDateTime(&clockTime);
	PrintHeader("GENTLE", Sensors.T1, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(118.0, 200.0, 50.0, OT_HEAT);
	}

	ResetTimer(TIMER_CYCLE);

	// First vacuum
	if (success) {
		PhasePreheat(118, 200);
		success = PhaseVacuum(1, 18.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(118, 220);
		success = PhasePressure(1, 170.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(118, 200);
		success = PhaseRelease(1, 110.0, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < 2; i++) {
		if (success) {
			PhasePreheat(118.0, 200.0);
			success = PhaseVacuum(2, 30.0, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(118.0, 220.0);
			success = PhasePressure(2, 170.0, 200.0, 180.0, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(118.0, 200.0);
			success = PhaseRelease(2, 110.0, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, 30.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(3, 190.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(130, 190);
		SterilParam_s sterilParam = {
				.chamberTemp = 123.2,
				.chamberTempMin = 121.0,
				.chamberTempMax = 125.0,
				.sterilTime = 1200,
				.pressMin = 210.0,
				.pressMax = 240.0,
				.steamTemp = 190.0,
				.steamTempMin = 170.0,
				.printInterval = 120,
				.timeout = 1800
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(3, 110.0, OT_RELEASE);
	}

	// Dry
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseDry(20.0, 50.0, 90.0, 540, 600);
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

	FanSet(ON);
	SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_HEATING, 0, 0, "");

	GetDateTime(&clockTime);
	PrintHeader("HELIX TEST", Sensors.T1, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(130.0, 200.0, 50.0, OT_HEAT);
	}

	ResetTimer(TIMER_CYCLE);

	// First vacuum
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseVacuum(1, 18.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(1, 170.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(1, 110.0, OT_RELEASE);
	}

	// Other vacuum
	for (int i = 0; i < 1; i++) {
		if (success) {
			PhasePreheat(130.0, 200.0);
			success = PhaseVacuum(2, 30.0, OT_VACUUM);
		}
		if (success) {
			PhasePreheat(130.0, 220.0);
			success = PhasePressure(2, 170.0, 200.0, 180.0, OT_PRESSURE);
		}
		if (success) {
			PhasePreheat(130.0, 200.0);
			success = PhaseRelease(2, 110.0, OT_RELEASE);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, 20.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(3, 290.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(130, 190);
		SterilParam_s sterilParam = {
				.chamberTemp = 135.2,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = 210,
				.pressMin = 310.0,
				.pressMax = 350.0,
				.steamTemp = 190.0,
				.steamTempMin = 170.0,
				.printInterval = 30,
				.timeout = 600
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(3, 110.0, OT_RELEASE);
	}

	// Dry
	if (success) {
		PhasePreheat(60, 90);
		success = PhaseDry(20.0, 50.0, 90.0, 300, 600);
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

	FanSet(ON);
	SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_HEATING, 0, 0, "");

	ResetTimer(TIMER_CYCLE);

	success = PhaseBegin(600);

	GetDateTime(&clockTime);
	PrintHeader("VACUUM TEST", Sensors.T1, &clockTime);

	if (success) {
		success = PhaseVacuum(1, 18.0, OT_VACUUM);
	}
	if (success) {
		success = PhaseWait(240);
	}
	if (success) {
		success = PhaseVacuumHold(18.0, 20.0, 240, 600);
	}
	if (success) {
		success = PhaseRelease(1, 110.0, OT_RELEASE);
	}

	// End
	GetTimer(TIMER_CYCLE, &tCycle);
	GetDateTime(&clockTime);
	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
	progStop = 0;

}

uint8_t ProgramTestSteril() {

	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

	FanSet(ON);
	SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_HEATING, 0, 0, "");

	GetDateTime(&clockTime);
	PrintHeader("TEST STERIL", Sensors.T1, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(130.0, 200.0, 50.0, OT_HEAT);
	}

	ResetTimer(TIMER_CYCLE);

	// Sterilization
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseVacuum(1, 18.0, OT_VACUUM);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(3, 290.0, 200.0, 180.0, OT_PRESSURE);
	}
	if (success) {
		PhasePreheat(130, 200);
		SterilParam_s sterilParam = {
				.chamberTemp = 135.2,
				.chamberTempMin = 134.0,
				.chamberTempMax = 136.0,
				.sterilTime = 300,
				.pressMin = 310.0,
				.pressMax = 340.0,
				.steamTemp = 200.0,
				.steamTempMin = 170.0,
				.printInterval = 30,
				.timeout = 800
		};
		success = PhaseSteril(&sterilParam);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(3, 110.0, OT_RELEASE);
	}

//	// Dry
//	if (success) {
//		PhasePreheat(60, 90);
//		success = PhaseDry(20.0, 50.0, 90.0, 540, 800);
//	}

	// End
//	GetTimer(TIMER_CYCLE, &tCycle);
//	GetDateTime(&clockTime);
//	PhaseFinish(success, &clockTime, &tCycle);

	progRunning = 0;
	progStop = 0;

}
