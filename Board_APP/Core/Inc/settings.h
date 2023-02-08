/*==============================================================================

Filename: settings.h

Created on: Jan 8, 2023

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


#ifndef INC_SETTINGS_H_
#define INC_SETTINGS_H_

#include <stdio.h>

#define DEFAULT_LOG_ACTIVE						1		// 1 = true, 0 = false
#define DEFAULT_LOG_ACTIVE_LEVEL 				0		// Log level 0 = all, 99 = none

#define DEFAULT_DOOR_SOLENOID_IMPULSE 	1000	// milliseconds

#define FW_VERSION_MAJOR 0
#define FW_VERSION_MINOR 3
#define FW_VERSION_REVISION 15

//#define UART_MONITOR		huart1
#define UART_TDS			huart2
#define UART_DISPLAY		huart3
#define UART_PRINTER		huart4
#define UART_WIFI			//TBD

#define SPI_ADC_T1T2		hspi1
#define SPI_FLASH			hspi2

#define I2C_EEPROM_CLOCK	hi2c1
#define I2C_ATM_PRESSURE 	//TBD


#define EPA_MACHINE_PROD_YEAR			0x0E	// 1 byte 0-199 after 2000
#define EPA_MACHINE_PROD_MONTH			0x0F	// 1 byte 0-52 week of the year
#define EPA_MACHINE_SERIAL_NUMBER	 	0x10	// 12 bytes
#define EPA_CYCLE_COUNTER_PERMANENT 	0x1C	// 2 bytes - Cycla counter permanent
#define EPA_CYCLE_COUNTER 				0x1E	// 2 bytes - Cycle counter resettable
#define EPA_T1_OFFSET 					0x20	// 1 byte +/- 128 in 0.1 steps
#define EPA_T2_OFFSET 					0x21	// 1 byte +/- 128 in 0.1 steps
#define EPA_CHAMBER_PRESSURE_OFFSET 	0x22	// 1 byte +/- 128 in 1 steps

typedef struct Settings_s {
	uint16_t Cycles;
	int16_t T1Offset;
	int16_t T2Offset;
	int16_t ChamberPressOffset;
} Settings_s;

typedef struct MachineData_s{
	uint32_t MCUID[3];
	uint16_t ReadOnlyCyclesCount;
	char Serial[12];
	uint8_t ProductionYear;
	uint8_t ProductionWeek;
} MachineData_s;

void SaveSettings();
void LoadSettings();

void* GetSetting(const char* settingName);
uint8_t SetSetting(const char* settingName, void* value);

#endif /* INC_SETTINGS_H_ */
