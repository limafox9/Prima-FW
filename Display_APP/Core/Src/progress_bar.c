/*
 * progress.c
 *
 *  Created on: Dec 5, 2022
 *      Author: josh
 */

#include "progress_bar.h"
#include <stdio.h>
#include "globals.h"
#include <stdlib.h>
#include <string.h>

uint16_t progressBarBits;


void ProgressBarWrite(uint16_t leds) {
	memcpy(&progressBarBits, &leds, 11);
	memcpy(&progressBarBits + 15, &leds, 1);
	ProgressBarRefresh();
}

void ProgressBarSetAll() {
	// Set first 11 bits (progress bar)
	memset(&progressBarBits, 0xFF, 11);
	ProgressBarRefresh();
}

void ProgressBarClearAll() {
	// Clear first 11 bits (progress bar)
	memset(&progressBarBits, 0x00, 11);
	ProgressBarRefresh();
}

void ProgressBarInit() {
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

void ProgressBarRefresh() {

	uint16_t bitsCount;
	uint16_t k;

	// Release latch
	HAL_GPIO_WritePin(PROGRESS_STROBE_GPIO_Port, PROGRESS_STROBE_Pin, GPIO_PIN_RESET);

	// Progress bar leds are the firs 11 bits of progressBarBits
	// Pushing segment serial data

	// bit 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1
	//     N N L L L P P P P P P P P P P P
	//     C C E E E 1 1 9 8 7 6 5 4 3 2 1
	//         D D D 1 0

	// Sets the 3 leds in the dashboard connected to the same ser/par chip
	for (bitsCount = 0; bitsCount < 5; bitsCount++) {
		// Bit check
		k = 0x8000 >> bitsCount;
		if(progressBarBits & k)	{
			HAL_GPIO_WritePin(PROGRESS_DATA_GPIO_Port, PROGRESS_DATA_Pin, GPIO_PIN_SET);
		} else {
			HAL_GPIO_WritePin(PROGRESS_DATA_GPIO_Port, PROGRESS_DATA_Pin, GPIO_PIN_RESET);
		}
		// Clock pulse
		HAL_GPIO_WritePin(PROGRESS_CLOCK_GPIO_Port, PROGRESS_CLOCK_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PROGRESS_CLOCK_GPIO_Port, PROGRESS_CLOCK_Pin, GPIO_PIN_SET);
	}

	// Inverts first 11 bits of progress leds so that the first is on the left
	// 1 = leftmost led in progress bar
	for (bitsCount = 0; bitsCount < 11; bitsCount++) {
		// Bit check
		k = 1 << bitsCount;
		if(progressBarBits & k) {
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

