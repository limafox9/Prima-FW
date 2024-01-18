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


#include <dashboard.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "serial.h"
#include "digits.h"
#include "cmsis_os.h"
#include "buzzer.h"
#include "globals.h"
#include "Inc/DB_protocol.h"
#include "_ver.h"
#include "keys.h"


extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;

s_DisplayProtocolData rxData;

volatile uint8_t UART_rxBuffer[UART_DISPLAY_RX_BUFFER_SIZE];
//uint8_t rxBuffer[DISPLAY_UART_BUFFER_SIZE] = {[0 ... DISPLAY_UART_BUFFER_SIZE - 1] = 0};

static void ParseCommand(s_DisplayProtocolData* data);
//extern void DBSendCommand(uint16_t command, uint32_t param1, uint32_t param2, uint32_t param3, char* digits);

//
void DisplayUARTInit() {

	//	DISPLAY_UART_HANDLE.Init.BaudRate = DISPLAY_UART_SPEED;
//		HAL_UART_Receive_DMA(&DISPLAY_UART_HANDLE, UART_rxBuffer, UART_DISPLAY_RX_BUFFER_SIZE);
	HAL_UARTEx_ReceiveToIdle_DMA(&DISPLAY_UART_HANDLE, UART_rxBuffer, UART_DISPLAY_RX_BUFFER_SIZE);
	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
}

// TRANSMIT
// ==============================================================
void SendCommand(uint16_t command, uint32_t param1, uint32_t param2, uint32_t param3, char* digits) {

	s_DisplayProtocolData txData = {command, param1, param2, param3, digits, 1};
	osSemaphoreWait(semaphoreSerialsHandle, 100);
	uint8_t txBuffer[UART_DISPLAY_TX_BUFFER_SIZE];
	memset(txBuffer, 255, sizeof(txBuffer));
	Display_DataToProtocol(&txData, txBuffer);

	// Chip 485 ENABLE
	HAL_GPIO_WritePin(USART1_CTS_GPIO_Port, USART1_CTS_Pin, GPIO_PIN_SET);
	HAL_Delay(2);
	// Send data
	HAL_UART_Transmit(&DISPLAY_UART_HANDLE, txBuffer, UART_DISPLAY_TX_BUFFER_SIZE, DISPLAY_UART_TX_TIMEOUT);
	// Chip 485 DISABLE
	HAL_Delay(2);
	HAL_GPIO_WritePin(USART1_CTS_GPIO_Port, USART1_CTS_Pin, GPIO_PIN_RESET);
	osSemaphoreRelease(semaphoreSerialsHandle);
}

//void DisplaySendOK() {
//
//	DBSendCommand(DISPLAY_CMD_OK, 0, 0, 0, 0);
//}
//
//void DisplaySendPanelReady(){
//
//	DBSendCommand(DISPLAY_CMD_READY, FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION, 0);
//}
//
//void DisplaySendFWVersion() {
//
//	DBSendCommand(DISPLAY_CMD_OK, FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION, 0);
//}


// RECEIVE
// ==============================================================


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {

	uint8_t headFound, tailFound;
	uint8_t tempBuf[UART_DISPLAY_RX_BUFFER_SIZE];
	osSemaphoreWait(semaphoreSerialsHandle, 100);
	for (int i = UART_DISPLAY_RX_BUFFER_SIZE; i >= 0; i--) {
		if ((UART_rxBuffer[i] == '<') && (UART_rxBuffer[i+1] == '~')) {


			uint8_t p2[i];
			memcpy(p2, UART_rxBuffer, i);

			uint8_t p1[UART_DISPLAY_RX_BUFFER_SIZE - i];
			memcpy(p1, UART_rxBuffer + i, UART_DISPLAY_RX_BUFFER_SIZE - i);

			memcpy(tempBuf, p1, UART_DISPLAY_RX_BUFFER_SIZE - i);
			memcpy(tempBuf + UART_DISPLAY_RX_BUFFER_SIZE - i, p2, i);


			Display_ProtocolToData(tempBuf, &rxData);
			if (rxData.Valid) {
				osSemaphoreRelease(semaphoreSerialsHandle);
				ParseCommand(&rxData);
			}
		}
	}


	HAL_UARTEx_ReceiveToIdle_DMA(&DISPLAY_UART_HANDLE, UART_rxBuffer, UART_DISPLAY_RX_BUFFER_SIZE);
	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
	osSemaphoreRelease(semaphoreSerialsHandle);
}


