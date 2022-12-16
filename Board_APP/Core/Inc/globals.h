/*==============================================================================

Filename: globals.h

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


#ifndef INC_GLOBALS_H_
#define INC_GLOBALS_H_

#include "main.h"
#include <stdio.h>

#define FW_VERSION_MAJOR 1
#define FW_VERSION_MINOR 0
#define FW_VERSION_REVISION 21

typedef struct _gpioPin {
	GPIO_TypeDef* GPIO_Port;
	uint16_t GPIO_Pin;
	uint8_t bit;
} gpioPin;

typedef enum {LED_OFF, LED_ON} LED_STATE;
typedef enum {BUZZER_OFF, BUZZER_ON} BUZZER_STATE;
typedef enum {FALSE, TRUE} LOGIC_STATE;

uint32_t GetSafeDeltaTick(uint32_t startTick, uint32_t endTick);
void SwapBytes(void *pBytes, size_t size);

// Bitwise operations ----------------------------------------------------------
#define BIT(n)					(1 << (x)) // Return the number representing the n_th bit
#define SETBIT(x, p)			((x) |= (1 << (p))) // Set p_th bit in x
#define SETBITS(x, n, p)		((x) |= (((0x00000001U << n) - 1) << p)) // Set n bits from position p in x
#define SETBITSMASK(x, m)		((x) |= (m)) // Set all bits in x masked by m; 0 leave bits unchanged
#define CLEARBIT(x, n)			((x) &= ~(0x00000001U << (n))) // Clear n_th bit in x
#define CLEARBITSMASK(x, m)		((x) &= ~(m)) // Clear all bits in x masked by m; 0 leave bits unchanged
#define TOGGLEBIT(x, n)			((x) ^= (0x00000001U << (n))) // Toggle n_th bit in x
#define TOGGLEBITSMASK(x, m)	((x) ^= (m)) // Toggle all bits in x masked by m; 0 leave bits unchanged
#define COPYBIT(x, y, p)		((x) = ((x) & ~(1 << p)) | ((y) & (1 << p))) // Copy p_th bit from y to x
#define COPYBITS(x, y, n, p)	((x) = ((x) & ~((((1 << n) - 1) << p))) | ((y) & (((0x00000001U << n) - 1) << p))) // Copy n bits from position p of y to same position of x
#define COPYBITSMASK(x, y, m)	((x) = ((x) & ~(m)) | ((y) & (m))) // Copy bits from y to x masked by m; 0 leave bits unchanged
#define CHECKBIT(x, n)			((x) & (1 << (n))) // Return 1 if n_th bit of x is 1, else 0

#define MAX_VALUE(a) (((unsigned long long)1 << (sizeof(a) * 8)) - 1) // maximum value for the variable type 'a'- MAX_VALUE(char) = 255.

#endif /* INC_GLOBALS_H_ */
