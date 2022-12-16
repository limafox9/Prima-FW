/*
 * Buzzer.c
 *
 *  Created on: 26 nov 2022
 *      Author: josh
 */

#include "buzzer.h"

gpioPin beepPin = {BEEP_GPIO_Port, BEEP_Pin};


void Beep(uint8_t hits, uint16_t onDelay, uint16_t offDelay) {

	for (uint8_t i = 0; i < hits; i++) {
		HAL_GPIO_WritePin(beepPin.GPIO_Port, beepPin.GPIO_Pin, BUZZER_ON);
		HAL_Delay(onDelay);
		HAL_GPIO_WritePin(beepPin.GPIO_Port, beepPin.GPIO_Pin, BUZZER_OFF);
		HAL_Delay(onDelay);
	}
}
