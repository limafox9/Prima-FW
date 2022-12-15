/*
 * progress.h
 *
 *  Created on: Dec 5, 2022
 *      Author: josh
 */

#ifndef INC_PROGRESS_H_
#define INC_PROGRESS_H_

#include "main.h"
#include "globals.h"

void ProgressWriteLeds(uint16_t leds);
void ProgressRefresh();
void ProgressSetAll();
void ProgressClearAll();

#endif /* INC_PROGRESS_H_ */
