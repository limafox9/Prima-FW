/*
 * progress.h
 *
 *  Created on: Dec 5, 2022
 *      Author: josh
 */

#ifndef INC_PROGRESS_BAR_H_
#define INC_PROGRESS_BAR_H_

#include "main.h"
#include <stdio.h>
#include <stdlib.h>

void ProgressBarWrite(uint16_t leds);
void ProgressBarRefresh();
void ProgressBarSetAll();
void ProgressBarClearAll();
void ProgressBarInit();

#endif /* INC_PROGRESS_BAR_H_ */
