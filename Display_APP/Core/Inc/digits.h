/*
 * display.h
 *
 *  Created on: Nov 26, 2022
 *      Author: josh
 */

#ifndef INC_DIGITS_H_
#define INC_DIGITS_H_

#include <stdio.h>

void DisplayDigitsInit();
void DisplayDigitsRefresh();
void DisplayDigitsWriteRow(uint8_t row, char* digits);
void DisplayDigitsClearAll();
void DisplayDigitsSetAll();

#endif /* INC_DIGITS_H_ */
