/*==============================================================================

Filename: panel_serial.c

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

// 3C7EDB0002000000640000006400000000000000CCCE7E3E10 -> 2 beeps

#include <dashboard_leds.h>
#include <string.h>
#include <stdio.h>

#include "serial.h"
#include "digits.h"
#include "cmsis_os.h"
#include "buzzer.h"
#include "globals.h"
#include "Inc/DB_Protocol.h"

extern uint16_t SET_KeysLongPressInterval;
extern uint16_t SET_FlashInterval;
extern uint8_t SET_KeysBeep;

extern UART_HandleTypeDef huart1;

uint8_t UART_rxBuffer[DB_UART_BUFFER_SIZE] = {[0 ... DB_UART_BUFFER_SIZE - 1] = 0};

static void ParseCommand(s_DisplayProtocolData* data);
extern void DBSendCommand(uint16_t command, uint32_t param1, uint32_t param2, uint32_t param3, char* digits);

// TRANSMIT
// ==============================================================

void DisplaySendOK() {

	DBSendCommand(DISPLAY_CMD_OK, 0, 0, 0, 0);
}

void DisplaySendPanelReady(){

	DBSendCommand(DISPLAY_CMD_READY, FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION, 0);
}

void DisplaySendFWVersion() {

	DBSendCommand(DISPLAY_CMD_OK, FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION, 0);
}


// RECEIVE
// ==============================================================

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {

	s_DisplayProtocolData rxData;

	HAL_UART_Receive_DMA(huart, UART_rxBuffer, DB_UART_BUFFER_SIZE);
	if (huart->Instance == huart1.Instance) {
		Display_ProtocolToData(UART_rxBuffer, &rxData);
		if (rxData.Valid) {
			ParseCommand(&rxData); // valid received command > parse
		}
	}
}


// COMMANDS
// ==============================================================

static void ParseCommand(s_DisplayProtocolData* data) {

	uint16_t duration, beeps, beepsInterval;

	switch (data->Command) {
		case DISPLAY_CMD_BEEP:
			DisplayBeep(data->Param1, data->Param2, data->Param3);
			break;
		case DISPLAY_CMD_WRITE_DIGITS:
			DisplayDigitsWriteRow(data->Param1, data->Digits);
			break;
		case DISPLAY_CMD_WRITE_DASHBOARD_LEDS:
//			DisplayWriteLeds((uint16_t)data->Param1);
			break;
		case DISPLAY_CMD_WRITE_PROGRESS_LEDS:
//			DisplayWriteProgressLeds((uint16_t)data->Param1);
			break;
		case DISPLAY_CMD_SET_LONG_PRESS_INTERVAL:
//			SET_KeysLongPressInterval = (uint16_t)data->Param1;
			break;
		case DISPLAY_CMD_SET_FLASH_INTERVAL:
//			SET_FlashInterval = (uint16_t)data->Param1;
			break;
		case DISPLAY_CMD_SET_KEYS_BEEP:
//			SET_KeysBeep = (bool)data->Param1;
			break;
		case DISPLAY_CMD_FW_VER:
			DBSendCommand(DISPLAY_CMD_OK, FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION, "");
			break;
		case DISPLAY_CMD_CHECK:
			duration = ((uint16_t)data->Param1 >= 500) && ((uint16_t)data->Param1 <= 10000) ? (uint16_t)data->Param1 : 3000;
			beeps = (uint8_t)data->Param2 <= 10 ? (uint8_t)data->Param2 : 10;
			beepsInterval = ((uint16_t)data->Param3 >= 50) && ((uint16_t)data->Param3 <= 1000) ? (uint16_t)data->Param3 : 100;
			DisplayDashboardSetAll();
			DisplayProgressSetAll();
			DisplayDigitsSetAll();
			osDelay(duration);
			DisplayDashboardClearAll();
			DisplayProgressClearAll();
			DisplayDigitsClearAll();
			DisplayBeep(beeps, beepsInterval, beepsInterval);
			break;
	}
}

