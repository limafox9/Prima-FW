/*==============================================================================

Filename: display.c

Created on: 16 feb 2023

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

#include "display.h"

#include "serial.h"
#include "sensors.h"
#include "phases.h"
#include "modes.h"
#include "Inc/DB_Protocol.h"

void DashLed(e_DisplayDashboardLeds led, e_PowerState status) {
	SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, led, status, 0, "");
}

void ProgressLed(ledProgress led) {
	SendCommand(DISPLAY_CMD_WRITE_PROGRESS_LEDS, led, 0, 0, "");
}

void DigitsWrite(uint8_t line, const char* digits) {
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, line, 0, 0, digits);
}

void DisplaySetBeep(e_PowerState set) {
	SendCommand(DISPLAY_CMD_SET_KEYS_BEEP, set, 0, 0,"");
}
void DisplaySetLongPressInterval(uint16_t interval) {
	SendCommand(DISPLAY_CMD_SET_LONG_PRESS_INTERVAL, interval, 0, 0,"");
}

void StartDisplayInfoTask() {

	RTCTimerTime_s tPhase, tDisplay;
	char s[8];


	for (;;) {
		if ((machineStatus.Status == STATUS_RUNNING) && (machineStatus.Show == SHOW_PROGRAM)) {
			SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, currPhase);
			ResetTimer(TIMER_DISPLAYINFO);
			while(1) {
				GetTimer(TIMER_DISPLAYINFO, &tDisplay);
				if (tDisplay.Seconds > 1) {
					break;
				}
				if ((machineStatus.Status) != STATUS_RUNNING || (machineStatus.Show != SHOW_PROGRAM)) break;
				osDelay(200);
			}
		}
		if ((machineStatus.Status == STATUS_RUNNING) && (machineStatus.Show == SHOW_PROGRAM)) {
			ResetTimer(TIMER_DISPLAYINFO);
			while(1) {
				GetTimer(TIMER_PHASE, &tPhase);
				sprintf(s, "%02d:%02d", tPhase.TimeMinute, tPhase.TimeSecond);
				SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, s);
				GetTimer(TIMER_DISPLAYINFO, &tDisplay);
				if (tDisplay.Seconds > 4) {
					break;
				}
				if ((machineStatus.Status) != STATUS_RUNNING || (machineStatus.Show != SHOW_PROGRAM)) break;
				osDelay(200);
			}
		}
	}

}


