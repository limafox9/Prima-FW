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
//		while(getSn_SR(HTTP_SOCKET) == SOCK_ESTABLISHED) {
		sprintf(Message, "<!DOCTYPE html><html>");
		send(0, (uint8_t*)Message, strlen(Message));
//		sprintf(Message, "<head><meta http-equiv=\"refresh\" content=\"2\"></head>");
//		send(0, (uint8_t*)Message, strlen(Message));
		sprintf(Message, "<script type=\"text/javascript\">function load(){setTimeout(\"window.open(self.location, '_self');\", 5000);}</script>");
		send(0, (uint8_t*)Message, strlen(Message));
		sprintf(Message, "<body onload=\"load()\" style=\"background-color:black; font-family:arial; color:white; margin-left:50px\">");
		send(0, (uint8_t*)Message, strlen(Message));
		sprintf(Message, "<br><img src=\"https://reserved.galbiatidentalgroup.com/wp-content/uploads/galbiati-logo-full-white_120.png\">");
		send(0, (uint8_t*)Message, strlen(Message));

		sprintf(Message, "<h1><br>GALBIATI PRIMA");
		send(0, (uint8_t*)Message, strlen(Message));
		sprintf(Message, "<h2 style=\"color:LightSkyBlue\">S/N: %s<h1><br>", machine.Serial);
		send(0, (uint8_t*)Message, strlen(Message));

		sprintf(Message, "T1: %.1f &deg;C<br>", sensors.T1);
		send(0, (uint8_t*)Message, strlen(Message));
		sprintf(Message, "T2: %.1f &deg;C<br><br>", sensors.T2);
		send(0, (uint8_t*)Message, strlen(Message));
		sprintf(Message, "T3: %.1f &deg;C<br>", sensors.TemperatureChamberHeater);
		send(0, (uint8_t*)Message, strlen(Message));
		sprintf(Message, "T4: %.1f &deg;C<br><br>", sensors.TemperatureSteamGenerator);
		send(0, (uint8_t*)Message, strlen(Message));
		sprintf(Message, "P1: %.1f kPa<br><br>", sensors.PressureChamber);
		send(0, (uint8_t*)Message, strlen(Message));
		sprintf(Message, "PA: %.1f kPa<br>", sensors.PressureAmbient);
		send(0, (uint8_t*)Message, strlen(Message));

		sprintf(Message, "</body></html>");
		send(0, (uint8_t*)Message, strlen(Message));
		//		Beep(2, 100, 100);
//osDelay(1000);
//		}
		disconnect(HTTP_SOCKET);
		close(HTTP_SOCKET);

	}

}
