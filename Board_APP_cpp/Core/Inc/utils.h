/*==============================================================================

Filename: utils.h

Created on: Jan 6, 2023

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



#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include "main.h"
#include "globals.h"

typedef enum {LOG_NONE = 0, LOG_DATE = 1, LOG_TIME = 2, LOG_ELAPSED = 4, LOG_ELAPSED_RESET = 8} LOG_TIMESTAMP;

void Log(LOG_LEVEL logLevel, uint8_t showDateTime, const char* format, ...);
uint32_t GetSafeDeltaTick(uint32_t startTick, uint32_t endTick);

bool PinRead(gpioPin_s pin);
void PinWrite(gpioPin_s pin, bool status);
void PinToggle(gpioPin_s pin, uint8_t status);

#define CHECKBIT(x, n)			((x) & (1 << (n))) // Return 1 if n_th bit of x is 1, else 0

#define MAX_VALUE(a) (((unsigned long long)1 << (sizeof(a) * 8)) - 1) // maximum value for the variable type 'a'- MAX_VALUE(char) = 255.

#endif /* INC_UTILS_H_ */
