/*==============================================================================

Filename: programs.h

Created on: 28 gen 2023

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



#ifndef INC_PROGRAMS_H_
#define INC_PROGRAMS_H_

#include <stdio.h>

#define PROGRAMS_N 5
#define TESTS_N 4

typedef enum {
	PROGRAM_UNWRAPPED,
	PROGRAM_WRAPPED,
	PROGRAM_PRION,
	PROGRAM_121,
	PROGRAM_TEST_HELIX,
	PROGRAM_TEST_VACUUM,
	PROGRAM_WASH,
	PROGRAM_WATER_LOAD
} Program_e;

void SetProgram();
void SetTest();

uint8_t ProgramWrapped();
uint8_t ProgramHelixTest();

#endif /* INC_PROGRAMS_H_ */
