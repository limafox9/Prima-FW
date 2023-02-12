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
#include "programs.h"
#include "modes.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart3_rx;

uint8_t UART_rxBuffer1[UART_DISPLAY_RX_BUFFER_SIZE];
uint8_t UART_rxBuffer[UART_DISPLAY_RX_BUFFER_SIZE];

#define MainBuf_SIZE 128
uint8_t rxBuf[UART_DISPLAY_RX_BUFFER_SIZE];
uint8_t rxMain[UART_DISPLAY_RX_BUFFER_SIZE];
uint8_t dataReady = 0;
s_DisplayProtocolData rxData;

static void ParseCommand(s_DisplayProtocolData* data);
extern void DBInit(UART_HandleTypeDef* huart, GPIO_TypeDef* rs485_CS_GPIOPort, uint16_t rs485_CS_GPIOPin);
extern void DBSend(uint16_t command, uint32_t param1, uint32_t param2, uint32_t param3, char* digits);


void SerialInit(UART_HandleTypeDef* huart, uint32_t baudRate) {

	if (huart->Instance == UART_DISPLAY.Instance) {
		HAL_UART_Receive_DMA(huart, UART_rxBuffer, UART_DISPLAY_RX_BUFFER_SIZE);
	}

//	HAL_UART_Receive_DMA(huart, UART_rxBuffer1, UART_DISPLAY_RX_BUFFER_SIZE);

//	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
//	HAL_UART_Receive_IT(huart, UART_rxBuffer1, UART_DISPLAY_RX_BUFFER_SIZE);
//	HAL_UARTEx_ReceiveToIdle_DMA(huart, UART_rxBuffer, DISPLAY_UART_BUFFER_SIZE);

}

//void SerialInitDB() {
//	DBInit(&UART_DISPLAY, USART3_CTS_GPIO_Port, USART3_CTS_Pin);
//	HAL_UART_Receive_DMA(&UART_DISPLAY, UART_rxBuffer, DB_UART_BUFFER_SIZE);
//}

// TRANSMIT
// ==============================================================

//void SerialSend(UART_HandleTypeDef* huart, uint8_t* data, uint32_t size) {
//	HAL_UART_Transmit(huart, data, size, DISPLAY_UART_BUFFER_SIZE);
////	HAL_UART_Transmit_DMA(huart, data, size);
//}

