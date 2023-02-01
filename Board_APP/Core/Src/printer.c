/*==============================================================================

Filename: printer.c

Created on: Jan 27, 2023

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
#include <stdarg.h>

#include "printer.h"

#include "time.h"
#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart4;

void PrinterInit() {
	char d[10];

	Print("%c%c", 0x1b, '@'); // Reset
	Print("%c%c", 0x1b, '5'); // Charset 2
	Print("%c%c", 0x1c, '.'); // Disable Chinese Print Mode
	Print("%c%c", 0x1b, 0x54); // Upside-down print
	Print("%c%c%c", 0x1b, 0x57, 1); // Font size
}

void PrintSize(uint8_t width, uint8_t height) {
	Print("%c%c%c", 0x1b, 'U', width); // Font width
//	Print("%c%c%c", 0x1b, 'W', size); // Font size
	Print("%c%c%c", 0x1b, 'V', height); // Font height
}

//void Print(const char* data) {
//	HAL_UART_Transmit(&huart4, data, strlen(data), 1000);
//}

void Print(const char* format, ...) {

		  va_list args;
		  char* buf[80];
		  va_start(args, format);
		  vsprintf(buf, format, args);
		  //printf(buf);
		  va_end(args);

		  printf("%s\n",buf);
		  HAL_UART_Transmit(&huart4, buf, strlen(buf), 1000);
//		  HAL_UART_Transmit(&huart1, "\n", 1, 100);

}

void PrintHeader (const char* title, float t1, RTCDateTime_s* time) {
	Print("\n");
	Print("********************************\n");
	PrintSize(2, 2);
	Print("%s\n", title);
	PrintSize(1, 1);
	Print("\n");
	Print("%d-%02d-%02d\n", time->Year, time->Month, time->Day);
	Print("\n");
	Print("START %5.1f             %02d:%02d:%02d\n", t1, time->Hour, time->Minute, time->Second);
	Print("--------------------------------\n");
	Print("STEP    T1     T2   PRESS   TIME\n");
//	Print("        oC     oC    kPa   mm:ss\n");
	Print("--------------------------------\n");
}

void PrintFooter (const char* result, RTCDateTime_s* time, RTCTimerTime_s* timer) {
	Print("--------------------------------\n");
	Print("FINISH                  %02d:%02d:%02d\n", time->Hour, time->Minute, time->Second);
	Print("TOTAL TIME                 %2d:%02d\n", timer->TimeMinute, timer->TimeSecond);
	Print("\n");
	PrintSize(3, 2);
	Print("%s\n", result);
	PrintSize(1, 1);
	Print("\n");
	Print("\n");
	Print("OPERATOR: ______________________\n");
	Print("\n");
	Print("********************************\n");
	Print("\n");
	Print("\n");
	Print("\n");
	Print("\n");
}

void PrintStepLine(const char* step, float t1, float t2, float pressure, RTCTimerTime_s* time) {
	Print("%.4s  %5.1f  %5.1f  %5.1f %3d:%02d\n", step, t1, t2, pressure, time->TimeMinute, time->TimeSecond);
}

void PrintDivisor() {
	char* s = "********************************\n";
	Print(s);
}

void PrintLF(uint8_t lines) {
	char data[lines + 1];
	for (int i = 0; i < lines; i++) {
		data[i] = 0x1b;
	}
	data[lines] = 0x0;
	Print(data);
}
