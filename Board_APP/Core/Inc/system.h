/*==============================================================================

Filename: system.h

Created on: Dec 17, 2022

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

================================================================================
PURPOSE:
USAGE:
==============================================================================*/


#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

#include "globals.h"
#include "PCF8563.h"
#include "cmsis_os.h"

#define MCU_UID_ADDRESS 0x1FFFF7E8

typedef enum {
	LOG_LEVEL_ALL = 0,
	LOG_LEVEL_DEBUG = 1,
	LOG_LEVEL_RELEASE = 2,
	LOG_LEVEL_OFF = 99
} e_LogLevel;

typedef enum {
	LOG_TIME_NONE = 0,
	LOG_TIME_DATE = 1,
	LOG_TIME_TIME = 2,
	LOG_TIME_ELAPSED = 4,
	LOG_TIME_ELAPSED_RESET = 8
} e_LogTimeStamp;

typedef enum {
	LOG_OUT_NONE = 0,
	LOG_OUT_SERIAL = 1,
	LOG_OUT_FILE = 2,
	LOG_OUT_PRINTER = 4
} e_LogOutput;

extern RTCDateTime_s RTCDateTime;


extern osThreadId keysTaskHandle;
extern osThreadId serialTaskHandle;
extern osThreadId sensorsTaskHandle;
extern osThreadId programTaskHandle;
extern osThreadId displayInfoTaskHandle;

extern osThreadId usbTaskHandle;

extern volatile uint8_t usb_mounted;
extern volatile uint8_t usb_recovering;

extern osSemaphoreId semaphoreSensorsHandle;
extern osSemaphoreId semaphoreSerialHandle;
extern QueueHandle_t usbQueue;
extern QueueHandle_t serialQueue;

void SysInit();
//void Beep(uint8_t hits, uint16_t onDelay, uint16_t offDelay);
void Log(e_LogLevel logLevel, e_LogTimeStamp logTimeStamp, e_LogOutput logOutput, const char* format, ...);
void EmergencyStop();

#endif /* INC_SYSTEM_H_ */
