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

uint32_t _UID[3];

uint8_t digitsRow1[8];
uint8_t digitsRow2[8];
uint8_t digitsRow3[8];

uint16_t progressBarBits;
uint16_t progressBarBitsBlink;

uint16_t panelLedsBits;
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
    for(lo = 0, hi = size - 1; hi > lo; lo++, hi--)
    {
        char tmp = p[lo];
        p[lo] = p[hi];
        p[hi] = tmp;
    }
}


