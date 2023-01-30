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
uint8_t currProgType; // 0 = Program, 1 = Test

uint8_t progRunning = 0;
uint8_t progStop = 0;

void SetProgram() {
	if (!progRunning) {
		if (currProgType == 0) currProgram++;
		currProgType = 0;
		if (currProgram > PROGRAMS_N) currProgram = 1;
		char s[8];
		sprintf(s,"Pr. %d", currProgram);
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, s);
		SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, currProgram, 0, 0, s);
	}
}

void SetTest() {
	if (!progRunning) {
		if (currProgType == 1) currTest++;
		currProgType = 1;
		if (currTest > TESTS_N) currTest = 1;
		char s[8];
		sprintf(s,"tE. %d", currTest);
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, s);
	}
}

void StartProgram() {
	if (progRunning) {
		progStop = 1;
	} else {
		if (!currProgType) {
			switch (currProgram) {
				case 1:
					ProgramWrapped();
			}
		} else {
			switch (currTest) {
				case 1:
					ProgramHelixTest();
			}
		}
	}
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

	PCF8563_GetDateTime(&clockTime);
	Print("********************************\n");
	PrintSize(2);
	Print("WRAPPED\n");
	PrintSize(1);
	Print("\n");
	Print("%d-%02d-%02d\n", clockTime.Year, clockTime.Month, clockTime.Day);
	Print("--------------------------------\n");
	Print("STEP    T1     T2   PRESS   TIME\n");
	Print("        oC     oC    kPa   mm:ss\n");
	Print("--------------------------------\n");
	Print("START %02d:%02d\n", clockTime.Hour, clockTime.Minute);

	// Heat up
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_HEATING, 0, 0, "");
		ResetTimer(TIMER_PHASE);
		success = PhaseHeatUp(130.0, 200.0, 50.0, 600);
		GetTimer(TIMER_PHASE, &tPhase);
		Print("HEAT   %03.1f %03.1f %03.1f   %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
	}

	// First vacuum
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_1STVACUUM, 0, 0, "");
		ResetTimer(TIMER_PHASE);
		PhasePreheat(130, 200);
		success = PhaseVacuum(18.0, 600);
		GetTimer(TIMER_PHASE, &tPhase);
		Print("VAC    %03.1f %03.1f %03.1f   %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
	}
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_1STPRESSURE, 0, 0, "");
		ResetTimer(TIMER_PHASE);
		PhasePreheat(130, 220);
		success = PhasePressure(170.0, 200.0, 180.0, 600);
		GetTimer(TIMER_PHASE, &tPhase);
		Print("PRES   %03.1f %03.1f %03.1f   %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
	}
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_NVACUUM, 0, 0, "");
		ResetTimer(TIMER_PHASE);
		PhasePreheat(130, 200);
		success = PhaseRelease(110.0, 600);
	}

	// Other vacuum
	for (int i = 0; i < 1; i++) {
		if (success) {
			ResetTimer(TIMER_PHASE);
			PhasePreheat(130.0, 200.0);
			success = PhaseVacuum(30.0, 600);
			GetTimer(TIMER_PHASE, &tPhase);
			Print("VAC    %03.1f %03.1f %03.1f   %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
		}
		if (success) {
			SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_NPRESSURE, 0, 0, "");
			ResetTimer(TIMER_PHASE);
			PhasePreheat(130.0, 220.0);
			success = PhasePressure(170.0, 200.0, 180.0, 600);
			GetTimer(TIMER_PHASE, &tPhase);
			Print("PRES   %03.1f %03.1f %03.1f   %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
		}
		if (success) {
			SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_NRELEASE, 0, 0, "");
			ResetTimer(TIMER_PHASE);
			PhasePreheat(130.0, 200.0);
			success = PhaseRelease(110.0, 600);
		}
	}

	// Sterilization
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_STERILVACUUM, 0, 0, "");
		PhasePreheat(130, 200);
		success = PhaseVacuum(20.0, 600);
		GetTimer(TIMER_PHASE, &tPhase);
		Print("VAC    %03.1f %03.1f %03.1f   %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
	}
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_STERILPRESSURE, 0, 0, "");
		ResetTimer(TIMER_PHASE);
		PhasePreheat(130, 220);
		success = PhasePressure(290.0, 200.0, 180.0, 600);
		GetTimer(TIMER_PHASE, &tPhase);
		Print("PRES   %03.1f %03.1f %03.1f   %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
	}
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_STERILMAINTAIN, 0, 0, "");
		ResetTimer(TIMER_PHASE);
		Print("STERIL -------------------------\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
		PhasePreheat(130, 190);
		success = PhaseSteril(135.2, 134.0, 136.0, 300, 310.0, 350.0, 190.0, 170.0, 800);
	}
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_STERILRELEASE, 0, 0, "");
		ResetTimer(TIMER_PHASE);
		PhasePreheat(130, 200);
		success = PhaseRelease(110.0, 600);
	}

	// Dry
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_DRY, 0, 0, "");
		success = PhaseDry(20.0, 50.0, 90.0, 540, 600);
		GetTimer(TIMER_PHASE, &tPhase);
		Print("DRY                        %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
	}

	GetTimer(TIMER_CYCLE, &tCycle);
	Print("--------------------------------\n");
	Print("TOTAL TIME                 %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tCycle.TimeMinute, tCycle.TimeSecond);

	// End

	if (success) {
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "PASS");
		Beep(3, 100, 100);
	} else {
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "FAIL");
		Beep(1, 1000, 100);
	}

	PhaseFinish();

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

	PCF8563_GetDateTime(&clockTime);
	Print("********************************\n");
	PrintSize(2);
	Print("HELIX TEST\n");
	PrintSize(1);
	Print("\n");
	Print("%d-%02d-%02d\n", clockTime.Year, clockTime.Month, clockTime.Day);
	Print("--------------------------------\n");
	Print("STEP    T1     T2   PRESS   TIME\n");
	Print("        oC     oC    kPa   mm:ss\n");
	Print("--------------------------------\n");
	Print("START %02d:%02d\n", clockTime.Hour, clockTime.Minute);

	// Heat up
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_HEATING, 0, 0, "");
		ResetTimer(TIMER_PHASE);
		success = PhaseHeatUp(130.0, 200.0, 50.0, 600);
		GetTimer(TIMER_PHASE, &tPhase);
		Print("HEATUP %03.1f %03.1f %03.1f   %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
	}

	// First vacuum
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_1STVACUUM, 0, 0, "");
		ResetTimer(TIMER_PHASE);
		PhasePreheat(130, 200);
		success = PhaseVacuum(18.0, 600);
		GetTimer(TIMER_PHASE, &tPhase);
		Print("VACUUM %03.1f %03.1f %03.1f   %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
	}
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_1STPRESSURE, 0, 0, "");
		ResetTimer(TIMER_PHASE);
		PhasePreheat(130, 220);
		success = PhasePressure(170.0, 200.0, 180.0, 600);
		GetTimer(TIMER_PHASE, &tPhase);
		Print("PRESS  %03.1f %03.1f %03.1f   %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
	}
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_NVACUUM, 0, 0, "");
		ResetTimer(TIMER_PHASE);
		PhasePreheat(130, 200);
		success = PhaseRelease(110.0, 600);
	}

	// Other vacuum
	for (int i = 0; i < 1; i++) {
		if (success) {
			ResetTimer(TIMER_PHASE);
			PhasePreheat(130.0, 200.0);
			success = PhaseVacuum(30.0, 600);
			GetTimer(TIMER_PHASE, &tPhase);
			Print("VACUUM %03.1f %03.1f %03.1f   %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
		}
		if (success) {
			SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_NPRESSURE, 0, 0, "");
			ResetTimer(TIMER_PHASE);
			PhasePreheat(130.0, 220.0);
			success = PhasePressure(170.0, 200.0, 180.0, 600);
			GetTimer(TIMER_PHASE, &tPhase);
			Print("PRESS  %03.1f %03.1f %03.1f   %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
		}
		if (success) {
			SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_NRELEASE, 0, 0, "");
			ResetTimer(TIMER_PHASE);
			PhasePreheat(130.0, 200.0);
			success = PhaseRelease(110.0, 600);
		}
	}

	// Sterilization
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_STERILVACUUM, 0, 0, "");
		PhasePreheat(130, 200);
		success = PhaseVacuum(20.0, 600);
		GetTimer(TIMER_PHASE, &tPhase);
		Print("VACUUM %03.1f %03.1f %03.1f   %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
	}
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_STERILPRESSURE, 0, 0, "");
		ResetTimer(TIMER_PHASE);
		PhasePreheat(130, 220);
		success = PhasePressure(290.0, 200.0, 180.0, 600);
		GetTimer(TIMER_PHASE, &tPhase);
		Print("PRESS  %03.1f %03.1f %03.1f   %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
	}
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_STERILMAINTAIN, 0, 0, "");
		ResetTimer(TIMER_PHASE);
		Print("STERIL -------------------------\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
		PhasePreheat(130, 190);
		success = PhaseSteril(135.2, 134.0, 136.0, 210, 310.0, 350.0, 190.0, 170.0, 800);
	}
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_STERILRELEASE, 0, 0, "");
		ResetTimer(TIMER_PHASE);
		PhasePreheat(130, 200);
		success = PhaseRelease(110.0, 600);
	}

	// Dry
	if (success) {
		SendCommand(COMMAND_SET_PROGRESS_LEDS, LED_PROGRESS_DRY, 0, 0, "");
		success = PhaseDry(20.0, 50.0, 90.0, 300, 600);
		GetTimer(TIMER_PHASE, &tPhase);
		Print("DRY                        %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tPhase.TimeMinute, tPhase.TimeSecond);
	}

	GetTimer(TIMER_CYCLE, &tCycle);
	Print("--------------------------------\n");
	Print("TOTAL TIME                 %01d:%01d\n", Sensors.T1, Sensors.T2, Sensors.PressureChamber, tCycle.TimeMinute, tCycle.TimeSecond);

	// End
	osThreadSuspend(sensorsTaskHandle);

	if (success) {
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "PASS");
		Beep(3, 100, 100);
	} else {
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "FAIL");
		Beep(1, 1000, 100);
	}

	PhaseFinish();

	progRunning = 0;
	progStop = 0;

}
