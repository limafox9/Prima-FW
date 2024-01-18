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
#include "cmsis_os.h"
#include "globals.h"
#include "serial.h"
#include "buzzer.h"
#include "Inc/DB_protocol.h"

extern uint16_t SET_KeysLongPressDuration;
extern bool SET_KeysBeep;


static void KeysReceived(uint8_t keysCode);

// ==============================================================

void KeysTask() {

	uint8_t pressKeys;
	uint32_t keyPressStart;

	for (;;) {
		pressKeys = GetPressedKeys();
		keyPressStart = HAL_GetTick();
		if (pressKeys != 0) {
			osDelay(5); // Debounce
			pressKeys = GetPressedKeys();
			while (GetPressedKeys() != 0) {
				if ((HAL_GetTick() - keyPressStart) > SET_KeysLongPressInterval) {
					pressKeys = GetPressedKeys();
					pressKeys += KEY_LONG_PRESS;
					break;
				}
				osDelay(5);
			}
			if (SET_KeysBeep) Beep(1, 100, 100);
			SendCommand(DISPLAY_CMD_KEYS_PRESSED, pressKeys, 0, 0, "");
			KeysReceived(pressKeys);
			while (GetPressedKeys()) osDelay(5);
		}
		osDelay(5);
	}
}

uint8_t GetPressedKeys() {

	uint8_t pressKeys = 0;
	// Keys active low
	pressKeys = !(HAL_GPIO_ReadPin(KEY_PROG_GPIO_Port, KEY_PROG_Pin)); // bit 1
	pressKeys += !(HAL_GPIO_ReadPin(KEY_TEST_GPIO_Port, KEY_TEST_Pin)) << 1; // bit 2
	pressKeys += !(HAL_GPIO_ReadPin(KEY_START_GPIO_Port, KEY_START_Pin)) << 2; // bit 3

	return pressKeys;
}

static void KeysReceived(uint8_t keysCode) {

	if (keysCode == KEY_PROG) {
		// Button code here
	} else if (keysCode == KEY_TEST) {
		// Button code here
	} else if (keysCode == KEY_START) {
		// Button code here
	} else if (keysCode == KEY_PROG + KEY_LONG_PRESS) {
		// Button code here
	} else if (keysCode == KEY_TEST + KEY_LONG_PRESS) {
		// Button code here
	} else if (keysCode == KEY_START + KEY_LONG_PRESS) {
		// Button code here
	} else if (keysCode == KEY_PROG + KEY_TEST) {
		// Button code here
	} else if (keysCode == KEY_PROG + KEY_START) {
		// Button code here
	} else if (keysCode == KEY_TEST + KEY_START) {
		// Button code here
	} else if (keysCode == KEY_PROG + KEY_TEST + KEY_START) {
		// Button code here
	} else if (keysCode == KEY_PROG + KEY_TEST + KEY_LONG_PRESS) {
		// Button code here
	} else if (keysCode == KEY_PROG + KEY_START + KEY_LONG_PRESS) {
		// Button code here
	} else if (keysCode == KEY_TEST + KEY_START + KEY_LONG_PRESS) {
		// Button code here
	} else if (keysCode == KEY_PROG + KEY_TEST + KEY_START + KEY_LONG_PRESS) {
		// Button code here
	} else {
		// Default code here
	}
}
