/*

Filename: globals.c

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

================================================================================
PURPOSE:
USAGE:
==============================================================================*/

#include <dashboard_leds.h>
#include "globals.h"
#include "cmsis_os.h"
#include "keys.h"
#include "digits.h"
#include "Inc/DB_globals.h"

// GLOBAL VARS =================================================================

osThreadId 	taskKeysHandle;
osThreadId 	taskDigitsHandle;

uint32_t 	UID[3];

uint8_t		digitsRow[DIGIT_ROWS][DIGIT_ROW_BUFFER_SIZE];

uint16_t 	progressBarBits;
uint16_t 	progressBarBitsBlink;

uint16_t 	panelLedsBits;
uint16_t 	panelLedsBitsBlink;

uint32_t 	digitsBits;

uint8_t 	keysPressed;

extern UART_HandleTypeDef huart1;

extern void DBInit(UART_HandleTypeDef* huart, GPIO_TypeDef* rs485_CS_GPIOPort, uint16_t rs485_CS_GPIOPin);

// SETTINGS ====================================================================
uint16_t 	SET_KeysLongPressDuration = DEFAULT_SETTING_KEYS_LONG_PRESS_INTERVAL;
bool 		SET_KeysBeep = DEFAULT_SETTING_KEYS_BEEP;
uint16_t 	SET_LedsFlashInterval = DEFAULT_SETTING_LEDS_FLASH_INTERVAL;

// Entry point
void SysInit() {

	osThreadDef(keysTask, StartKeysTask, osPriorityNormal, 1, 256);
	taskKeysHandle = osThreadCreate(osThread(keysTask), NULL);

	osThreadDef(digitsTask, DisplayDigitsRefresh, osPriorityNormal, 1, 256);
	taskDigitsHandle = osThreadCreate(osThread(digitsTask), NULL);

	DisplayDigitsInit();
	DBInit(&UART_DISPLAY, USART1_CTS_GPIO_Port, USART1_CTS_Pin);		// Display serial initialize

	DisplayDashboardSetAll();
	DisplayProgressSetAll();
	DisplayDigitsSetAll();
	osDelay(2000);

	DisplayDashboardClearAll();
	DisplayProgressClearAll();
	DisplayDigitsClearAll();

	char buf[8] = {[1 ... 7] = 0};
	sprintf(buf, "%01d.%01d.%02d", FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION);
	DisplayDigitsWriteRow(2, buf);
	osDelay(2000);

	DisplayDigitsClearAll();

	DisplayBeep(3,100,100);

	osDelay(500);
	DisplaySendPanelReady();
}

uint32_t GetSafeDeltaTick(uint32_t startTick, uint32_t endTick) {

	uint32_t deltaTick;
	// In case of HAL_GetTick() overflow (~49 days). Counter restarts from 0.
	if (endTick < startTick) {
		deltaTick = MAX_VALUE(startTick) - startTick + endTick;
	} else {
		deltaTick = endTick - startTick;
	}
	return deltaTick;
}

void SwapBytes(void *pBytes, size_t size) {
    char *p = pBytes;
    size_t lo, hi;
    for(lo = 0, hi = size - 1; hi > lo; lo++, hi--)
    {
        char tmp = p[lo];
        p[lo] = p[hi];
        p[hi] = tmp;
    }
}


