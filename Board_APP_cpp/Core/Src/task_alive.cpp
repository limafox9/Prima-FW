/*==============================================================================

Filename: task_main.cpp

Created on: Jan 5, 2023

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


#include "class_RTClock.h"
#include "task_alive.h"

#include "main.h"
#include "globals.h"
#include "cmsis_os.h"
#include "class_machine.h"

extern I2C_HandleTypeDef hi2c1;
extern machine_c M;

RTCDateTime_s r;

void taskAliveCpp() {

	M.Init();
	M.Beep(3, 100, 100);
	Log(LOG_LEVEL_RELEASE, LOG_NONE, "\n\n****************************************");
	Log(LOG_LEVEL_RELEASE, LOG_NONE, "PRIMA Board APP v%d.%d.%d", M.GetFirmwareVersion()[0], M.GetFirmwareVersion()[1], M.GetFirmwareVersion()[2]);
	Log(LOG_LEVEL_RELEASE, LOG_NONE, "UID: %08X %08X %08X", M.GetUID()[0], M.GetUID()[1], M.GetUID()[2]);
	Log(LOG_LEVEL_RELEASE, LOG_NONE, "Serial: %08X %08X %08X", M.GetUID()[0], M.GetUID()[1], M.GetUID()[2]);
	M.RTClock.GetDateTime(&r);
	Log(LOG_LEVEL_RELEASE, LOG_NONE, "Date: %02d-%02d-%02d - Time: %02d:%02d\n", r.Year, r.Month, r.Day, r.Hour, r.Minute);

	Log(LOG_LEVEL_DEBUG, LOG_TIME, "Kernel Started");
	Log(LOG_LEVEL_DEBUG, LOG_TIME, "Temp: %0.1f %0.1f", M.TempSensor.GetTemp(ADC_ZFQ), M.TempSensor.GetTemp(ADC_GB));

	for (;;) {
		HAL_GPIO_TogglePin(LED_HEART_GPIO_Port, LED_HEART_Pin);
	    osDelay(HEART_LED_BLINK_DURATION);
	}
}

// Define all C function calls from main.c below
extern "C"
{
    void taskAliveC() {
    	taskAliveCpp();
    }
}


