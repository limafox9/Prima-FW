/*
 * display.h
 *
 *  Created on: Nov 26, 2022
 *      Author: josh
 */

#ifndef INC_DIGITS_H_
#define INC_DIGITS_H_

#define DISPLAY_REFRESH_MILLIS 2


void DigitsInit();
void DigitsRefresh();
void DigitsWriteRow1(char* digits);
void DigitsWriteRow2(char* digits);
void DigitsWriteRow3(const char* digits, ...);
void DigitsClearAll();
void DigitsSetAll();

#endif /* INC_DIGITS_H_ */
