/*
 * leds.h
 *
 *  Created on: Dec 4, 2022
 *      Author: josh
 */

#ifndef INC_DASHBOARD_LEDS_H_
#define INC_DASHBOARD_LEDS_H_

#include <progress_bar_leds.h>
#include "main.h"
#include "globals.h"

void DisplayDashboardWriteLeds(uint16_t leds);
void DisplayDashboardSetAll();
void DisplayDashboardClearAll();
void DisplayDashboardRefresh();

#endif /* INC_DASHBOARD_LEDS_H_ */