//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
//
//
//
//	s_DisplayProtocolData rxData;
//
//	if (huart->Instance == DISPLAY_UART_HANDLE.Instance) {
////		Display_ProtocolToData(UART_rxBuffer, &rxData);
////		if (rxData.Valid) {
////			ParseCommand(&rxData); // valid received command > parse
////		}
//
//		uint8_t headFound, tailFound;
//		uint8_t tempBuf[UART_DISPLAY_RX_BUFFER_SIZE];
//
//		for (int i = UART_DISPLAY_RX_BUFFER_SIZE; i >= 0; i--) {
//			if ((UART_rxBuffer[i] == '<') && (UART_rxBuffer[i+1] == '~')) {
//
//				HAL_Delay(5);
//
//				uint8_t p2[i];
//				memcpy(p2, UART_rxBuffer, i);
//
//				uint8_t p1[UART_DISPLAY_RX_BUFFER_SIZE - i];
//				memcpy(p1, UART_rxBuffer + i, UART_DISPLAY_RX_BUFFER_SIZE - i);
//
//				memcpy(tempBuf, p1, UART_DISPLAY_RX_BUFFER_SIZE - i);
//				memcpy(tempBuf + UART_DISPLAY_RX_BUFFER_SIZE - i, p2, i);
//
//				Display_ProtocolToData(tempBuf, &rxData);
//				if (rxData.Valid) {
//					ParseCommand(&rxData);
//				}
//			}
//		}
//
//
//	}
//
//
//
////	if (huart->Instance == DISPLAY_UART_HANDLE.Instance) {
////
////		uint8_t headFound, tailFound;
////		uint8_t tempBuf[UART_DISPLAY_BUFFER_SIZE];
////
////		for (int i = 0; i < UART_DISPLAY_BUFFER_SIZE; i++) {
////			if ((UART_rxBuffer[i] == '<') && (UART_rxBuffer[i+1] == '~')) {
////
////				uint8_t p1[UART_DISPLAY_BUFFER_SIZE - i];
////				memcpy(p1, UART_rxBuffer + i, UART_DISPLAY_BUFFER_SIZE - i);
////
////				uint8_t p2[i];
////				HAL_UART_Receive(huart, p2, i, 10);
////
////				memcpy(tempBuf, p1, UART_DISPLAY_BUFFER_SIZE - i);
////				memcpy(tempBuf + UART_DISPLAY_BUFFER_SIZE - i, p2, i);
////
////				Display_ProtocolToData(tempBuf, &rxData);
////				if (rxData.Valid) {
////					ParseCommand(&rxData); // valid received command > parse
////				}
////			}
////		}
////	}
//
//	//	DISPLAY_UART_HANDLE.Init.BaudRate = DISPLAY_UART_SPEED;
//		HAL_UART_Receive_DMA(&DISPLAY_UART_HANDLE, UART_rxBuffer, UART_DISPLAY_RX_BUFFER_SIZE);
//	//	HAL_UARTEx_ReceiveToIdle_DMA(&DISPLAY_UART_HANDLE, UART_rxBuffer, DISPLAY_UART_BUFFER_SIZE);
//
//
//
//}


// COMMANDS
// ==============================================================

static void ParseCommand(s_DisplayProtocolData* data) {

	uint16_t duration, beeps, beepsInterval;

	switch (data->Command) {
		case DISPLAY_CMD_BEEP:
			Beep(data->Param1, data->Param2, data->Param3);
			break;
		case DISPLAY_CMD_WRITE_DIGITS:
			DigitsWrite(data->Param1, data->Digits);
			break;
		case DISPLAY_CMD_WRITE_DASHBOARD_LEDS:
			DashboardWrite((uint16_t)data->Param1, (uint8_t)data->Param2);
			break;
		case DISPLAY_CMD_WRITE_PROGRESS_LEDS:
//			DisplayWriteProgressLeds((uint16_t)data->Param1);
			ProgressBarWrite((uint16_t)data->Param1);
			ProgressBarRefresh();
			break;
		case DISPLAY_CMD_SET_LONG_PRESS_INTERVAL:
			SET_KeysLongPressInterval = (uint16_t)data->Param1;
			break;
		case DISPLAY_CMD_SET_FLASH_INTERVAL:
			SET_FlashInterval = (uint16_t)data->Param1;
			break;
		case DISPLAY_CMD_SET_KEYS_BEEP:
			SET_KeysBeep = (bool)data->Param1;
			break;
		case DISPLAY_CMD_FW_VER:
			SendCommand(DISPLAY_CMD_OK, FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION, "");
			break;
		case DISPLAY_CMD_GET_KEYS:
			SendCommand(DISPLAY_CMD_KEYS_OK, 1, GetPressedKeys(), 0, "");
			break;
		case DISPLAY_CMD_CHECK:
//			duration = ((uint16_t)data->Param1 >= 500) && ((uint16_t)data->Param1 <= 10000) ? (uint16_t)data->Param1 : 3000;
//			beeps = (uint8_t)data->Param2 <= 10 ? (uint8_t)data->Param2 : 10;
//			beepsInterval = ((uint16_t)data->Param3 >= 50) && ((uint16_t)data->Param3 <= 1000) ? (uint16_t)data->Param3 : 100;
//			DisplayDashboardSetAll();
//			DisplayProgressSetAll();
//			DisplayDigitsSetAll();
//			osDelay(duration);
//			DisplayDashboardClearAll();
//			DisplayProgressClearAll();
//			DisplayDigitsClearAll();
//			DisplayBeep(beeps, beepsInterval, beepsInterval);
			break;
	}
}
