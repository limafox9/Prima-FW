/*
 * leds.c
 *
 *  Created on: Dec 4, 2022
 *      Author: josh
 */

#include <dashboard_leds.h>
#include "globals.h"

extern uint16_t progressBarBits;
extern uint16_t panelLedsBits;

extern uint16_t DB_ProgressBarBits;
extern uint16_t DB_PanelLedsBits;

const s_GPIO panelBits[] = {
	{LED_MODE_UNWRAPPED_GPIO_Port, LED_MODE_UNWRAPPED_Pin, 0},
	{LED_MODE_WRAPPED_GPIO_Port, LED_MODE_WRAPPED_Pin, 1},
	{LED_MODE_PRION_GPIO_Port, LED_MODE_PRION_Pin, 2},
	{LED_MODE_GENTLE_GPIO_Port, LED_MODE_GENTLE_Pin, 3},
	{LED_TEST_HELIX_GPIO_Port, LED_TEST_HELIX_Pin, 4},
	{LED_TEST_VACUUM_GPIO_Port, LED_TEST_VACUUM_Pin, 5},
	{LED_WARNING_CTL_GPIO_Port, LED_WARNING_CTL_Pin, 6},
	{LED_WARNING_LSH_GPIO_Port, LED_WARNING_LSH_Pin, 7},
	{LED_WARNING_TDS_GPIO_Port, LED_WARNING_TDS_Pin, 8},
	{LED_TEMP_121_GPIO_Port, LED_TEMP_121_Pin, 9},
	{LED_TEMP_134_GPIO_Port, LED_TEMP_134_Pin, 10}
};

void SetLed(const s_GPIO* led, POWER_STATE status);
void DisplayDashboardRefresh();

//---------------------------------------------------------------------------

void DisplayDashboardWriteLeds(uint16_t leds) {
	COPYBITS(DB_PanelLedsBits, leds, 14, 0);
//	memcpy(&panelLedsBits, leds, 11);
	DisplayDashboardRefresh();
}

void DisplayDashboardSetAll() {
	//panelLedsBits = 0b00000000000000;
	DB_PanelLedsBits = 0xFFFF;
	DisplayDashboardRefresh();
}

void DisplayDashboardClearAll() {
	//panelLedsBits = 0b00000000000000;
	DB_PanelLedsBits = 0;
	DisplayDashboardRefresh();
}

void DisplayDashboardRefresh() {

	//int k = 0x8000;

//	SetLed(&ledModeWrapped, panelLedsBits && (0x8000 >> ledModeWrapped.bit));

	for (int i = 0; i < 11; i++) {
//		SetLed(&a[i], ((panelLedsBits & (1 << a[i].bit)) > 0) ? LED_ON : LED_OFF);
		SetLed(&panelBits[i], CHECKBIT(DB_PanelLedsBits, panelBits[i].bit) > 0 ? ON : OFF);
	}

	for (int i = 11; i < 14; i++) {
		COPYBIT(progressBarBits, DB_PanelLedsBits, i);
	}

	DisplayProgressRefresh();
}

void SetLed(const s_GPIO* led, POWER_STATE status) {
	if (status == OFF) {
		HAL_GPIO_WritePin(led->GPIO_Port, led->GPIO_Pin, OFF);
//	} else if (status == LED_ON) {
//		HAL_GPIO_WritePin(led->GPIO_Port, led->GPIO_Pin, LED_ON);
	} else {
		HAL_GPIO_WritePin(led->GPIO_Port, led->GPIO_Pin, ON);
	}
//	HAL_GPIO_WritePin(led->GPIO_Port, led->GPIO_Pin, status);
}