void SendCommand(uint16_t command, uint32_t param1, uint32_t param2, uint32_t param3, char* digits) {

	s_DisplayProtocolData txData = {command, param1, param2, param3, digits, 1};

//	char buf[8] = {[0 ... 7] = 0};
	memcpy(&txData.Digits, digits, strlen(digits) + 1);

	uint8_t txBuffer[UART_DISPLAY_TX_BUFFER_SIZE];
	Display_DataToProtocol(&txData, txBuffer);

	// Chip 485 ENABLE
	HAL_GPIO_WritePin(USART3_CTS_GPIO_Port, USART3_CTS_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	// Send data
	HAL_UART_Transmit(&UART_DISPLAY, txBuffer, UART_DISPLAY_TX_BUFFER_SIZE, DISPLAY_UART_TX_TIMEOUT);
//	HAL_UART_Transmit_DMA(&UART_DISPLAY, txBuffer, DISPLAY_UART_BUFFER_SIZE);
	// Chip 485 DISABLE
	HAL_Delay(1);
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

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef* huart) {
	if (huart->Instance == UART_DISPLAY.Instance) {

//		memcpy (rxMain, UART_rxBuffer, UART_DISPLAY_RX_BUFFER_SIZE);
	}
}

//void SetBuffer() {
//	for (int i = 0; i < DISPLAY_UART_BUFFER_SIZE; i++) {
//		if ((UART_rxBuffer[i] == '<') && (UART_rxBuffer[i + 1] == '~')) {
//			memcpy (rxMain, &UART_rxBuffer[i], DISPLAY_UART_BUFFER_SIZE - i);
//			memcpy (rxMain, &UART_rxBuffer[0], i);
//			break;
//		}
//	}
//}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
//	void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size) {

	if (huart->Instance == UART_DISPLAY.Instance) {

		uint8_t headFound, tailFound;
		uint8_t tempBuf[UART_DISPLAY_RX_BUFFER_SIZE];

		for (int i = 0; i < UART_DISPLAY_RX_BUFFER_SIZE; i++) {
			if ((UART_rxBuffer[i] == '<') && (UART_rxBuffer[i+1] == '~')) {

				uint8_t p1[UART_DISPLAY_RX_BUFFER_SIZE - i];
				memcpy(p1, UART_rxBuffer + i, UART_DISPLAY_RX_BUFFER_SIZE - i);

				uint8_t p2[i];
				HAL_UART_Receive(huart, p2, i, 10);

				memcpy(tempBuf, p1, UART_DISPLAY_RX_BUFFER_SIZE - i);
				memcpy(tempBuf + UART_DISPLAY_RX_BUFFER_SIZE - i, p2, i);

				Display_ProtocolToData(tempBuf, &rxData);
				if (rxData.Valid) {
					dataReady = 1;
				}

				__NOP();
//				memcpy(tempBuf, UART_rxBuffer + i, UART_DISPLAY_RX_BUFFER_SIZE - i);
//				uint8_t index = i;
//				char nextBuf;
//				HAL_StatusTypeDef hal_status;
//				do {
//					hal_status = HAL_UART_Receive(&UART_DISPLAY, (uint8_t*)&TempBuffer, UART_DISPLAY_RX_BUFFER_SIZE, 10);
//				} while(hal_status != HAL_TIMEOUT);
//
//	//			memcpy(&rxBuf[0], &UART_rxBuffer1[0] + i, UART_DISPLAY_RX_BUFFER_SIZE - i);
//	//			memcpy(&rxBuf[UART_DISPLAY_RX_BUFFER_SIZE - i], &UART_rxBuffer1[0], i);
//				memset(rxBuf, 0, sizeof(rxBuf));
//				uint8_t p0[UART_DISPLAY_RX_BUFFER_SIZE];
//				memcpy(p0,UART_rxBuffer1,UART_DISPLAY_RX_BUFFER_SIZE);
//				uint8_t p1[UART_DISPLAY_RX_BUFFER_SIZE - i];
//				uint8_t p2[i];
//	//			for (int j = 0; j < UART_DISPLAY_RX_BUFFER_SIZE - i; j++) {
//	//				p1[j] == UART_rxBuffer1[UART_DISPLAY_RX_BUFFER_SIZE - i + j];
//	//			}
//	//			for (int j = 0; j < UART_DISPLAY_RX_BUFFER_SIZE; j++) {
//	//				p2[j] == UART_rxBuffer1[j];
//	//			}
//				memcpy(p2, p0, i);
//				memcpy(p1, p0 + i, UART_DISPLAY_RX_BUFFER_SIZE - i);
//	//			memcpy(rxBuf, UART_rxBuffer1 + i, UART_DISPLAY_RX_BUFFER_SIZE - i);
//	//			memcpy(rxBuf + UART_DISPLAY_RX_BUFFER_SIZE - i, UART_rxBuffer1, i);
//				break;

			}
		}

//		OrderBuffer();
////		SetBuffer();
//		Display_ProtocolToData(rxBuf, &rxData);
//		if (rxData.Valid) {
//			dataReady = 1;
//		}
//		memset(UART_rxBuffer1, 255, UART_DISPLAY_RX_BUFFER_SIZE);
//		HAL_UART_Receive_DMA(huart, UART_rxBuffer1, UART_DISPLAY_RX_BUFFER_SIZE);


//		__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);

//		HAL_UART_Receive_IT(huart, UART_rxBuffer1, UART_DISPLAY_RX_BUFFER_SIZE);
//		HAL_UARTEx_ReceiveToIdle_DMA(huart, UART_rxBuffer, DISPLAY_UART_BUFFER_SIZE);
//		__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
	}
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
//	if (huart->Instance == USART2)
//	{
//		oldPos = newPos;  // Update the last position before copying new data
//
//		/* If the data in large and it is about to exceed the buffer size, we have to route it to the start of the buffer
//		 * This is to maintain the circular buffer
//		 * The old data in the main buffer will be overlapped
//		 */
//		if (oldPos+Size > MainBuf_SIZE)  // If the current position + new data size is greater than the main buffer
//		{
//			uint16_t datatocopy = MainBuf_SIZE-oldPos;  // find out how much space is left in the main buffer
//			memcpy ((uint8_t *)MainBuf+oldPos, RxBuf, datatocopy);  // copy data in that remaining space
//
//			oldPos = 0;  // point to the start of the buffer
//			memcpy ((uint8_t *)MainBuf, (uint8_t *)RxBuf+datatocopy, (Size-datatocopy));  // copy the remaining data
//			newPos = (Size-datatocopy);  // update the position
//		}
//
//		/* if the current position + new data size is less than the main buffer
//		 * we will simply copy the data into the buffer and update the position
//		 */
//		else
//		{
//			memcpy ((uint8_t *)MainBuf+oldPos, RxBuf, Size);
//			newPos = Size+oldPos;
//		}

//	uint8_t pos;
//	uint8_t temp[UART_DISPLAY_RX_BUFFER_SIZE];
//
////	memcpy(temp, buffer, UART_DISPLAY_RX_BUFFER_SIZE);
//
//	for (int i = 0; i < UART_DISPLAY_RX_BUFFER_SIZE; i++) {
//		if ((UART_rxBuffer1[i] == '<') && (UART_rxBuffer1[i+1] == '~')) {
////			memcpy(&rxBuf[0], &UART_rxBuffer1[0] + i, UART_DISPLAY_RX_BUFFER_SIZE - i);
////			memcpy(&rxBuf[UART_DISPLAY_RX_BUFFER_SIZE - i], &UART_rxBuffer1[0], i);
//			memset(rxBuf, 0, sizeof(rxBuf));
//			uint8_t p0[UART_DISPLAY_RX_BUFFER_SIZE];
//			memcpy(p0,UART_rxBuffer1,UART_DISPLAY_RX_BUFFER_SIZE);
//			uint8_t p1[UART_DISPLAY_RX_BUFFER_SIZE - i];
//			uint8_t p2[i];
////			for (int j = 0; j < UART_DISPLAY_RX_BUFFER_SIZE - i; j++) {
////				p1[j] == UART_rxBuffer1[UART_DISPLAY_RX_BUFFER_SIZE - i + j];
////			}
////			for (int j = 0; j < UART_DISPLAY_RX_BUFFER_SIZE; j++) {
////				p2[j] == UART_rxBuffer1[j];
////			}
//			memcpy(p2, p0, i);
//			memcpy(p1, p0 + i, UART_DISPLAY_RX_BUFFER_SIZE - i);
////			memcpy(rxBuf, UART_rxBuffer1 + i, UART_DISPLAY_RX_BUFFER_SIZE - i);
////			memcpy(rxBuf + UART_DISPLAY_RX_BUFFER_SIZE - i, UART_rxBuffer1, i);
//			break;
//		}
//	}

		/* start the DMA again */
//	HAL_UARTEx_ReceiveToIdle_DMA(huart, UART_rxBuffer1, UART_DISPLAY_RX_BUFFER_SIZE);
//	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);

//	}
}
void OrderBuffer() {

	uint8_t pos;
	uint8_t temp[UART_DISPLAY_RX_BUFFER_SIZE];

//	memcpy(temp, buffer, UART_DISPLAY_RX_BUFFER_SIZE);

	for (int i = 0; i < UART_DISPLAY_RX_BUFFER_SIZE; i++) {
		if ((UART_rxBuffer1[i] == '<') && (UART_rxBuffer1[i+1] == '~')) {
//			memcpy(&rxBuf[0], &UART_rxBuffer1[0] + i, UART_DISPLAY_RX_BUFFER_SIZE - i);
//			memcpy(&rxBuf[UART_DISPLAY_RX_BUFFER_SIZE - i], &UART_rxBuffer1[0], i);
			memset(rxBuf, 0, sizeof(rxBuf));
			uint8_t p0[UART_DISPLAY_RX_BUFFER_SIZE];
			memcpy(p0,UART_rxBuffer1,UART_DISPLAY_RX_BUFFER_SIZE);
			uint8_t p1[UART_DISPLAY_RX_BUFFER_SIZE - i];
			uint8_t p2[i];
//			for (int j = 0; j < UART_DISPLAY_RX_BUFFER_SIZE - i; j++) {
//				p1[j] == UART_rxBuffer1[UART_DISPLAY_RX_BUFFER_SIZE - i + j];
//			}
//			for (int j = 0; j < UART_DISPLAY_RX_BUFFER_SIZE; j++) {
//				p2[j] == UART_rxBuffer1[j];
//			}
			memcpy(p2, p0, i);
			memcpy(p1, p0 + i, UART_DISPLAY_RX_BUFFER_SIZE - i);
//			memcpy(rxBuf, UART_rxBuffer1 + i, UART_DISPLAY_RX_BUFFER_SIZE - i);
//			memcpy(rxBuf + UART_DISPLAY_RX_BUFFER_SIZE - i, UART_rxBuffer1, i);
			break;
		}
	}
}

