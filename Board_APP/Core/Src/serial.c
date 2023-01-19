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
#include "actuators.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

uint8_t UART_rxBuffer[DISPLAY_UART_BUFFER_SIZE] = {[0 ... DISPLAY_UART_BUFFER_SIZE - 1] = 0};

static void ParseCommand(s_DisplayProtocolData* data);
extern void DBInit(UART_HandleTypeDef* huart, GPIO_TypeDef* rs485_CS_GPIOPort, uint16_t rs485_CS_GPIOPin);
extern void DBSend(uint16_t command, uint32_t param1, uint32_t param2, uint32_t param3, char* digits);

void SerialInit(UART_HandleTypeDef* huart, uint32_t baudRate) {
	huart->Init.BaudRate = baudRate;
	HAL_UART_Receive_DMA(huart, UART_rxBuffer, DISPLAY_UART_BUFFER_SIZE);
}

//void SerialInitDB() {
//	DBInit(&UART_DISPLAY, USART3_CTS_GPIO_Port, USART3_CTS_Pin);
//	HAL_UART_Receive_DMA(&UART_DISPLAY, UART_rxBuffer, DB_UART_BUFFER_SIZE);
//}

// TRANSMIT
// ==============================================================

void SerialSend(UART_HandleTypeDef* huart, uint8_t* data, uint32_t size) {
	HAL_UART_Transmit(huart, data, size, DISPLAY_UART_BUFFER_SIZE);
}

void SendCommand(uint16_t command, uint32_t param1, uint32_t param2, uint32_t param3, char* digits) {


	s_DisplayProtocolData txData = {command, param1, param2, param3, digits, 1};

//	char buf[8] = {[0 ... 7] = 0};
	memcpy(&txData.Digits, digits, strlen(digits) + 1);

	uint8_t txBuffer[DISPLAY_UART_BUFFER_SIZE];
	Display_DataToProtocol(&txData, txBuffer);

	// Chip 485 ENABLE
	HAL_GPIO_WritePin(USART3_CTS_GPIO_Port, USART3_CTS_Pin, GPIO_PIN_SET);
	// Send data
	HAL_UART_Transmit(&UART_DISPLAY, txBuffer, DISPLAY_UART_BUFFER_SIZE, DISPLAY_UART_TX_TIMEOUT);
	// Chip 485 DISABLE
	HAL_GPIO_WritePin(USART3_CTS_GPIO_Port, USART3_CTS_Pin, GPIO_PIN_RESET);
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

	if (huart->Instance == UART_DISPLAY.Instance) {
		s_DisplayProtocolData rxData;
		Display_ProtocolToData(UART_rxBuffer, &rxData);
		if (rxData.Valid) {
			ParseCommand(&rxData); // valid received command > parse
		}
	}

	HAL_UART_Receive_DMA(huart, UART_rxBuffer, DISPLAY_UART_BUFFER_SIZE);
}


// COMMANDS
// ==============================================================

static void ParseCommand(s_DisplayProtocolData* data) {

	switch (data->Command) {
		case DISPLAY_CMD_BEEP:
			Beep(data->Param1, data->Param2, data->Param3);
			break;
		case DISPLAY_CMD_FW_VER:
			SendCommand(DISPLAY_CMD_OK, FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION, "");
			break;
		case DISPLAY_CMD_READY:
			SendCommand(DISPLAY_CMD_WRITE_DIGITS, 1, 0, 0, "CIAO");
			break;
		case DISPLAY_CMD_KEYS_PRESSED:
			if ((data->Param1 & KEY_START) == KEY_START) {
				HeaterChamberTempSet(30);
				HeaterSteamGeneratorTempSet(30);
			}
			if ((data->Param1 & KEY_TEST) == KEY_TEST) {
				HeaterChamberTempSet(0);
				HeaterSteamGeneratorTempSet(0);
			}
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



