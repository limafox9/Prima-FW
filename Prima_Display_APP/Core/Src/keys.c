/*
 * keys.c
 *
 *  Created on: Dec 6, 2022
 *      Author: josh
 */

#include "keys.h"

static uint8_t GetKeys();
static uint8_t GetPressedKeys();

// ==============================================================

void _task_Keys() {

	uint8_t i;
	static uint8_t lasti;

  for(;;) {
	 i=GetKeys();
	 while (lasti == i) {
		 i=GetKeys();
		 osDelay(50);
	 }

	 lasti = i;
	 //if (i > 0 && i < 16) Beep(i,100,100);
	 //if (i > 0 && i > 16) Beep(i-16,500,100);

//	 a.keysPressed = i;
	 DisplaySerialTransmit(&huart1, COMMAND_KEYS_PRESSED, i, 0, 0, 0);
	 osDelay(50);
  }
}

static uint8_t GetKeys() {

	static uint8_t lastKeys = 0;
	uint8_t currentKeys = 0;
	uint32_t startKeypress;
	static uint32_t lastKeypress = 0;

	//return 1;

	while (lastKeys > 0) {
		lastKeys = GetPressedKeys();
		osDelay(20);
	}

	currentKeys = GetPressedKeys();

	if (currentKeys > 0) startKeypress = HAL_GetTick();

	while (currentKeys > 0) {
		lastKeys = currentKeys;
		if (GetSafeDeltaTick(startKeypress, HAL_GetTick()) > KEY_LONG_PRESS_MS) {
			// bit 4 = long press
			lastKeys += 8;
			break;
		}
		if (GetSafeDeltaTick(lastKeypress, HAL_GetTick()) < KEY_DOBLECLICK_INTERVAL_MS) {
			// bit 5 = double click
			lastKeys += 16;
			break;
		}
		currentKeys = GetPressedKeys();
		osDelay(20);
	}
//
//	while (GetSafeDeltaTick(lastKeypress, HAL_GetTick()) < KEY_DOBLECLICK_INTERVAL_MS) {
//		currentKeys = GetPressedKeys();
//		if (currentKeys > 0) {
//			// bit 5 = double click
//			lastValue += 16;
//			break;
//		}
//		osDelay(20);
//	}
	if (lastKeys > 0) {
		lastKeypress = HAL_GetTick();
	}

	return lastKeys;
}

static uint8_t GetPressedKeys() {

	uint8_t currentKeys = 0;

	// Keys active low
	currentKeys = !(HAL_GPIO_ReadPin(KEY_PROG_GPIO_Port, KEY_PROG_Pin)); // bit 1
	currentKeys += !(HAL_GPIO_ReadPin(KEY_TEST_GPIO_Port, KEY_TEST_Pin)) << 1; // bit 2
	currentKeys += !(HAL_GPIO_ReadPin(KEY_START_GPIO_Port, KEY_START_Pin)) << 2; // bit 3

	return currentKeys;
}