void StartSerialTask() {

	for (;;) {
//		HAL_UART_Receive(&UART_DISPLAY, UART_rxBuffer1, 29, 50);

//		s_DisplayProtocolData rxData;
//		Display_ProtocolToData(UART_rxBuffer, &rxData);
		if (dataReady) {
			dataReady = 0;
			ParseCommand(&rxData); // valid received command > parse
			memset(rxBuf, 0, sizeof(rxBuf));
		}
//		memset(UART_rxBuffer,0,29);
		osDelay(50);
	}
}
//void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size) {
//	s_DisplayProtocolData rxData;
//	static uint16_t oldPos, newPos;
//
//	if (huart->Instance == UART_DISPLAY.Instance) {
//		oldPos = newPos;  // Update the last position before copying new data
//
//		/* If the data in large and it is about to exceed the buffer size, we have to route it to the start of the buffer
//		 * This is to maintain the circular buffer
//		 * The old data in the main buffer will be overlapped
//		 */
//		if (oldPos+Size > MainBuf_SIZE)  // If the current position + new data size is greater than the main buffer
//		{
//			uint16_t datatocopy = MainBuf_SIZE-oldPos;  // find out how much space is left in the main buffer
//			memcpy ((uint8_t *)rxMain+oldPos, UART_rxBuffer, datatocopy);  // copy data in that remaining space
//
//			oldPos = 0;  // point to the start of the buffer
//			memcpy ((uint8_t *)rxMain, (uint8_t *)UART_rxBuffer+datatocopy, (Size-datatocopy));  // copy the remaining data
//			newPos = (Size-datatocopy);  // update the position
//		}
//
//		/* if the current position + new data size is less than the main buffer
//		 * we will simply copy the data into the buffer and update the position
//		 */
//		else
//		{
//			memcpy ((uint8_t *)rxMain+oldPos, UART_rxBuffer, Size);
//			newPos = Size+oldPos;
//		}
//
//		//		osDelay(50);
//		Display_ProtocolToData(rxMain, &rxData);
//		if (rxData.Valid) {
//			ParseCommand(&rxData); // valid received command > parse
//		}
//	}
//
//	HAL_UART_Transmit(&huart1, UART_rxBuffer, 32, 100);
//	s_DisplayProtocolData rxData;
//	if (huart->Instance == UART_DISPLAY.Instance) {
////		memcpy ((uint8_t *)rxMain, UART_rxBuffer, Size);
//		Display_ProtocolToData(UART_rxBuffer, &rxData);
//		HAL_UART_Receive(&UART_DISPLAY, UART_rxBuffer, DISPLAY_UART_BUFFER_SIZE, DISPLAY_UART_TX_TIMEOUT);
//		if (rxData.Valid) {
//			ParseCommand(&rxData); // valid received command > parse
//		}
//	}
//
////	HAL_UART_Receive_DMA(huart, UART_rxBuffer, DISPLAY_UART_BUFFER_SIZE);
//	HAL_UARTEx_ReceiveToIdle_DMA(huart, UART_rxBuffer, DISPLAY_UART_BUFFER_SIZE);
//	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
//}

