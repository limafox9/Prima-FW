/*==============================================================================

Filename: keys.c

Created on: Dec 16, 2022

Original work: Copyright (c) 2022 Luca Favenzi
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

==================================================================================
PURPOSE:
USAGE:
================================================================================*/

#include "keys.h"
#include "system.h"
#include "cmsis_os.h"
#include "serial.h"
#include "Inc/DB_Protocol.h"
#include "programs.h"
#include "modes.h"

#define KEY_LONG_PRESS_MS 3000

uint8_t _keysBeepActive = 0;

extern osThreadId programTaskHandle;

static uint8_t GetPressedKeys();
static void KeysReceived(uint8_t keysCode);

// ==============================================================

void ParseCommand(s_DisplayProtocolData* data) {

	if (data->Param1 == KEY_PROG) {
		if ((machineStatus.Status == STATUS_IDLE) && (machineStatus.Show == SHOW_PROGRAM)) {
			SelectProgram();
		} else if ((machineStatus.Status == STATUS_READY) && (machineStatus.Show == SHOW_PROGRAM)) {
			SelectProgram();
		} else if (machineStatus.Status == STATUS_RUNNING) {
			machineStatus.Show = SHOW_READINGS;
			SelectReading();
		}

		if (machineStatus.Show == SHOW_PARAMETERS) {
			SelectParameter();
		} else if (machineStatus.Show == SHOW_READINGS) {
			SelectReading();
		} else {
			__NOP();
		}

	} else if (data->Param1 == KEY_TEST) {
		if ((machineStatus.Status == STATUS_IDLE) && (machineStatus.Show == SHOW_PROGRAM)) {
			SelectTest();
		} else if ((machineStatus.Status == STATUS_READY) && (machineStatus.Show == SHOW_PROGRAM)) {
			SelectTest();
		} else if (machineStatus.Status == STATUS_RUNNING) {
			machineStatus.Show = SHOW_PROGRAM;
			ShowProgram();
		}

		if (machineStatus.Show == SHOW_PARAMETERS) {
			ChangeParameter(1);
		} else {
			__NOP();
		}

	} else if (data->Param1 == KEY_START) {
		if (machineStatus.Status == STATUS_READY) {
			programStart = 1;
			osThreadResume(programTaskHandle);
		}

		if (machineStatus.Show == SHOW_PARAMETERS) {
			ChangeParameter(-1);
		} else {
			__NOP();
		}

	} else if (data->Param1 == KEY_PROG + KEY_LONG_PRESS) {
		if (machineStatus.Status == STATUS_IDLE) {
			__NOP();
		} else if (machineStatus.Status == STATUS_READY) {
			__NOP();
		}

		if (machineStatus.Show == SHOW_PARAMETERS) {
			machineStatus.Show = SHOW_PROGRAM;
			SaveSettings();
			ShowProgram();
		} else if (machineStatus.Show == SHOW_READINGS) {
			machineStatus.Show = SHOW_PROGRAM;
			ShowProgram();
		} else {
			__NOP();
		}

	} else if (data->Param1 == KEY_TEST + KEY_LONG_PRESS) {
		// Button code here

	} else if (data->Param1 == KEY_START + KEY_LONG_PRESS) {
		if (machineStatus.Status == STATUS_IDLE) {
			__NOP();
		} else if (machineStatus.Status == STATUS_READY) {
			__NOP();
		} else if (machineStatus.Status = STATUS_RUNNING) {
			abortProgram = 1;
			machineStatus.Show = SHOW_PROGRAM;
			ShowProgram();
		} else {
			__NOP();
		}

	} else if (data->Param1 == KEY_TEST + KEY_PROG) {
		// Button code here
	} else if (data->Param1 == KEY_START + KEY_PROG) {
		// Button code here
	} else if (data->Param1 == KEY_START + KEY_TEST) {
		// Button code here
	} else if (data->Param1 == KEY_START + KEY_TEST + KEY_PROG) {
		// Button code here

	} else if (data->Param1 == KEY_TEST + KEY_PROG + KEY_LONG_PRESS) {
		// Button code here

	} else if (data->Param1 == KEY_START + KEY_TEST + KEY_LONG_PRESS) {
		if (machineStatus.Status == STATUS_IDLE) {
			machineStatus.Show = SHOW_PARAMETERS;
			ShowParameter();
		} else if (machineStatus.Status == STATUS_READY) {
			machineStatus.Show = SHOW_PARAMETERS;
			ShowParameter();
		} else {
			__NOP();
		}

	} else if (data->Param1 == KEY_START + KEY_PROG + KEY_LONG_PRESS) {
		if (machineStatus.Status == STATUS_IDLE) {
			machineStatus.Show = SHOW_READINGS;
			ShowReading();
		} else if (machineStatus.Status == STATUS_READY) {
			machineStatus.Show = SHOW_READINGS;
			ShowReading();
		} else {
			__NOP();
		}

	} else if (data->Param1 == KEY_START + KEY_TEST + KEY_PROG + KEY_LONG_PRESS) {
		// Button code here
	} else {
		__NOP();
	}
}

void StartKeysTask() {

	uint8_t pressKeys;
	uint32_t keyPressStart;

	for (;;) {
		pressKeys = GetPressedKeys();
		keyPressStart = HAL_GetTick();
		if (pressKeys != 0) {
			osDelay(50); // Debounce
			while (pressKeys == GetPressedKeys()) {
				if ((HAL_GetTick() - keyPressStart) > KEY_LONG_PRESS_MS) {
					pressKeys += KEY_LONG_PRESS;
					break;
				}
				osDelay(20);
			}
			KeysReceived(pressKeys);
		}
		osDelay(20);
	}
}

static uint8_t GetPressedKeys() {

	uint8_t pressKeys = 0;
	// Keys active low
	pressKeys = !(HAL_GPIO_ReadPin(KEY_S1_GPIO_Port, KEY_S1_Pin)); // bit 1
	pressKeys += !(HAL_GPIO_ReadPin(KEY_S2_GPIO_Port, KEY_S2_Pin)) << 1; // bit 2
	pressKeys += !(HAL_GPIO_ReadPin(KEY_S3_GPIO_Port, KEY_S3_Pin)) << 2; // bit 3

	return pressKeys;
}

static void KeysReceived(uint8_t keysCode) {

	if (keysCode == KEY_S1) {

	} else if (keysCode == KEY_S2) {

	} else if (keysCode == KEY_S3) {

	} else if (keysCode == KEY_S1 + KEY_LONG_PRESS) {
		// Button code here

	} else if (keysCode == KEY_S2 + KEY_LONG_PRESS) {
		// Button code here

	} else if (keysCode == KEY_S3 + KEY_LONG_PRESS) {
	} else if (keysCode == KEY_S2 + KEY_S1) {
		// Button code here
	} else if (keysCode == KEY_S3 + KEY_S1) {
		// Button code here
	} else if (keysCode == KEY_S3 + KEY_S2) {
		// Button code here
	} else if (keysCode == KEY_S3 + KEY_S2 + KEY_S1) {
		// Button code here

	} else if (keysCode == KEY_S2 + KEY_S1 + KEY_LONG_PRESS) {
		// Button code here

	} else if (keysCode == KEY_S3 + KEY_S2 + KEY_LONG_PRESS) {
	} else if (keysCode == KEY_S3 + KEY_S1 + KEY_LONG_PRESS) {

	} else if (keysCode == KEY_S3 + KEY_S2 + KEY_S1 + KEY_LONG_PRESS) {
		// Button code here
	} else {
		__NOP();
	}
}



