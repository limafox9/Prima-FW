/*
 * leds.c
 *
 *  Created on: Dec 4, 2022
 *      Author: josh
 */

#include "leds.h"

const GPIO_PIN panelBits[] = {
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

void SetLed(const GPIO_PIN* led, LED_STATE status);
void PanelRefresh();

//---------------------------------------------------------------------------

void PanelSetAll() {
	//panelLedsBits = 0b00000000000000;
	panelLedsBits = 0xFFFF;
	PanelRefresh();
}

void PanelClearAll() {
	//panelLedsBits = 0b00000000000000;
	panelLedsBits = 0;
	PanelRefresh();
}

void PanelRefresh() {

	//int k = 0x8000;

//	SetLed(&ledModeWrapped, panelLedsBits && (0x8000 >> ledModeWrapped.bit));

	for (int i = 0; i < 11; i++) {
//		SetLed(&a[i], ((panelLedsBits & (1 << a[i].bit)) > 0) ? LED_ON : LED_OFF);
		SetLed(&panelBits[i], CHECKBIT(panelLedsBits, panelBits[i].bit) > 0 ? LED_ON : LED_OFF);
	}

	for (int i = 11; i < 14; i++) {
		COPYBIT(progressBarBits, panelLedsBits, i);
	}

	ProgressRefresh();
}

void SetLed(const GPIO_PIN* led, LED_STATE status) {
	if (status == LED_OFF) {
		HAL_GPIO_WritePin(led->GPIO_Port, led->GPIO_Pin, LED_OFF);
//	} else if (status == LED_ON) {
//		HAL_GPIO_WritePin(led->GPIO_Port, led->GPIO_Pin, LED_ON);
	} else {
		HAL_GPIO_WritePin(led->GPIO_Port, led->GPIO_Pin, LED_ON);
	}
//	HAL_GPIO_WritePin(led->GPIO_Port, led->GPIO_Pin, status);
}
