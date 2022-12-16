/*==============================================================================

Filename: leds.c

Created on: Dec 16, 2022

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

#include "leds.h"
#include "main.h"
#include "globals.h"
#include "panel_serial.h"

extern uint16_t panelLedsBits;

void StartLedsTask() {

	panelLedsBits = 0;
	uint16_t lastLeds = 0, currLeds = 0;

	for (;;) {
		currLeds = 0;

		//currLeds = HAL_GPIO_ReadPin(SW_DOOR_GPIO_Port, SW_DOOR_Pin);
		currLeds = HAL_GPIO_ReadPin(SW_DOOR_GPIO_Port, SW_DOOR_Pin) ? currLeds & ~1 : currLeds | 1;
		currLeds = HAL_GPIO_ReadPin(SW_LSH_GPIO_Port, SW_LSH_Pin) ? currLeds & ~(1UL << 1): currLeds | (1UL << 1);

		if  (lastLeds != currLeds) {
			panelLedsBits = currLeds;
			PanelSerialTransmit(COMMAND_SET_DASHBOARD_LEDS, panelLedsBits, 0, 0, 0);
			lastLeds = panelLedsBits;
		}

		osDelay(10);
	}

}


