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

==================================================================================
PURPOSE: Modbus CRC16 calculation
USAGE:
================================================================================*/

// 3C7EDB0002000000640000006400000000000000CCCE7E3E10 -> 2 beeps

#include "panel_serial.h"
#include "crc16.h"
#include "main.h"
#include "keys.h"
//#include "leds.h"

extern uint8_t _keysBeepActive;

UART_HandleTypeDef* pActiveUART;

const char activeProtocolHead[] = {'<', '~'};
const char activeProtocolTail[] =  {'~', '>'};
const char activeProtocolEOF = '\n';

const uint8_t pos_head = 0;
const uint8_t pos_command = 2;
const uint8_t pos_param1 = 4;
const uint8_t pos_param2 = 8;
const uint8_t pos_param3 = 12;
const uint8_t pos_digits = 16;
const uint8_t pos_checksum = 24;
const uint8_t pos_tail = 26;
const uint8_t pos_EOF = 28;

void SerialTransmit(uint8_t* data, uint16_t dataSize);


void PanelSerialInit(UART_HandleTypeDef* pUART) {
	pActiveUART = pUART;
}

void PanelSerialTransmit(uint16_t command, uint32_t param1, uint32_t param2, uint32_t param3, char* digits) {

	const uint16_t txBufferSize = pos_EOF + 1;

	uint8_t txBuffer[txBufferSize];
	memset(txBuffer, 0, txBufferSize);

	// Head
	memcpy(txBuffer + pos_head, activeProtocolHead, 2);
	// Command
	memcpy(txBuffer + pos_command, &command, 2);
	// Data
	memcpy(txBuffer + pos_param1, param1, 4);
	memcpy(txBuffer + pos_param2, &param2, 4);
	memcpy(txBuffer + pos_param3, &param3, 4);
	memcpy(txBuffer + pos_digits, digits, 8);
	// CRC (Command + Data)
	uint16_t crc;
	crc = CRC16(txBuffer + pos_command, pos_checksum - pos_command);
	memcpy(txBuffer + pos_checksum, &crc, 2);
	// Tail
	memcpy(txBuffer + pos_tail, activeProtocolTail, 2);
	// EOF
	memcpy(txBuffer + pos_EOF, &activeProtocolEOF, 1);

	SerialTransmit(txBuffer, txBufferSize);

}

void SerialTransmit(uint8_t* data, uint16_t dataSize) {

	// Chip 485 ENABLE
	HAL_GPIO_WritePin(USART3_CTS_GPIO_Port, USART3_CTS_Pin, 1);
	// UART Transmit
	HAL_UART_Transmit(pActiveUART, data, dataSize, PANEL_SERIAL_TX_TIMEOUT);
	// Chip 485 DISABLE
	HAL_GPIO_WritePin(USART3_CTS_GPIO_Port, USART3_CTS_Pin, 0);

}

#define BUFFER_SIZE 30
uint16_t newPos = 0;
uint16_t oldPos = 0;
uint8_t MainBuf[BUFFER_SIZE];


void PanelSerialReceive(uint8_t* buffer, uint16_t size) {

	uint16_t rxCRC16;
	uint16_t calcCRC16;


	oldPos = newPos;  // Update the last position before copying new data

	/* If the data in large and it is about to exceed the buffer size, we have to route it to the start of the buffer
	 * This is to maintain the circular buffer
	 * The old data in the main buffer will be overlapped
	 */
	if ((oldPos + size) > BUFFER_SIZE)  // If the current position + new data size is greater than the main buffer
	{
		uint16_t datatocopy = BUFFER_SIZE - oldPos;  // find out how much space is left in the main buffer
		memcpy ((uint8_t *)MainBuf+oldPos, buffer, datatocopy);  // copy data in that remaining space

		oldPos = 0;  // point to the start of the buffer
		memcpy ((uint8_t *)MainBuf, (uint8_t *)buffer + datatocopy, (size - datatocopy));  // copy the remaining data
		newPos = (size - datatocopy);  // update the position
	}

	/* if the current position + new data size is less than the main buffer
	 * we will simply copy the data into the buffer and update the position
	 */
	else
	{
		memcpy ((uint8_t *)MainBuf + oldPos, buffer, size);
		newPos = size + oldPos;
	}


	if ((buffer[pos_head] == activeProtocolHead[0]) && (buffer[pos_head + 1] == activeProtocolHead[1])) { // Header found
		if ((buffer[pos_tail] == activeProtocolTail[0]) && (buffer[pos_tail + 1] == activeProtocolTail[1])) { // Tail found
			memcpy(&rxCRC16, buffer + pos_checksum, 2);
			//memset(uartBuffer + _pos_checksum, 0, 2);
			calcCRC16 = CRC16(buffer + pos_command, pos_checksum - pos_command);
//			if (rxCRC16 == calcCRC16) {
//				ParseCommand(buffer); // Valid command
//			}
			ParseCommand(buffer);
		}
	}
}

void ParseCommand(uint8_t* buffer) {

	uint16_t command;
	uint32_t param1, param2, param3;
	char digitsBuffer[8] = {0};

	uint64_t message = 0;
	static uint8_t i = 0;

	memcpy(&command, buffer + pos_command, 2);
	memcpy(&param1, buffer + pos_param1, 4);
	memcpy(&param2, buffer + pos_param2, 4);
	memcpy(&param3, buffer + pos_param3, 4);
	memcpy(digitsBuffer, buffer + pos_digits, 8);

	switch (command) {
	case COMMAND_PLAY_BEEP:
		Beep(param1, param2, param3);
		break;
	case COMMAND_KEYS_PRESSED:
		KeysReceived(param1);
		break;
//		case COMMAND_WRITE_DIGITS_ROW_1:
//			DigitsWriteRow1(&param1);
//			break;
//		case COMMAND_WRITE_DIGITS_ROW_2:
//			DigitsWriteRow2(&param1);
//			break;
//		case COMMAND_WRITE_DIGITS_ROW_3:
//			DigitsWriteRow3(&param1);
//			break;
//		case COMMAND_SET_DASHBOARD_LEDS:
//			PanelWriteLeds((uint16_t)param1);
//			break;
//		case COMMAND_SET_PROGRESS_LEDS:
//			ProgressWriteLeds((uint16_t)param1);
//			break;
//		case COMMAND_SET_PANEL_KEYS_BEEP:
//			_keysBeepActive = (uint8_t)param1;
//			break;
	}

	//SerialTransmit("COMMAND OK\n", 11);

}
