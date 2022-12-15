/*
 * display.c
 *
 *  Created on: 26 nov 2022
 *      Author: josh
 */

#include <stdio.h>
#include "globals.h"
#include "cmsis_os.h"
#include "main.h"
#include "digits.h"

extern uint8_t digitsRow1[4];
extern uint8_t digitsRow2[4];
extern uint8_t digitsRow3[4];

// Character to segments conversion - @ for all segments
// Segments:	dp g f e d c b a
// Byte for E:	 0 1 1 1 1 0 0 1
const uint8_t displayChar[] = {
//      NULL  SOH   STX   ETX   EOT   ENQ   ACK   BEL    BS   TAB    LF    VT    FF    CR    SO    SI
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//      DLE   DC1   DC2   DC3   DC4   NAK   SYN   ETB   CAN    EM   SUB   ESC    FS    GS    RS    US
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//      SPACE   !     "     #     $     %     &     '     (     )     *     +     ,     -     .     /
		0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x80, 0x52,
//        0     1     2     3     4     5     6     7     8     9     :     ;     <     =     >     ?
		0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00,
//        @     A     B     C     D     E     F     G     H     I     J     K     L     M     N     O
		0xFF, 0x77, 0x00, 0x39, 0x00, 0x79, 0x71, 0x3D, 0x76, 0x30, 0x1E, 0x00, 0x38, 0x00, 0x37, 0x3F,
//        P     Q     R     S     T     U     V     W     X     Y     Z     [     \     ]     ^     _
		0x73, 0x00, 0x00, 0x6D, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x39, 0x64, 0x0F, 0x23, 0x08,
//        `     a     b     c     d     e     f     g     h     i     j     k     l     m     n     o
		0x02, 0x5F, 0x7C, 0x58, 0x5E, 0x00, 0x00, 0x6F, 0x74, 0x10, 0x00, 0x00, 0x06, 0x00, 0x54, 0x5C,
//        p     q     r     s     t     u     v     w     x     y     z     {     |     }     ~    DEL
		0x00, 0x67, 0x50, 0x00, 0x78, 0x1C, 0x00, 0x00, 0x00, 0x6E, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00
};

void DigitsInit()
{

	uint8_t columnDataBit, columnIndex;

	// All display off

	// Release latch
	HAL_GPIO_WritePin(DISPLAY_STROBE_GPIO_Port, DISPLAY_STROBE_Pin, GPIO_PIN_RESET);

	// All columns OFF while pushing serial data
	HAL_GPIO_WritePin(DISPLAY_DIGIT_1_GPIO_Port, DISPLAY_DIGIT_1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DISPLAY_DIGIT_2_GPIO_Port, DISPLAY_DIGIT_2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DISPLAY_DIGIT_3_GPIO_Port, DISPLAY_DIGIT_3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DISPLAY_DIGIT_4_GPIO_Port, DISPLAY_DIGIT_4_Pin, GPIO_PIN_RESET);

	for (columnIndex =0; columnIndex < 4; columnIndex++) {

		// Pushing segment data all 0
		HAL_GPIO_WritePin(DISPLAY_DATA_GPIO_Port, DISPLAY_DATA_Pin, GPIO_PIN_RESET);

		for(columnDataBit = 0; columnDataBit < 24; columnDataBit++) {
			// Clock pulse while data is all 0
			HAL_GPIO_WritePin(DISPLAY_CLOCK_GPIO_Port, DISPLAY_CLOCK_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(DISPLAY_CLOCK_GPIO_Port, DISPLAY_CLOCK_Pin, GPIO_PIN_SET);
		}
	}

	// Set latch
	HAL_GPIO_WritePin(DISPLAY_STROBE_GPIO_Port, DISPLAY_STROBE_Pin, GPIO_PIN_SET);

}

void DigitsWriteRow1(char* digits) {
	memcpy(digitsRow1, digits, 4);
}

void DigitsWriteRow2(char* digits) {
	memcpy(digitsRow2, digits, 4);
}

void DigitsWriteRow3(char* digits) {
	memcpy(digitsRow3, digits, 4);
}

void DigitsRefresh() {

	uint8_t columnIndex;
	uint32_t k, columnDataBit, columnData;

	// Release latch
	HAL_GPIO_WritePin(DISPLAY_STROBE_GPIO_Port, DISPLAY_STROBE_Pin, GPIO_PIN_RESET);

	for (;;) {
		for (columnIndex = 0; columnIndex < 4; columnIndex++) {

			//if (str[columnIndex + 1] == '.') segmants += 0x8

			columnData = displayChar[digitsRow1[columnIndex]] << 8;
			columnData += displayChar[digitsRow2[columnIndex]] << 16;
			columnData += displayChar[digitsRow3[columnIndex]];

			// All columns OFF while pushing serial data
			HAL_GPIO_WritePin(DISPLAY_DIGIT_1_GPIO_Port, DISPLAY_DIGIT_1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(DISPLAY_DIGIT_2_GPIO_Port, DISPLAY_DIGIT_2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(DISPLAY_DIGIT_3_GPIO_Port, DISPLAY_DIGIT_3_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(DISPLAY_DIGIT_4_GPIO_Port, DISPLAY_DIGIT_4_Pin, GPIO_PIN_RESET);

			// Pushing segment serial data
			for (columnDataBit = 0; columnDataBit < 24; columnDataBit++) {
				// Bit check
				k = 0x800000 >> columnDataBit;
				if(columnData & k)
				{
					HAL_GPIO_WritePin(DISPLAY_DATA_GPIO_Port, DISPLAY_DATA_Pin, GPIO_PIN_SET);
				} else {
					HAL_GPIO_WritePin(DISPLAY_DATA_GPIO_Port, DISPLAY_DATA_Pin, GPIO_PIN_RESET);
				}
				// Clock pulse
				HAL_GPIO_WritePin(DISPLAY_CLOCK_GPIO_Port, DISPLAY_CLOCK_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(DISPLAY_CLOCK_GPIO_Port, DISPLAY_CLOCK_Pin, GPIO_PIN_SET);
			}

			// Set latch
			HAL_GPIO_WritePin(DISPLAY_STROBE_GPIO_Port, DISPLAY_STROBE_Pin, GPIO_PIN_SET);

			// Multiplexing digits
			switch (columnIndex) {
				case 0:
					HAL_GPIO_WritePin(DISPLAY_DIGIT_1_GPIO_Port, DISPLAY_DIGIT_1_Pin, GPIO_PIN_SET);
					break;
				case 1:
					HAL_GPIO_WritePin(DISPLAY_DIGIT_2_GPIO_Port, DISPLAY_DIGIT_2_Pin, GPIO_PIN_SET);
					break;
				case 2:
					HAL_GPIO_WritePin(DISPLAY_DIGIT_3_GPIO_Port, DISPLAY_DIGIT_3_Pin, GPIO_PIN_SET);
					break;
				case 3:
					HAL_GPIO_WritePin(DISPLAY_DIGIT_4_GPIO_Port, DISPLAY_DIGIT_4_Pin, GPIO_PIN_SET);
					break;
				}
			osDelay(DISPLAY_REFRESH_MILLIS);
		}
	}

}
