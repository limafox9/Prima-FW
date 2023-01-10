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

#include "serial.h"

#include <stdio.h>
#include "main.h"

#define SERIAL_BUFFER_SIZE 32

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

uint8_t serialBuffer[SERIAL_BUFFER_SIZE];

void SerialInit() {
	HAL_UART_Receive_DMA(&huart1, serialBuffer, SERIAL_BUFFER_SIZE);
	HAL_UART_Receive_DMA(&huart3, serialBuffer, SERIAL_BUFFER_SIZE);
}

void SerialSend(UART_HandleTypeDef* huart, uint8_t* data, uint32_t size) {
	HAL_UART_Transmit_DMA(huart, data, size);
}

void SerialReceived(UART_HandleTypeDef* huart) {
	if (huart->Instance == huart3.Instance) {
		PanelSerialReceive(serialBuffer, SERIAL_BUFFER_SIZE);
	}
}

// =============================================================================

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef* huart) {

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {

}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef* huart) {

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
	HAL_UART_Receive_DMA(huart, serialBuffer, SERIAL_BUFFER_SIZE);
//	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
	SerialReceived(huart);
}



