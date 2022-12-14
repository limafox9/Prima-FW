/*
 * buzzer.h
 *
 *  Created on: Nov 26, 2022
 *      Author: josh
 */

#ifndef INC_BUZZER_TASK_H_
#define INC_BUZZER_TASK_H_

#include "globals.h"
#include "main.h"

extern void Beep(uint8_t hits, uint16_t onDelay, uint16_t offDelay);
extern void BeepTask();

#endif /* INC_BUZZER_TASK_H_ */
