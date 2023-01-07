/*==============================================================================

Filename: class_machine.cpp

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


#include "class_machine.h"


extern I2C_HandleTypeDef hi2c1;

void machine_c::Init() {
	RTClock.Init(&hi2c1);
}

const uint32_t* machine_c::GetUID() {
	return uid;
}

const uint8_t* machine_c::GetFirmwareVersion() {
	return fwVersion;
}

void machine_c::Beep(uint8_t hits, uint16_t onDelay, uint16_t offDelay) {

	for (uint8_t i = 0; i < hits; i++) {
		PinWrite(beepPin, ON);
		osDelay(onDelay);
		PinWrite(beepPin, OFF);
		osDelay(offDelay);
	}
}

