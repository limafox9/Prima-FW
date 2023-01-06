/*==============================================================================

Filename: class_machine.h

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


#ifndef INC_CLASS_MACHINE_H_
#define INC_CLASS_MACHINE_H_

#include "main.h"
#include "globals.h"
#include "cmsis_os.h"

class machine_c {

private:
	const uint32_t uid[3] = {*(uint32_t*)(UID_BASE), *(uint32_t*)(UID_BASE + 0x04), *(uint32_t*)(UID_BASE + 0x08)}; // STM32 MCU Unique ID 96bit
	const uint8_t fwVersion[3] = {FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION};

	const gpioPin_s beepPin = {BEEP_GPIO_Port, BEEP_Pin};

public:
	void Beep(u8 hits, u16 onDelay, u16 offDelay);
	const uint32_t* GetUID();
	const uint8_t* GetFirmwareVersion();
	void Init();
};

#endif /* INC_CLASS_MACHINE_H_ */
