/*
 * Buzzer.c
 *
 *  Created on: 26 nov 2022
 *      Author: josh
 */

#include "buzzer.h"

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
