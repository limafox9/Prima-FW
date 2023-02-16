/*
 * Buzzer.c
 *
 *  Created on: 26 nov 2022
 *      Author: josh
 */

#include "buzzer.h"
#include "cmsis_os.h"


void Beep(uint8_t hits, uint16_t onDelay, uint16_t offDelay) {

	uint32_t tick;

	for (uint8_t i = 0; i < hits; i++) {
		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, ON);
		HAL_Delay(onDelay);
		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, OFF);
		HAL_Delay(offDelay);
	}
}
