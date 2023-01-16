/*==============================================================================

Filename: serial.h

Created on: Dec 15, 2022

Original work: Copyright (c) 2022 Luca Favenzi
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

==================================================================================
PURPOSE:
USAGE:
================================================================================*/

#ifndef INC_SERIAL_H_
#define INC_SERIAL_H_

//#include "main.h"
//#include "globals.h"

//typedef enum _protocolCommand {
//	COMMAND_OK = 0x100,
//	COMMAND_PLAY_BEEP = 0xDB,			// param1: n. of beeps, param2: onTIme(ms), param3: offTime(ms)
//	COMMAND_KEYS_PRESSED = 0x6666,
//	COMMAND_WRITE_DIGITS_ROW_1 = 0xD1,
//	COMMAND_WRITE_DIGITS_ROW_2 = 0xD2,
//	COMMAND_WRITE_DIGITS_ROW_3 = 0xD3,
//	COMMAND_SET_DASHBOARD_LEDS = 0xDE,
//	COMMAND_SET_PROGRESS_LEDS = 0xDF,
//	COMMAND_SET_PANEL_KEYS_BEEP = 0x55,	// param1: 1 = panel buzzer beep on keypress, 0 = mute
//	COMMAND_PANEL_READY = 0x43,
//	COMMAND_PANEL_CHECK = 0x32,
//	COMMAND_GET_PANEL_FW_VERSION = 0x12
//} protocolCommand;
//

//
//void PanelSerialInit(UART_HandleTypeDef* pUART, DMA_HandleTypeDef* pDMA);
//void DisplaySend(uint16_t command, uint32_t param1, uint32_t param2, uint32_t param3, char* digits);
//void PanelSerialReceive(uint8_t* buffer, uint16_t size);
//void StartSerialTask();

#endif /* INC_SERIAL_H_ */
