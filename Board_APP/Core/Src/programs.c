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
			case 2:
				ProgramWrapped();
			case 3:
				ProgramPrion();
			case 4:
				ProgramGentle();
			}
		} else {
			switch (currTest) {
				case 1:
					ProgramHelixTest();
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

	ResetTimer(TIMER_CYCLE);

	GetDateTime(&clockTime);
	PrintHeader("UNWRAPPED", Sensors.T1, &clockTime);
	// Heat up
	if (success) {
		success = PhaseHeatUp(130.0, 200.0, 50.0, 600);
	}

	// Sterilization
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseVacuum(1, 18.0, 600);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(3, 290.0, 200.0, 180.0, 600);
	}
	if (success) {
		PhasePreheat(130, 190);
		success = PhaseSteril(135.2, 134.0, 136.0, 300, 310.0, 350.0, 190.0, 170.0, 800);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(3, 110.0, 600);
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

uint8_t ProgramWrapped() {
	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

	FanSet(ON);
	SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_HEATING, 0, 0, "");

	ResetTimer(TIMER_CYCLE);

	GetDateTime(&clockTime);
	PrintHeader("WRAPPED", Sensors.T1, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(130.0, 200.0, 50.0, 600);
	}

	// First vacuum
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseVacuum(1, 18.0, 600);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(1, 170.0, 200.0, 180.0, 600);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(1, 110.0, 600);
	}

	// Other vacuum
	for (int i = 0; i < 2; i++) {
		if (success) {
			PhasePreheat(130.0, 200.0);
			success = PhaseVacuum(2, 30.0, 600);
		}
		if (success) {
			PhasePreheat(130.0, 220.0);
			success = PhasePressure(2, 170.0, 200.0, 180.0, 600);
		}
		if (success) {
			PhasePreheat(130.0, 200.0);
			success = PhaseRelease(2, 110.0, 600);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, 30.0, 600);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(3, 290.0, 200.0, 180.0, 600);
	}
	if (success) {
		PhasePreheat(130, 190);
		success = PhaseSteril(135.2, 134.0, 136.0, 300, 310.0, 340.0, 190.0, 170.0, 800);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(3, 110.0, 600);
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

uint8_t ProgramPrion() {
	uint8_t success = 1;
	RTCDateTime_s clockTime;
	RTCTimerTime_s tCycle, tPhase;

	progRunning = 1;
	progStop = 0;

	FanSet(ON);
	SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_HEATING, 0, 0, "");

	ResetTimer(TIMER_CYCLE);

	GetDateTime(&clockTime);
	PrintHeader("PRION", Sensors.T1, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(130.0, 200.0, 50.0, 600);
	}

	// First vacuum
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseVacuum(1, 18.0, 600);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(1, 170.0, 200.0, 180.0, 600);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(1, 110.0, 600);
	}

	// Other vacuum
	for (int i = 0; i < 2; i++) {
		if (success) {
			PhasePreheat(130.0, 200.0);
			success = PhaseVacuum(2, 30.0, 600);
		}
		if (success) {
			PhasePreheat(130.0, 220.0);
			success = PhasePressure(2, 170.0, 200.0, 180.0, 600);
		}
		if (success) {
			PhasePreheat(130.0, 200.0);
			success = PhaseRelease(2, 110.0, 600);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, 30.0, 600);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(3, 290.0, 200.0, 180.0, 600);
	}
	if (success) {
		PhasePreheat(130, 190);
		success = PhaseSteril(135.2, 134.0, 136.0, 1080, 310.0, 340.0, 190.0, 170.0, 1500);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(3, 110.0, 600);
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

	ResetTimer(TIMER_CYCLE);

	GetDateTime(&clockTime);
	PrintHeader("GENTLE", Sensors.T1, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(130.0, 200.0, 50.0, 600);
	}

	// First vacuum
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseVacuum(1, 18.0, 600);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(1, 170.0, 200.0, 180.0, 600);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(1, 110.0, 600);
	}

	// Other vacuum
	for (int i = 0; i < 2; i++) {
		if (success) {
			PhasePreheat(130.0, 200.0);
			success = PhaseVacuum(2, 30.0, 600);
		}
		if (success) {
			PhasePreheat(130.0, 220.0);
			success = PhasePressure(2, 170.0, 200.0, 180.0, 600);
		}
		if (success) {
			PhasePreheat(130.0, 200.0);
			success = PhaseRelease(2, 110.0, 600);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, 30.0, 600);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(3, 190.0, 200.0, 180.0, 600);
	}
	if (success) {
		PhasePreheat(130, 190);
		success = PhaseSteril(123.0, 121.0, 125.0, 1200, 210.0, 240.0, 190.0, 170.0, 2000);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(3, 110.0, 600);
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

	ResetTimer(TIMER_CYCLE);

	GetDateTime(&clockTime);
	PrintHeader("HELIX TEST", Sensors.T1, &clockTime);

	// Heat up
	if (success) {
		success = PhaseHeatUp(130.0, 200.0, 50.0, 600);
	}

	// First vacuum
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseVacuum(1, 18.0, 600);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(1, 170.0, 200.0, 180.0, 600);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(1, 110.0, 600);
	}

	// Other vacuum
	for (int i = 0; i < 1; i++) {
		if (success) {
			PhasePreheat(130.0, 200.0);
			success = PhaseVacuum(2, 30.0, 600);
		}
		if (success) {
			PhasePreheat(130.0, 220.0);
			success = PhasePressure(2, 170.0, 200.0, 180.0, 600);
		}
		if (success) {
			PhasePreheat(130.0, 200.0);
			success = PhaseRelease(2, 110.0, 600);
		}
	}

	// Sterilization
	if (success) {
		success = PhaseVacuum(3, 20.0, 600);
	}
	if (success) {
		PhasePreheat(130, 220);
		success = PhasePressure(3, 290.0, 200.0, 180.0, 600);
	}
	if (success) {
		PhasePreheat(130, 190);
		success = PhaseSteril(135.2, 134.0, 136.0, 210, 310.0, 350.0, 190.0, 170.0, 800);
	}
	if (success) {
		PhasePreheat(130, 200);
		success = PhaseRelease(3, 110.0, 600);
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
