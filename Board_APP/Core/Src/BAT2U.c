/*==============================================================================

Filename: BAT2U.c

Created on: Jan 12, 2023

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

#include "BAT2U.h"

#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>

UART_HandleTypeDef* huart_water;

static int16_t ByteInBuffer(uint8_t byteFind, uint8_t* bufferSearch, uint16_t bufferSize);
static uint8_t GetChecksum(uint8_t* buffer, uint16_t size);
static uint8_t Timeout(uint32_t timeoutMilliseconds);

void BAT2U_Init(UART_HandleTypeDef* huart) {
	huart_water = huart;
	huart_water->Init.BaudRate = 2400;
}

uint16_t BAT2U_GetTDSValue(uint8_t channel, uint16_t timeout) {
	int16_t pos;
	uint16_t value;
	uint8_t rxBuffer[6];
	uint8_t txBuffer[6] = {0xA0, 0x00, 0x00, 0x00, 0x00, 0x00};
	txBuffer[5] = GetChecksum(txBuffer, 5);
	HAL_UART_Transmit(huart_water, txBuffer, 6, 1000);
	Timeout(0);
	do {
		HAL_UART_Receive_DMA(huart_water, rxBuffer, 6);
		pos = ByteInBuffer(0xAA, rxBuffer, 6);
		osDelay(100);
	} while ((pos < 0) && (!Timeout(timeout)));
	if (channel ==1) {
		value = rxBuffer[1] << 8 | rxBuffer[2];
	} else if (channel == 2) {
		value = rxBuffer[3] << 8 | rxBuffer[4];
	}
	return value;
}

uint16_t BAT2U_GetTempValue(uint8_t channel, uint16_t timeout) {
	int16_t pos;
	uint16_t value = 0;
	uint8_t rxBuffer[6];
	uint8_t txBuffer[6] = {0xA0, 0x00, 0x00, 0x00, 0x00, 0x00};
	txBuffer[5] = GetChecksum(txBuffer, 5);
	HAL_UART_Transmit(huart_water, txBuffer, 6, 1000);
	Timeout(0);
	do {
		HAL_UART_Receive_DMA(huart_water, rxBuffer, 6);
		pos = ByteInBuffer(0xAB, rxBuffer, 6);
		osDelay(100);
	} while ((pos < 0) && (!Timeout(timeout)));
	if (channel == 1) {
		value = rxBuffer[1] << 8 | rxBuffer[2];
	} else if (channel == 2) {
		value = rxBuffer[3] << 8 | rxBuffer[4];
	}
	return value;
}

// Calibration @25°C +/-5°C. Both channels at the same time.
uint16_t BAT2U_TDSCalibration(uint16_t timeout) {
	int16_t pos;
	uint8_t rxBuffer[6];
	uint8_t txBuffer[6] = {0xA6, 0x00, 0x00, 0x00, 0x00, 0x00};
	txBuffer[5] = GetChecksum(txBuffer, 5);
	HAL_UART_Transmit(huart_water, txBuffer, 6, 1000);
	Timeout(0);
	do {
		HAL_UART_Receive_DMA(huart_water, rxBuffer, 6);
		pos = ByteInBuffer(0xAC, rxBuffer, 6);
		osDelay(100);
	} while ((pos < 0) && (!Timeout(timeout)));
	return (pos >= 0) ? 1 : 0;
}

// Set NTC resistance at room temperature
uint16_t BAT2U_NTCCalibration(uint8_t channel, uint16_t resistanceOhm, uint16_t timeout) {
	int16_t pos;
	uint8_t rxBuffer[6];
	uint8_t txBuffer[6] = {0xA6, 0x00, 0x00, 0x00, 0x00, 0x00};
	txBuffer[5] = GetChecksum(txBuffer, 5);
	HAL_UART_Transmit(huart_water, txBuffer, 6, 1000);
	Timeout(0);
	do {
		HAL_UART_Receive_DMA(huart_water, rxBuffer, 6);
		pos = ByteInBuffer(0xAC, rxBuffer, 6);
		osDelay(100);
	} while ((pos < 0) && (!Timeout(timeout)));
	return (pos >= 0) ? 1 : 0;
}

static int16_t ByteInBuffer(uint8_t byteFind, uint8_t* bufferSearch, uint16_t bufferSize) {
	int16_t ret = -1;
	for (uint16_t i = 0; i < bufferSize; i++) {
		if (bufferSearch[i] == byteFind) {
			ret = 1;
			break;
		}
	}
	return ret;
}

static uint8_t GetChecksum(uint8_t* buffer, uint16_t size) {
	uint32_t sum = 0;
	for (uint16_t i = 0; i < size; i++) {
		sum += buffer[i];
	}
}

static uint8_t Timeout(uint32_t timeoutMilliseconds) {
	static uint32_t lastTick;
	if (timeoutMilliseconds == 0) {
		lastTick = HAL_GetTick();
		return 0;
	}
	if ((HAL_GetTick() - lastTick) > timeoutMilliseconds) {
		return 1;
	}

}
