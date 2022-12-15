/*
 * progress.c
 *
 *  Created on: Dec 5, 2022
 *      Author: josh
 */

#include <stdio.h>
#include "progress.h"

extern uint16_t progressBarBits;


void ProgressSetAll() {
	memset(&progressBarBits, 0xFF, 11);
	ProgressRefresh();
}

void ProgressClearAll() {
	memset(&progressBarBits, 0x00, 11);
	ProgressRefresh();
}

void ProgressInit()
{
	uint16_t bitsCount;

	// Release latch
	HAL_GPIO_WritePin(PROGRESS_STROBE_GPIO_Port, PROGRESS_STROBE_Pin, GPIO_PIN_RESET);

	// Pushing segment data all 0
	HAL_GPIO_WritePin(PROGRESS_DATA_GPIO_Port, PROGRESS_DATA_Pin, GPIO_PIN_RESET);

	for(bitsCount = 0; bitsCount < 16; bitsCount++) {
		// Clock pulse while data is all 0
		HAL_GPIO_WritePin(PROGRESS_CLOCK_GPIO_Port, PROGRESS_CLOCK_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PROGRESS_CLOCK_GPIO_Port, PROGRESS_CLOCK_Pin, GPIO_PIN_SET);
	}

	// Set latch
	HAL_GPIO_WritePin(PROGRESS_STROBE_GPIO_Port, PROGRESS_STROBE_Pin, GPIO_PIN_SET);

}

void ProgressRefresh()
{

	uint16_t bitsCount;
	uint32_t k;

	// Release latch
	HAL_GPIO_WritePin(PROGRESS_STROBE_GPIO_Port, PROGRESS_STROBE_Pin, GPIO_PIN_RESET);
		// Progress bar leds are the firs 11 bits of progressBarBits
		// Pushing segment serial data
		for (bitsCount = 0; bitsCount < 16; bitsCount++) {
			// Bit check
			k = 0x8000 >> bitsCount;
			if(progressBarBits & k)
			{
				HAL_GPIO_WritePin(PROGRESS_DATA_GPIO_Port, PROGRESS_DATA_Pin, GPIO_PIN_SET);
			} else {
				HAL_GPIO_WritePin(PROGRESS_DATA_GPIO_Port, PROGRESS_DATA_Pin, GPIO_PIN_RESET);
			}
			// Clock pulse
			HAL_GPIO_WritePin(PROGRESS_CLOCK_GPIO_Port, PROGRESS_CLOCK_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(PROGRESS_CLOCK_GPIO_Port, PROGRESS_CLOCK_Pin, GPIO_PIN_SET);
		}

		// Set latch
		HAL_GPIO_WritePin(PROGRESS_STROBE_GPIO_Port, PROGRESS_STROBE_Pin, GPIO_PIN_SET);

}

