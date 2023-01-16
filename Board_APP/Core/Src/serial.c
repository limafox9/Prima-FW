/*==============================================================================

Filename: serial.c

Created on: Jan 10, 2023

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

#include <string.h>
#include <stdio.h>

#include "serial.h"
#include "cmsis_os.h"
#include "buzzer.h"
#include "globals.h"
#include "Inc/DB_Protocol.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

uint8_t UART_rxBuffer[DB_UART_BUFFER_SIZE] = {[0 ... DB_UART_BUFFER_SIZE - 1] = 0};

static void ParseCommand(s_DisplayProtocolData* data);
extern void DBSend(uint16_t command, uint32_t param1, uint32_t param2, uint32_t param3, char* digits);

void SerialInit(UART_HandleTypeDef* huart, uint32_t baudRate) {
	huart->Init.BaudRate = baudRate;
	HAL_UART_Receive_DMA(huart, UART_rxBuffer, DB_UART_BUFFER_SIZE);
}

// TRANSMIT
// ==============================================================

void SerialSend(UART_HandleTypeDef* huart, uint8_t* data, uint32_t size) {
	HAL_UART_Transmit(huart, data, size, DB_UART_BUFFER_SIZE);
}
//
//void DisplaySendOK() {
//
//	DBSend(DISPLAY_CMD_OK, 0, 0, 0, 0);
//}
//
//void DisplaySendFWVersion() {
//
//	DBSend(DISPLAY_CMD_OK, FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION, 0);
//}


// RECEIVE
// ==============================================================

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {


	HAL_UART_Receive_DMA(huart, UART_rxBuffer, DB_UART_BUFFER_SIZE);

	if (huart->Instance == UART_DISPLAY.Instance) {
		s_DisplayProtocolData rxData;
		Display_ProtocolToData(UART_rxBuffer, &rxData);
		if (rxData.Valid) {
			ParseCommand(&rxData); // valid received command > parse
		}
	} else if (huart->Instance == UART_TDS.Instance) {
		Beep(2, 100, 100);
	}
}


// COMMANDS
// ==============================================================

static void ParseCommand(s_DisplayProtocolData* data) {

	switch (data->Command) {
		case DISPLAY_CMD_BEEP:
			Beep(data->Param1, data->Param2, data->Param3);
			break;
		case DISPLAY_CMD_FW_VER:
			DBSendCommand(DISPLAY_CMD_OK, FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION, "");
			break;
		case DISPLAY_CMD_READY:
			DBSendCommand(DISPLAY_CMD_WRITE_DIGITS, 1,0,0,"CIAO");
			break;
	}
}

//#include "serial.h"
//
//#include <stdio.h>
//#include "main.h"
//
//#define SERIAL_TX_BUFFER_SIZE 32
//#define SERIAL_RX_BUFFER_SIZE 32
//
//uint8_t serialTXBuffer[SERIAL_TX_BUFFER_SIZE];
//uint8_t serialRXBuffer[SERIAL_RX_BUFFER_SIZE];
//
//
//
//__weak void SerialReceived(UART_HandleTypeDef* huart, uint8_t* data, uint32_t size) {
////	if (huart->Instance == huart3.Instance) {
//////		PanelSerialReceive(serialRXBuffer, SERIAL_RX_BUFFER_SIZE);
////	} else if (huart->Instance == huart2.Instance) {
////
////	}
//}
//
//// =============================================================================
//
//void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef* huart) {
//
//}
//
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {
//
//}
//
//void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef* huart) {
//
//}
//
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
//	SerialReceived(huart, serialRXBuffer, SERIAL_RX_BUFFER_SIZE);
//	HAL_UART_Receive_DMA(huart, serialRXBuffer, SERIAL_RX_BUFFER_SIZE);
//	//	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
//}



