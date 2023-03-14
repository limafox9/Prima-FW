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
#include "globals.h"
#include "Inc/DB_Protocol.h"
#include "actuators.h"
#include "programs.h"
#include "modes.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart3_rx;

extern osThreadId programTaskHandle;

uint8_t UART_rxBuffer1[UART_DISPLAY_RX_BUFFER_SIZE];
volatile uint8_t UART_rxBuffer[UART_DISPLAY_RX_BUFFER_SIZE];

#define MainBuf_SIZE 128
//uint8_t rxBuf[UART_DISPLAY_RX_BUFFER_SIZE];
uint8_t rxMain[UART_DISPLAY_RX_BUFFER_SIZE];
uint8_t dataReady = 0;
s_DisplayProtocolData rxData;

//static void ParseCommand(s_DisplayProtocolData* data);
extern void DBInit(UART_HandleTypeDef* huart, GPIO_TypeDef* rs485_CS_GPIOPort, uint16_t rs485_CS_GPIOPin);
extern void DBSend(uint16_t command, uint32_t param1, uint32_t param2, uint32_t param3, char* digits);


void SerialInit(UART_HandleTypeDef* huart, uint32_t baudRate) {

//	HAL_UART_Receive_DMA(huart, UART_rxBuffer, UART_DISPLAY_RX_BUFFER_SIZE);

	HAL_UARTEx_ReceiveToIdle_DMA(huart, UART_rxBuffer, UART_DISPLAY_RX_BUFFER_SIZE);
	return;
	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
}

void SendCommand(uint16_t command, uint32_t param1, uint32_t param2, uint32_t param3, char* digits) {

	s_DisplayProtocolData txData = {command, param1, param2, param3, digits, 1};

	memcpy(&txData.Digits, digits, strlen(digits) + 1);

	uint8_t txBuffer[UART_DISPLAY_TX_BUFFER_SIZE + 10];
	memset(txBuffer, 255, sizeof(txBuffer));
	Display_DataToProtocol(&txData, txBuffer);

	// Chip 485 ENABLE
	HAL_GPIO_WritePin(USART3_CTS_GPIO_Port, USART3_CTS_Pin, GPIO_PIN_SET);
	HAL_Delay(5);
	// Send data
	HAL_UART_Transmit(&UART_DISPLAY, txBuffer, UART_DISPLAY_TX_BUFFER_SIZE, DISPLAY_UART_TX_TIMEOUT);
	// Chip 485 DISABLE
	HAL_Delay(5);
	HAL_GPIO_WritePin(USART3_CTS_GPIO_Port, USART3_CTS_Pin, GPIO_PIN_RESET);
}

// RECEIVE
// ==============================================================

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {

	uint8_t headFound, tailFound;
	uint8_t tempBuf[UART_DISPLAY_RX_BUFFER_SIZE];

	for (int i = UART_DISPLAY_RX_BUFFER_SIZE; i >= 0; i--) {
		if ((UART_rxBuffer[i] == '<') && (UART_rxBuffer[i+1] == '~')) {

			HAL_Delay(5);

			uint8_t p2[i];
			memcpy(p2, UART_rxBuffer, i);

			uint8_t p1[UART_DISPLAY_RX_BUFFER_SIZE - i];
			memcpy(p1, UART_rxBuffer + i, UART_DISPLAY_RX_BUFFER_SIZE - i);

			memcpy(tempBuf, p1, UART_DISPLAY_RX_BUFFER_SIZE - i);
			memcpy(tempBuf + UART_DISPLAY_RX_BUFFER_SIZE - i, p2, i);

			Display_ProtocolToData(tempBuf, &rxData);
			if (rxData.Valid) {
				dataReady = 1;
			}
		}
	}

	HAL_UARTEx_ReceiveToIdle_DMA(huart, UART_rxBuffer, UART_DISPLAY_RX_BUFFER_SIZE);
	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);


}


//void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
//
//	if (huart->Instance == UART_DISPLAY.Instance) {
//
////		osDelay(10);
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
//					dataReady = 1;
//				}
//			}
//		}
//
////		for (int i = 0; i < UART_DISPLAY_RX_BUFFER_SIZE; i++) {
////			if ((UART_rxBuffer[i] == '<') && (UART_rxBuffer[i+1] == '~')) {
////
////				uint8_t p2[i];
////				HAL_UART_Receive(huart, p2, i, 50);
////
////				uint8_t p1[UART_DISPLAY_RX_BUFFER_SIZE - i];
////				memcpy(p1, UART_rxBuffer + i, UART_DISPLAY_RX_BUFFER_SIZE - i);
////
////				memcpy(tempBuf, p1, UART_DISPLAY_RX_BUFFER_SIZE - i);
////				memcpy(tempBuf + UART_DISPLAY_RX_BUFFER_SIZE - i, p2, i);
////
////				Display_ProtocolToData(tempBuf, &rxData);
////				if (rxData.Valid) {
////					dataReady = 1;
////				}
////			}
////		}
//
//		HAL_UART_Receive_DMA(huart, UART_rxBuffer, UART_DISPLAY_RX_BUFFER_SIZE);
////		__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
//	}
//}

