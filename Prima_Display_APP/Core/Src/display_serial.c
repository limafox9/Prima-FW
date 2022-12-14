/*==============================================================================

Filename: display_serial.c

Created on: Dec 9, 2022

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

==================================================================================
PURPOSE:
USAGE:
================================================================================*/

#include <display_serial.h>
#include <string.h>
#include "crc16.h"

UART_HandleTypeDef* p_activeUART;
DMA_HandleTypeDef* p_activeDMA;

const uint16_t activeTxTimeout = 100;
const char activeProtocolHead[] = {'<', '~'};
const char activeProtocolTail[] =  {'~', '>'};
const char activeProtocolEOF = '\n';

const uint8_t _pos_head = 0;
const uint8_t _pos_command = 2;
const uint8_t _pos_param1 = 4;
const uint8_t _pos_param2 = 8;
const uint8_t _pos_param3 = 12;
const uint8_t _pos_param4 = 16;
const uint8_t _pos_checksum = 20;
const uint8_t _pos_tail = 22;
const uint8_t _pos_EOF = 24;

uint8_t rxData[25] = {0};
uint16_t rxDataPos = 0;

#define BUFFER_SIZE 30
uint8_t uartBuffer[BUFFER_SIZE];

void SerialTransmit(UART_HandleTypeDef* huart, uint8_t* p_Buffer, uint16_t txBufferSize);
void ParseCommand(uint8_t* buffer);
void SwapBytes(void *pv, size_t n);


void DisplaySerialInit(UART_HandleTypeDef* huart, DMA_HandleTypeDef* hdma, uint8_t* buffer, uint16_t bufferSize) {

	p_activeUART = huart;
	p_activeDMA = hdma;
	// Start receive
//	HAL_UARTEx_ReceiveToIdle_DMA(p_activeUART, uartBuffer, UART_BUFFER_SIZE); // Interrupt on receive buffer full or idle (end receive)
//	__HAL_DMA_DISABLE_IT(hdma, DMA_IT_HT); // Disable half buffer received interrupt

}

void DisplaySerialTransmit(UART_HandleTypeDef* huart, uint16_t command, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4) {

	const uint16_t txBufferSize = _pos_EOF + 1;
	uint8_t txBuffer[txBufferSize];
	memset(txBuffer, 0, txBufferSize);

	// Head
	memcpy(txBuffer + _pos_head, activeProtocolHead, 2);
	// Command
	memcpy(txBuffer + _pos_command, &command, 2);
	// Data
	memcpy(txBuffer + _pos_param1, &param1, 4);
	memcpy(txBuffer + _pos_param2, &param2, 4);
	memcpy(txBuffer + _pos_param3, &param3, 4);
	memcpy(txBuffer + _pos_param4, &param4, 4);
	// CRC (Command + Data)
	uint16_t crc;
	crc = crc16(txBuffer + _pos_command, _pos_checksum - _pos_command);
	memcpy(txBuffer + _pos_checksum, &crc, 2);
	// Tail
	memcpy(txBuffer + _pos_tail, activeProtocolTail, 2);
	// EOF
	memcpy(txBuffer + _pos_EOF, &activeProtocolEOF, 1);

	//char* str_txBufferTerminated[10];
	//sprintf(str_txBufferTerminated, "%s%s%s\n", (const char*)s, (const char*)ptr_payload, (const char*)activeProtocolTail.packetTerminator);

	SerialTransmit(huart, txBuffer, txBufferSize);

}

void SerialTransmit(UART_HandleTypeDef* huart, uint8_t* p_Buffer, uint16_t txBufferSize) {

	// Chip 485 ENABLE
	 HAL_GPIO_WritePin(USART1_CTS_GPIO_Port, USART1_CTS_Pin, 1);
	// Transmit
	HAL_UART_Transmit(huart, p_Buffer, txBufferSize, activeTxTimeout);
	// Chip 485 DISABLE
	 HAL_GPIO_WritePin(USART1_CTS_GPIO_Port, USART1_CTS_Pin, 0);

}

DisplaySerialReceive(uint8_t* buffer, uint16_t size) {

	uint16_t rxCRC16;
	uint16_t calcCRC16;

	if ((buffer[_pos_head] == activeProtocolHead[0]) && (buffer[_pos_head + 1] == activeProtocolHead[1])) { // Header found
		if ((buffer[_pos_tail] == activeProtocolTail[0]) && (buffer[_pos_tail + 1] == activeProtocolTail[1])) { // Tail found
			memcpy(&rxCRC16, buffer + _pos_checksum, 2);
			//memset(uartBuffer + _pos_checksum, 0, 2);
			calcCRC16 = crc16(buffer + _pos_command, _pos_checksum - _pos_command);
			if (rxCRC16 == calcCRC16) {
				ParseCommand(buffer); // Valid command
			}
		}
	}
}

void ParseCommand(uint8_t* buffer) {

	uint16_t command;
	uint32_t param1, param2, param3, param4;
	static uint8_t i = 0;

	memcpy(&command, buffer + _pos_command, 2);
	memcpy(&param1, buffer + _pos_param1, 4);
	memcpy(&param2, buffer + _pos_param2, 4);
	memcpy(&param3, buffer + _pos_param3, 4);
	memcpy(&param4, buffer + _pos_param4, 4);

	switch (command) {
		case COMMAND_PLAY_BEEP:
			Beep((uint8_t)param1, (uint16_t)param2, (uint16_t)param3);
			if (i > 1) i = 0;
			if (i == 1) {
				PanelSetAll();
				i++;
			} else {
				PanelClearAll();
				i++;
			}

			break;
	}

	//SerialTransmit("COMMAND OK\n", 11);

}


void SwapBytes(void *pv, size_t n) {
    char *p = pv;
    size_t lo, hi;
    for(lo=0, hi=n-1; hi>lo; lo++, hi--)
    {
        char tmp=p[lo];
        p[lo] = p[hi];
        p[hi] = tmp;
    }
}
