/*==============================================================================

Filename: keys.c

Created on: Dec 15, 2022

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

#include "main.h"
#include "serial.h"
#include "bootloader.h"
#include "Inc/DB_protocol.h"
#include <stdint.h>

#define KEY_LONG_PRESS_MS 3000

uint8_t SET_KeysBeep = 1;

static uint8_t GetPressedKeys();
static void KeysReceived(uint8_t keysCode);

// ==============================================================

void ParseCommand(s_DisplayProtocolData* data) {

	if (data->Command == DISPLAY_CMD_KEYS_PRESSED) {
		Beep(1, 100, 100);
		if (data->Param1 == KEY_PROG) {
			__NOP();
		} else if (data->Param1 == KEY_TEST) {
			__NOP();
		} else if (data->Param1 == KEY_START) {
			__NOP();
		} else if (data->Param1 == KEY_PROG + KEY_LONG_PRESS) {
			__NOP();
		} else if (data->Param1 == KEY_TEST + KEY_LONG_PRESS) {
			__NOP();
		} else if (data->Param1 == KEY_START + KEY_LONG_PRESS) {
			__NOP();
		} else if (data->Param1 == KEY_TEST + KEY_PROG) {
			__NOP();
		} else if (data->Param1 == KEY_START + KEY_PROG) {
			__NOP();
		} else if (data->Param1 == KEY_START + KEY_TEST) {
			__NOP();
		} else if (data->Param1 == KEY_START + KEY_TEST + KEY_PROG) {
			__NOP();
		} else if (data->Param1 == KEY_TEST + KEY_PROG + KEY_LONG_PRESS) {
			__NOP();
		} else if (data->Param1 == KEY_START + KEY_TEST + KEY_LONG_PRESS) {
			__NOP();
		} else if (data->Param1 == KEY_START + KEY_PROG + KEY_LONG_PRESS) {
			__NOP();
		} else if (data->Param1 == KEY_START + KEY_TEST + KEY_PROG + KEY_LONG_PRESS) {
			__NOP();
		} else {
			__NOP();
		}
	} else if (data->Command == DISPLAY_CMD_OK) {
		DisplayFWVersionMajor = (uint8_t)data->Param1;
		DisplayFWVersionMinor = (uint8_t)data->Param2;
		DisplayFWVersionRevision = (uint8_t)data->Param3;
	} else if (data->Command == DISPLAY_CMD_KEYS_OK) {
		keysReceivedOK = (uint8_t)data->Param1;
		keysReceived = (uint8_t)data->Param2;
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
		__NOP();
	} else if (keysCode == KEY_S2) {
		__NOP();
	} else if (keysCode == KEY_S3) {
		__NOP();
	} else if (keysCode == KEY_S1 + KEY_LONG_PRESS) {
		__NOP();
	} else if (keysCode == KEY_S2 + KEY_LONG_PRESS) {
		__NOP();
	} else if (keysCode == KEY_S3 + KEY_LONG_PRESS) {
		__NOP();
	} else if (keysCode == KEY_S2 + KEY_S1) {
		__NOP();
	} else if (keysCode == KEY_S3 + KEY_S1) {
		__NOP();
	} else if (keysCode == KEY_S3 + KEY_S2) {
		__NOP();
	} else if (keysCode == KEY_S3 + KEY_S2 + KEY_S1) {
		__NOP();
	} else if (keysCode == KEY_S2 + KEY_S1 + KEY_LONG_PRESS) {
		__NOP();
	} else if (keysCode == KEY_S3 + KEY_S2 + KEY_LONG_PRESS) {
		__NOP();
	} else if (keysCode == KEY_S3 + KEY_S1 + KEY_LONG_PRESS) {
		__NOP();
	} else if (keysCode == KEY_S3 + KEY_S2 + KEY_S1 + KEY_LONG_PRESS) {
		__NOP();
	} else {
		__NOP();
	}
}



