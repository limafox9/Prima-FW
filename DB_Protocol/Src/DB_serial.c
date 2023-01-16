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

#include "DB_serial.h"
#include "DB_globals.h"
#include "DB_protocol.h"
#include <stdio.h>

//uint8_t serialTXBuffer[SERIAL_TX_BUFFER_SIZE];
uint8_t serialRXBuffer[DB_UART_BUFFER_SIZE];

UART_HandleTypeDef* pDBhuart;
GPIO_TypeDef* pRS485_CS_GPIOPort;
uint16_t pRS485_CS_Pin;


void DBInit(UART_HandleTypeDef* huart, GPIO_TypeDef* rs485_CS_GPIOPort, uint16_t rs485_CS_GPIOPin) {

	pRS485_CS_GPIOPort = rs485_CS_GPIOPort;
	pRS485_CS_Pin = rs485_CS_GPIOPin;
	huart->Init.BaudRate = DB_UART_SPEED;
	pDBhuart = huart;
	HAL_UART_Receive_DMA(huart, serialRXBuffer, DB_UART_BUFFER_SIZE);
}

void DBSendCommand(uint16_t command, uint32_t param1, uint32_t param2, uint32_t param3, char* digits) {

	s_DisplayProtocolData txData = {command, param1, param2, param3, digits, 1};

	uint8_t txBuffer[DB_UART_BUFFER_SIZE];
	Display_DataToProtocol(&txData, txBuffer);

	// Chip 485 ENABLE
	HAL_GPIO_WritePin(pRS485_CS_GPIOPort, pRS485_CS_Pin, GPIO_PIN_SET);
	// Send data
	HAL_UART_Transmit(pDBhuart, txBuffer, DB_UART_BUFFER_SIZE, DB_UART_TX_TIMEOUT);
	// Chip 485 DISABLE
	HAL_GPIO_WritePin(pRS485_CS_GPIOPort, pRS485_CS_Pin, GPIO_PIN_RESET);
}

void DBSendDigits(uint8_t row, char* digits) {

	DBSendCommand(DISPLAY_CMD_WRITE_DIGITS, row, 0, 0, digits);
}

//
//__weak void DBSerialReceived(uint8_t* data, uint32_t size) {
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
//	DBSerialReceived(huart, serialRXBuffer, DB_UART_BUFFER_SIZE);
//	HAL_UART_Receive_DMA(huart, serialRXBuffer, DB_UART_BUFFER_SIZE);
//	//	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
//}