void StartSerialTask() {

	for (;;) {
		if (dataReady) {
			dataReady = 0;
			ParseCommand(&rxData); // valid received command > parse
//			memset(rxBuf, 0, sizeof(rxBuf));
		}
		osDelay(10);
	}
}

// COMMANDS
// ==============================================================
//
//static void ParseCommand(s_DisplayProtocolData* data) {
//
//	if (data->Param1 == KEY_PROG) {
//		if (machineStatus == OP_IDLE) {
//			SelectProgram();
//		} else if (machineStatus == OP_READY) {
//			SelectProgram();
//		} else if (machineStatus == OP_PARAMETERS) {
//			SelectParameter();
//		} else if (machineStatus == OP_RUNNING) {
//			SelectReading();
//			ShowReading();
//		} else if (machineStatus == OP_READINGS) {
//			SelectReading();
//		} else {
//			__NOP();
//		}
//
//	} else if (data->Param1 == KEY_TEST) {
//		if (machineStatus == OP_IDLE) {
//			SelectTest();
//		} else if (machineStatus == OP_READY) {
//			SelectTest();
//		} else if (machineStatus == OP_PARAMETERS) {
//			ChangeParameter(1);
//		} else {
//			__NOP();
//		}
//
//	} else if (data->Param1 == KEY_START) {
//		if (machineStatus == OP_IDLE) {
//			programStart = 1;
//			machineStatus == OP_RUNNING;
//			osThreadResume(programTaskHandle);
//		} else if (machineStatus == OP_READY) {
//			programStart = 1;
//			machineStatus == OP_RUNNING;
//			osThreadResume(programTaskHandle);
//		} else if (machineStatus == OP_PARAMETERS) {
//			ChangeParameter(-1);
//		} else {
//			__NOP();
//		}
//
//	} else if (data->Param1 == KEY_PROG + KEY_LONG_PRESS) {
//		// Button code here
//
//	} else if (data->Param1 == KEY_TEST + KEY_LONG_PRESS) {
//		// Button code here
//
//	} else if (data->Param1 == KEY_START + KEY_LONG_PRESS) {
//		if (lastError != 0) {
//			currProgType =
//			currProgram = 99;
//			machineStatus = machineStatusEx;
//			ShowProgram();
//		} else if (machineStatus == OP_IDLE) {
//		__NOP();
//		} else if (machineStatus == OP_READY) {
//			__NOP();
//		} else if (machineStatus == OP_PARAMETERS) {
//			machineStatus = machineStatusEx;
//			ShowProgram();
//		} else if (machineStatus == OP_RUNNING) {
//			abortProgram = 1;
//			ShowProgram();
//		} else if (machineStatus == OP_READINGS) {
//			machineStatus = machineStatusEx;
//			ShowProgram();
//		} else {
//			__NOP();
//		}
//
//	} else if (data->Param1 == KEY_TEST + KEY_PROG) {
//		// Button code here
//	} else if (data->Param1 == KEY_START + KEY_PROG) {
//		// Button code here
//	} else if (data->Param1 == KEY_START + KEY_TEST) {
//		// Button code here
//	} else if (data->Param1 == KEY_START + KEY_TEST + KEY_PROG) {
//		// Button code here
//
//	} else if (data->Param1 == KEY_TEST + KEY_PROG + KEY_LONG_PRESS) {
//		// Button code here
//
//	} else if (data->Param1 == KEY_START + KEY_TEST + KEY_LONG_PRESS) {
//		if (machineStatus == OP_IDLE) {
//			machineStatusEx = machineStatus;
//			machineStatus = OP_PARAMETERS;
//			ShowParameter();
//		} else if (machineStatus == OP_READY) {
//			__NOP();
//		} else if (machineStatus == OP_PARAMETERS) {
//			__NOP();
//		} else {
//			__NOP();
//		}
//
//	} else if (data->Param1 == KEY_START + KEY_PROG + KEY_LONG_PRESS) {
//		if (machineStatus == OP_IDLE) {
//			machineStatusEx = machineStatus;
//			machineStatus = OP_READINGS;
//			ShowReading();
//		} else if (machineStatus == OP_READY) {
//			__NOP();
//		} else if (machineStatus == OP_PARAMETERS) {
//			__NOP();
//		} else {
//			__NOP();
//		}
//
//	} else if (data->Param1 == KEY_START + KEY_TEST + KEY_PROG + KEY_LONG_PRESS) {
//		// Button code here
//	} else {
//		__NOP();
//	}
//}


