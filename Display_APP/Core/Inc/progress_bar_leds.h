/*
 * progress.h
 *
 *  Created on: Dec 5, 2022
 *      Author: josh
 */

#ifndef INC_PROGRESS_BAR_LEDS_H_
#define INC_PROGRESS_BAR_LEDS_H_

#include "main.h"
#include "globals.h"

void DisplayProgressWriteLeds(uint16_t leds);
void DisplayProgressRefresh();
void DisplayProgressSetAll();
void DisplayProgressClearAll();

#endif /* INC_PROGRESS_BAR_LEDS_H_ */
