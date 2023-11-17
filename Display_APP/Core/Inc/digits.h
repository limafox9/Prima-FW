/*
 * display.h
 *
 *  Created on: Nov 26, 2022
 *      Author: josh
 */

#ifndef INC_DIGITS_H_
#define INC_DIGITS_H_

#include <stdio.h>
#include <stdint.h>

#define DIGITS_ROWS 3
#define DIGITS_COLUMNS 4
#define DIGITS_ROW_BUFFER_SIZE 	8
#define DIGITS_REFRESH_INTERVAL 2 // milliseconds

void DigitsInit();
void DigitsRefreshTask();
void DigitsWrite(uint8_t row, char* digits);
void DigitsClearAll();
void DigitsSetAll();

#endif /* INC_DIGITS_H_ */
