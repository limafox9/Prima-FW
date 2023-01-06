/*==============================================================================

Filename: globals.h

Created on: Jan 5, 2023

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



#ifndef INC_GLOBALS_H_
#define INC_GLOBALS_H_

#include <stdint.h>
#include "main.h"

#define FW_VERSION_MAJOR 	2
#define FW_VERSION_MINOR 	3
#define FW_VERSION_REVISION 14

typedef enum {LOG_LEVEL_DEBUG = 1, LOG_LEVEL_RELEASE = 2} LOG_LEVEL;
#define LOG_ACTIVE			true
#define LOG_LEVEL_ACTIVE	LOG_LEVEL_DEBUG

#define HEART_LED_BLINK_DURATION			200		// Heart LED blink toggle duration (milliseconds)
#define DEFAULT_SOLENOID_IMPULSE_DURATION 	1000

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef struct gpioPin_s {
	GPIO_TypeDef* Port;
	uint16_t Pin;
} gpioPin_s;

typedef enum {FALSE, TRUE} LOGIC_STATE;
typedef enum {OFF, ON} POWER_STATE;
typedef enum {UNLOCK, LOCK} LOCK_STATE;
typedef enum {LOW, HIGH} LEVEL_STATE;


#endif /* INC_GLOBALS_H_ */
