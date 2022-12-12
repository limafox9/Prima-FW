/*==============================================================================

Filename: DB_Protocol.h

Created on: Jan 13, 2023

Original work: Copyright (c) 2023 Luca Favenzi
Modified work:

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Visit www.favenzi.it
      luca@favenzi.it

================================================================================
PURPOSE:
USAGE:
==============================================================================*/



#ifndef DB_PROTOCOL_H_
#define DB_PROTOCOL_H_

#include <stdio.h>

#define POS_HEAD 		0
#define POS_COMMAND 	2
#define POS_PARAM1		4
#define POS_PARAM2 		8
#define POS_PARAM3 		12
#define POS_DIGITS 		16
#define POS_CHECKSUM 	24
#define POS_TAIL 		26
#define POS_EOF			28

#define DISPLAY_PROTOCOL_BUFFER_SIZE (POS_EOF + 1)

#define DISPLAY_UART_SPEED 				115200
#define DISPLAY_UART_BUFFER_SIZE 		48
#define DISPLAY_UART_TX_TIMEOUT 		500
#define DISPLAY_UART_RX_TIMEOUT 		500

#define KEY_PROG 		1
#define KEY_TEST 		2
#define KEY_START 		4

#define KEY_S1 			1
#define KEY_S2 			2
#define KEY_S3 			4

#define KEY_LONG_PRESS 	8

typedef enum {
	DISPLAY_CMD_OK 							= 0x0100,	// message received ok. param1..4 depend on command
	DISPLAY_CMD_ERROR 						= 0x0110,	// message error (fail or incomplete receive). param1..4 depend on command
	DISPLAY_CMD_BEEP 						= 0x00DB,	// beeps buzzer on display board. param1: n. of beeps; param2: on time(ms); param3: off time(ms)
	DISPLAY_CMD_KEYS_PRESSED 				= 0x6666, 	// keys pressed on display board
	DISPLAY_CMD_WRITE_DIGITS 				= 0x00D1, 	// write 7-segment digits. param1: row (1..3); param2: digits flash bit mask; param3: flash rate (ms); param4 text
	DISPLAY_CMD_WRITE_DASHBOARD_LEDS 		= 0x00DE, 	// write dashboard leds. param1: leds bits; param2: leds flash bit mask; param3: flash rate (ms)
	DISPLAY_CMD_WRITE_PROGRESS_LEDS 		= 0x00DF, 	// write progress leds. param1: leds bits; param2: leds flash bit mask; param3: flash rate (ms)
	DISPLAY_CMD_SET_KEYS_BEEP 				= 0x0055,	// keys pressed beep on/off. param1: 1 = panel buzzer beep on keypress, 0 = mute
	DISPLAY_CMD_SET_LONG_PRESS_INTERVAL		= 0x0056,	// keys long press duration. param1: press duration (milliseconds)
	DISPLAY_CMD_SET_FLASH_INTERVAL			= 0x0057,	// leds and digits flash rate. param1: flash duration (milliseconds)
	DISPLAY_CMD_READY 						= 0x0043, 	// display (re)booted and ready
	DISPLAY_CMD_CHECK 						= 0x0032,	// display check. All leds and segments ON + BEEP. Param1: ON time (ms); param2 n. of beeps (100ms ON, 100ms OFF)
	DISPLAY_CMD_FW_VER 						= 0x0012, 	// get firmware version
} e_DisplayProtocolCommand;

typedef enum {
	DISPLAY_LED_MODE_UNWRAPPED			= 1,		// 0b0000000000000001
	DISPLAY_LED_MODE_WRAPPED			= 2,		// 0b0000000000000010
	DISPLAY_LED_MODE_PRION				= 4,		// 0b0000000000000100
	DISPLAY_LED_MODE_GENTLE				= 8,		// 0b0000000000001000
	DISPLAY_LED_TEST_HELIX				= 16,		// 0b0000000000010000
	DISPLAY_LED_TEST_VACUUM				= 32,		// 0b0000000000100000
	DISPLAY_LED_WARNING_CTL				= 64,		// 0b0000000001000000
	DISPLAY_LED_WARNING_LSH				= 128,		// 0b0000000010000000
	DISPLAY_LED_WARNING_TDS				= 256,		// 0b0000000100000000
	DISPLAY_LED_TEMP_121				= 512,		// 0b0000001000000000
	DISPLAY_LED_TEMP_134				= 1024,		// 0b0000010000000000
	//
	DISPLAY_LED_BCLASS					= 2048,		// 0b0000100000000000
	DISPLAY_LED_WATER_FILL				= 4096,		// 0b0001000000000000
	DISPLAY_LED_WASH_CYCLE				= 8192,		// 0b0010000000000000
} e_DisplayDashboardLeds;

typedef enum {
	LED_PROGRESS_NONE = 			0b0000000000000000,
	LED_PROGRESS_HEATING = 			0b0000000000000001,
	LED_PROGRESS_1STVACUUM = 		0b0000000000000011,
	LED_PROGRESS_1STPRESSURE = 		0b0000000000000111,
	LED_PROGRESS_NVACUUM = 			0b0100000000000111,
	LED_PROGRESS_NPRESSURE = 		0b0100000000010111,
	LED_PROGRESS_NRELEASE = 		0b0100000000110111,
	LED_PROGRESS_STERILVACUUM = 	0b0100000001110111,
	LED_PROGRESS_STERILPRESSURE = 	0b0100000011110111,
	LED_PROGRESS_STERILMAINTAIN = 	0b0100000111110111,
	LED_PROGRESS_STERILRELEASE = 	0b0100001111110111,
	LED_PROGRESS_DRY = 				0b0100011111110111,
} ledProgress;

typedef struct __s_DisplayProtocolData {
	e_DisplayProtocolCommand Command;
	uint32_t Param1;
	uint32_t Param2;
	uint32_t Param3;
	char Digits[8];
	uint8_t Valid;
} s_DisplayProtocolData;

void Display_DataToProtocol(s_DisplayProtocolData* dataIn, uint8_t* bufferOut);
void Display_ProtocolToData(uint8_t* bufferIn, s_DisplayProtocolData* dataOut);

#endif /* DB_PROTOCOL_H_ */
