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

#include "dashboard.h"
#include "globals.h"
#include "cmsis_os.h"
#include "keys.h"
#include "digits.h"
#include "serial.h"
#include "buzzer.h"
#include "progress_bar.h"
#include "Inc/DB_Protocol.h"
#include <stdlib.h>

// GLOBAL VARS =================================================================

osThreadId 	taskKeysHandle;
osThreadId 	taskDigitsHandle;
osSemaphoreId semaphoreSerialsHandle;

uint32_t 	UID[3];

//uint16_t 	progressBarBits;
//uint16_t 	progressBarBitsBlink;

//uint16_t 	dashboardLedsBits;
//uint16_t 	panelLedsBitsBlink;

//uint32_t 	digitsBits;

//uint8_t 	keysPressed;

extern UART_HandleTypeDef huart1;
//extern uint8_t UART_rxBuffer[DB_UART_BUFFER_SIZE] = {[0 ... DB_UART_BUFFER_SIZE - 1] = 0};

//extern void DBInit(UART_HandleTypeDef* huart, GPIO_TypeDef* rs485_CS_GPIOPort, uint16_t rs485_CS_GPIOPin);

// SETTINGS ====================================================================
uint16_t 	SET_KeysLongPressInterval = DEFAULT_SETTING_KEYS_LONG_PRESS_INTERVAL;
uint8_t 		SET_KeysBeep = DEFAULT_SETTING_KEYS_BEEP;
uint16_t 	SET_FlashInterval = DEFAULT_SETTING_LEDS_FLASH_INTERVAL;

// Entry point
void SysInit() {


	osSemaphoreDef(semaphoreSerial);
	semaphoreSerialsHandle = osSemaphoreCreate(osSemaphore(semaphoreSerial), 1);
	  osSemaphoreWait(semaphoreSerialsHandle, 100);

	  osDelay(100);
	DigitsInit();
	ProgressBarInit();
	  osDelay(100);
	  osSemaphoreRelease(semaphoreSerialsHandle);

	osThreadDef(keysTask, KeysTask, osPriorityNormal, 1, 1024);
	taskKeysHandle = osThreadCreate(osThread(keysTask), NULL);

	osThreadDef(digitsTask, DigitsRefreshTask, osPriorityNormal, 1, 512);
	taskDigitsHandle = osThreadCreate(osThread(digitsTask), NULL);

//	ProgressBarSetAll();

//	DBInit(&UART_DISPLAY, USART1_CTS_GPIO_Port, USART1_CTS_Pin);		// Display serial initialize
	//	HAL_UART_Receive_DMA(&UART_DISPLAY, UART_rxBuffer, DB_UART_BUFFER_SIZE);

//	DashboardSetAll();
//	ProgressBarSetAll();
//	DigitsSetAll();
//	osDelay(2000);
//
//	DashboardClearAll();
//	ProgressBarClearAll();
//	DigitsClearAll();
//
//	char buf[8] = {[1 ... 7] = 0};
//	sprintf(buf, "%01d.%01d.%02d", FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION);
//	DigitsWrite(1, " UEr");
//	DigitsWrite(3, buf);
//	osDelay(2000);
//
//	DigitsClearAll();
//
//	Beep(3,100,100);


	DisplayUARTInit();
//	SendCommand(DISPLAY_CMD_OK, FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION, "");
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


