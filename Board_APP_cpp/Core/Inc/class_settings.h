/*==============================================================================

Filename: settings.h

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



#ifndef INC_CLASS_SETTINGS_H_
#define INC_CLASS_SETTINGS_H_

#include "main.h"

class settings_c {
public:
	struct FirmwareVersion {
		uint8_t Major = 2;
		uint8_t Minor = 3;
		uint8_t Revision = 14;
	};

	typedef enum {LOG_LEVEL_DEBUG = 1, LOG_LEVEL_RELEASE = 2} LOG_LEVEL;
	bool LogActive = true;
	LOG_LEVEL LogLevel = LOG_LEVEL_DEBUG;

	uint16_t HeartLed_ms = 200;
	uint16_t SolenoidImpulseDuration_ms = 1000;
};

#endif /* INC_CLASS_SETTINGS_H_ */
