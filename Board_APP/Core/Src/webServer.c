/*==============================================================================

Filename: webServer.c

Created on: May 9, 2024

Original work: Copyright (c) 2024 Luca Favenzi
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

#include "main.h"
#include "cmsis_os.h"
#include "globals.h"
#include "actuators.h"
#include "W5500.h"
#include "webServer.h"
#include "socket.h"
#include "sensors.h"

#include <stdio.h>
#include <string.h>

#define HTTP_SOCKET	0

extern Sensors_s sensors;

void webInit() {

	HAL_GPIO_WritePin(ETH_PMODE0_GPIO_Port, ETH_PMODE0_Pin, ON);
	HAL_GPIO_WritePin(ETH_PMODE1_GPIO_Port, ETH_PMODE1_Pin, ON);
	HAL_GPIO_WritePin(ETH_PMODE2_GPIO_Port, ETH_PMODE2_Pin, ON);
	HAL_GPIO_WritePin(ETH_nRST_GPIO_Port, ETH_nRST_Pin, ON);
	W5500_Init();

}

void StartWebServerTask() {

	uint8_t stat1;
	char Message[128];

	for (;;) {

		stat1 = socket(HTTP_SOCKET, Sn_MR_TCP, 80, 0);
		stat1 = listen(HTTP_SOCKET);

		while(getSn_SR(HTTP_SOCKET) == SOCK_LISTEN) {
			osDelay(200);
		}

		sprintf(Message, "T1: %.1f\n\r", sensors.T1);
		send(0, (uint8_t*)Message, strlen(Message));
		sprintf(Message, "T2: %.1f\n\r", sensors.T2);
		send(0, (uint8_t*)Message, strlen(Message));
		sprintf(Message, "T3: %.1f\n\r", sensors.TemperatureChamberHeater);
		send(0, (uint8_t*)Message, strlen(Message));
		sprintf(Message, "T4: %.1f\n\r", sensors.TemperatureSteamGenerator);
		send(0, (uint8_t*)Message, strlen(Message));
		sprintf(Message, "P1: %.1f\n\r", sensors.PressureChamber);
		send(0, (uint8_t*)Message, strlen(Message));
		sprintf(Message, "PA: %.1f\n\r", sensors.PressureAmbient);
		send(0, (uint8_t*)Message, strlen(Message));

		//		Beep(2, 100, 100);

		disconnect(HTTP_SOCKET);
		close(HTTP_SOCKET);

	}

}
