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
PURPOSE: Modbus CRC16 calculation
USAGE:
================================================================================*/

#include "keys.h"

#define KEY_LONG_PRESS_MS 3000
#define KEY_DOBLECLICK_INTERVAL_MS 100

uint8_t _keysBeepActive = TRUE;

static uint8_t GetKeys();
static uint8_t GetPressedKeys();

// ==============================================================

void StartKeysTask() {

	uint8_t currKeys;
	static uint8_t lastKeys;

	for (;;) {
		currKeys = GetKeys();
		while ((lastKeys == currKeys)) {
			currKeys = GetKeys();
			osDelay(10);
		}

		if (currKeys > 0) {
			KeysReceived(currKeys);
			lastKeys = currKeys;
		}

		osDelay(10);
	}
}

static uint8_t GetKeys() {

	static uint8_t lastKeys = 0;
	uint8_t currentKeys = 0;
	uint32_t startKeypress;
	static uint32_t lastKeypress = 0;

	while (lastKeys > 0) {
		lastKeys = GetPressedKeys();
		osDelay(10);
	}

	currentKeys = GetPressedKeys();

	if (currentKeys > 0) startKeypress = HAL_GetTick();

	while (currentKeys > 0) {
		lastKeys = currentKeys;
		if (GetSafeDeltaTick(startKeypress, HAL_GetTick()) > KEY_LONG_PRESS_MS) {
			// bit 4 = long press
			lastKeys += KEY_LONG_PRESS;
		}
		if (GetSafeDeltaTick(lastKeypress, HAL_GetTick()) < KEY_DOBLECLICK_INTERVAL_MS) {
			// bit 5 = double click
			lastKeys += KEY_LONG_PRESS;
		}
		currentKeys = GetPressedKeys();
		osDelay(10);
	}

	if (lastKeys > 0) lastKeypress = HAL_GetTick();

	return lastKeys;
}

static uint8_t GetPressedKeys() {

	uint8_t currentKeys = 0;

	// Keys active low
	currentKeys = !(HAL_GPIO_ReadPin(KEY_S1_GPIO_Port, KEY_S1_Pin)); // bit 1
	currentKeys += !(HAL_GPIO_ReadPin(KEY_S2_GPIO_Port, KEY_S2_Pin)) << 1; // bit 2
	currentKeys += !(HAL_GPIO_ReadPin(KEY_S3_GPIO_Port, KEY_S3_Pin)) << 2; // bit 3

	return currentKeys;
}

void KeysReceived(uint8_t keysCode) {

	if (keysCode == KEY_S1) {
		// Button code here
	} else if (keysCode == KEY_S2) {
		// Button code here
		Beep(1,100,100);
	} else if (keysCode == KEY_S3) {
		// Button code here
	} else if (keysCode == KEY_S1 + KEY_LONG_PRESS) {
		// Button code here
	} else if (keysCode == KEY_S2 + KEY_LONG_PRESS) {
		// Button code here
	} else if (keysCode == KEY_S3 + KEY_LONG_PRESS) {
		// Button code here
	} else if (keysCode == KEY_S1 + KEY_DOUBLE_PRESS) {
		// Button code here
	} else if (keysCode == KEY_S2 + KEY_DOUBLE_PRESS) {
		// Button code here
	} else if (keysCode == KEY_S3 + KEY_DOUBLE_PRESS) {
		// Button code here
	} else if (keysCode == KEY_S1 + KEY_S2) {
		// Button code here
	} else if (keysCode == KEY_S1 + KEY_S3) {
		// Button code here
	} else if (keysCode == KEY_S2 + KEY_S3) {
		// Button code here
	} else if (keysCode == KEY_S1 + KEY_S2 + KEY_S3) {
		// Button code here
	} else if (keysCode == KEY_S1 + KEY_S2 + KEY_LONG_PRESS) {
		// Button code here
	} else if (keysCode == KEY_S1 + KEY_S3 + KEY_LONG_PRESS) {
		// Button code here
	} else if (keysCode == KEY_S2 + KEY_S3 + KEY_LONG_PRESS) {
		// Button code here
	} else if (keysCode == KEY_S1 + KEY_S2 + KEY_S3 + KEY_LONG_PRESS) {
		// Button code here
	} else {
		// Default code here
	}
}



