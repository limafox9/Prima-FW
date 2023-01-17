/*
 * leds.h
 *
 *  Created on: Dec 4, 2022
 *      Author: josh
 */

#ifndef INC_DASHBOARD_H_
#define INC_DASHBOARD_H_

#include <stdio.h>

void DashboardWrite(uint16_t leds);
void DashboardSetAll();
void DashboardClearAll();
void DashboardRefresh();

#endif /* INC_DASHBOARD_H_ */
