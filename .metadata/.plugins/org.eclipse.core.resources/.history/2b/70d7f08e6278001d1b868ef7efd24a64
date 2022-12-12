/*
 * serial.h
 *
 *  Created on: Dec 6, 2022
 *      Author: josh
 */

#ifndef INC_SERIAL_H_
#define INC_SERIAL_H_

#include "crc16.h"
#include "main.h"
#include "globals.h"

#define UART huart1
#define SERIAL_TX_TIMEOUT 50

#define SERIAL_PACKET_HEAD 0x3C4F437E
#define SERIAL_PACKET_TAIL 0x2A3E0D0A

typedef enum command_e{
	COMMAND_PLAY_BEEP = 0x011A,
	COMMAND_KEYS_PRESSED = 0x0111,
	COMMAND_DISPLAY_DIGITS = 0x0117,
	COMMAND_SET_PANEL_LEDS = 0x011E,
	COMMAND_SET_PROGRESS_LEDS = 0x011F
} command_e;

extern void SendKeys(uint8_t keys);
//void sendSerialData(struct serialKeys_s* keys, UART_HandleTypeDef* huart);

#endif /* INC_SERIAL_H_ */