// COMMANDS
// ==============================================================

static void ParseCommand(s_DisplayProtocolData* data) {



	if (data->Param1 == KEY_START) {
		if (machineStatus == OP_IDLE) {
			StartProgram();
		} else if (machineStatus == OP_READY) {
			StartProgram();
		} else {
			__NOP();
		}

	} else if (data->Param1 == KEY_TEST) {
		if (machineStatus == OP_IDLE) {
			SelectTest();
		} else if (machineStatus == OP_READY) {
			SelectTest();
		} else {
			__NOP();
		}

	} else if (data->Param1 == KEY_PROG) {
		if (machineStatus == OP_IDLE) {
			SelectProgram();
		} else if (machineStatus == OP_READY) {
			SelectProgram();
		} else {
			__NOP();
		}

	} else if (data->Param1 == KEY_START + KEY_LONG_PRESS) {
		// Button code here
	} else if (data->Param1 == KEY_TEST + KEY_LONG_PRESS) {
		// Button code here
	} else if (data->Param1 == KEY_PROG + KEY_LONG_PRESS) {
		// Button code here
	} else if (data->Param1 == KEY_START + KEY_TEST) {
		// Button code here
	} else if (data->Param1 == KEY_START + KEY_PROG) {
		// Button code here
	} else if (data->Param1 == KEY_TEST + KEY_PROG) {
		// Button code here
	} else if (data->Param1 == KEY_START + KEY_TEST + KEY_PROG) {
		// Button code here
	} else if (data->Param1 == KEY_START + KEY_TEST + KEY_LONG_PRESS) {
		switch (machineStatus) {
		case OP_IDLE:
			SelectProgram();
			break;
		default:
			__NOP();
			break;
		}

	} else if (data->Param1 == KEY_START + KEY_PROG + KEY_LONG_PRESS) {
		// Button code here
	} else if (data->Param1 == KEY_TEST + KEY_PROG + KEY_LONG_PRESS) {
		// Button code here
	} else if (data->Param1 == KEY_START + KEY_TEST + KEY_PROG + KEY_LONG_PRESS) {
		// Button code here
	} else {
		__NOP();
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



