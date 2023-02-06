/*==============================================================================

Filename: serial.h

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



#ifndef INC_SERIAL_H_
#define INC_SERIAL_H_

#include <stdio.h>
#include "main.h"

//#define UART_MONITOR 	huart1
#define UART_TDS		huart2
#define UART_DISPLAY 	huart3
#define UART_DISPLAY_BUFFER_SIZE 64		// default 30 (bytes)

void SerialInit(UART_HandleTypeDef* huart, uint32_t baudRate);
//void SerialReceived(UART_HandleTypeDef* huart, uint8_t* data, uint32_t size);
void SerialSend(UART_HandleTypeDef* huart, uint8_t* data, uint32_t size);


#endif /* INC_SERIAL_H_ */
