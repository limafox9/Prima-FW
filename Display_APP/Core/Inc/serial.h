/*==============================================================================

Filename: serial.h

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
PURPOSE:
USAGE:
================================================================================*/

#ifndef INC_SERIAL_H_
#define INC_SERIAL_H_

#include <stdint.h>

#define DISPLAY_UART_HANDLE				huart1

#define DISPLAY_DIGIT_ROWS 				3
#define DISPLAY_DIGIT_COLUMNS			4
#define DISPLAY_DIGIT_ROW_BUFFER_SIZE 	8

#define UART_DISPLAY_TX_BUFFER_SIZE 29
#define UART_DISPLAY_RX_BUFFER_SIZE 29

void DisplayUARTInit();
void SerialTask();
void SendCommand(uint16_t command, uint32_t param1, uint32_t param2, uint32_t param3, char* digits);

#endif /* INC_SERIAL_H_ */
