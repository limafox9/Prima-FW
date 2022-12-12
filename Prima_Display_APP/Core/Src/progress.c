/*
 * progress.c
 *
 *  Created on: Dec 5, 2022
 *      Author: josh
 */

#include "progress.h"


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

