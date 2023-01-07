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


typedef struct GPIOPin_s {
	GPIO_TypeDef* Port;
	uint16_t Pin;
} GPIOPin_s;

typedef enum {FALSE, TRUE} LOGIC_STATE;
typedef enum {OFF, ON} POWER_STATE;
typedef enum {UNLOCK, LOCK} LOCK_STATE;
typedef enum {LOW, HIGH} LEVEL_STATE;

typedef enum {LOG_NONE = 0, LOG_DATE = 1, LOG_TIME = 2, LOG_ELAPSED = 4, LOG_ELAPSED_RESET = 8} LOG_TIMESTAMP;

bool PinRead(GPIOPin_s pin);
void PinWrite(GPIOPin_s pin, bool status);
void PinToggle(GPIOPin_s pin, uint8_t status);

void Log(LOG_LEVEL logLevel, uint8_t showDateTime, const char* format, ...);


// MACROS ======================================================================

#define MAX_VALUE(a) (((unsigned long long)1 << (sizeof(a) * 8)) - 1) // maximum value for the variable type 'a'- MAX_VALUE(char) = 255.

// BITWISE MACROS ==============================================================

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


#endif /* INC_GLOBALS_H_ */
