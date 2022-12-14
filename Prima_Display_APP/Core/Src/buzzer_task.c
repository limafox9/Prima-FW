/*
 * Buzzer.c
 *
 *  Created on: 26 nov 2022
 *      Author: josh
 */

#include "buzzer_task.h"

GPIO_PIN beepPin = {BEEP_GPIO_Port, BEEP_Pin};



void Beep(uint8_t hits, uint16_t onDelay, uint16_t offDelay) {

	uint8_t i;

	for (i = 0; i < hits; i++) {
		HAL_GPIO_WritePin(beepPin.GPIO_Port, beepPin.GPIO_Pin, GPIO_PIN_SET);
		HAL_Delay(onDelay);
		HAL_GPIO_WritePin(beepPin.GPIO_Port, beepPin.GPIO_Pin, GPIO_PIN_RESET);
//		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
		HAL_Delay(onDelay);
	}
}

void BeepTask() {

	osEvent evt;

	uint64_t *c = 0;
	uint8_t hits = 0;
	uint16_t onDelay = 0;
	uint16_t offDelay = 0;

	for (;;) {

			evt = osMessageGet(qBeepHandle, osWaitForever);
			if (evt.status == osEventMessage) {
				c = evt.value.p;
				memcpy(&hits, c, 1);
				memcpy(&onDelay, c+1, 2);
				memcpy(&offDelay, c+3, 2);

				for (uint8_t i = 0; i < hits; i++) {
					HAL_GPIO_WritePin(beepPin.GPIO_Port, beepPin.GPIO_Pin, GPIO_PIN_SET);
					HAL_Delay(onDelay);
					HAL_GPIO_WritePin(beepPin.GPIO_Port, beepPin.GPIO_Pin, GPIO_PIN_RESET);
			//		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
					HAL_Delay(onDelay);
				}
			 // free memory allocated for message
			    }


		osDelay(100);
	}
}
