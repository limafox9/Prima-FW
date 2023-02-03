/*==============================================================================

Filename: globals.c

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


#include "globals.h"

#include "system.h"

#include "settings.h"

#include <stdio.h>
#include <stdarg.h>

extern UART_HandleTypeDef huart1;
extern MachineData_s machineData;

#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)

uint32_t _UID[3];

#define DIGIT_BUFFER_SIZE 10
uint8_t digitsRow1[DIGIT_BUFFER_SIZE];
uint8_t digitsRow2[DIGIT_BUFFER_SIZE];
uint8_t digitsRow3[DIGIT_BUFFER_SIZE];

uint16_t progressBarBits;
uint16_t progressBarBitsBlink;

uint16_t dashboardLedsBits;
uint16_t panelLedsBitsBlink;

uint32_t digitsBits;

uint8_t keysPressed;

uint32_t GetSafeDeltaTick(uint32_t startTick, uint32_t endTick) {

	uint32_t deltaTick;

	// In case of HAL_GetTick() overflow (~49 days). Counter restarts from 0.
	if (endTick < startTick) {
		deltaTick = MAX_VALUE(startTick) - startTick + endTick;
	} else {
		deltaTick = endTick - startTick;
	}
	return deltaTick;
}

void SwapBytes(void *pBytes, size_t size) {
    char *p = pBytes;
    size_t lo, hi;
    for (lo=0, hi=size-1; hi>lo; lo++, hi--)
    {
        char tmp=p[lo];
        p[lo] = p[hi];
        p[hi] = tmp;
    }
}

//void Log(LOG_LEVEL logLevel, const char* format, ...) {
//
//	if (logLevel <= ACTIVE_LOG_LEVEL) {
//		  va_list args;
//		  char* buf[80];
//		  va_start(args, format);
//		  vsprintf(buf, format, args);
//		  //printf(buf);
//		  va_end(args);
//
//		  printf("%s\n",buf);
////		  HAL_UART_Transmit(&huart1, buf, strlen(buf), 100);
////		  HAL_UART_Transmit(&huart1, "\n", 1, 100);
//	}
//}

// Redirect printf to serial monitor
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}

