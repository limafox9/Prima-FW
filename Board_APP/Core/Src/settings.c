/*==============================================================================

Filename: settings.c

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

#include "settings.h"

#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "settings.h"
#include "system.h"

typedef union {
	uint8_t u8;
	uint16_t u16;
	uint32_t u32;
	float flt;
} u_value;

typedef struct {
	uint32_t eepromAddress;
	e_DataType dataType;
	const char* name;
	u_value value;
} s_SettingItem;

Settings_s settings;
MachineData_s machine;

void LoadSettings() {
	// Get MCU unique ID 96 bit
	machine.MCUID[0] = *(uint32_t*)(MCU_UID_ADDRESS);
	machine.MCUID[1] = *(uint32_t*)(MCU_UID_ADDRESS + 0x04);
	machine.MCUID[2] = *(uint32_t*)(MCU_UID_ADDRESS + 0x08);
	// EEPROM read
	EEPROM_Read(EPA_MACHINE_SERIAL_NUMBER, &machine.Serial, 12);
	EEPROM_Read(EPA_CYCLE_COUNTER_PERMANENT, &machine.ReadOnlyCyclesCount, 2);
	EEPROM_Read(EPA_CYCLE_COUNTER, &settings.Cycles, 2);
	EEPROM_Read(EPA_T1_OFFSET, &settings.T1Offset, 2);
	EEPROM_Read(EPA_T2_OFFSET, &settings.T2Offset, 2);
	EEPROM_Read(EPA_CHAMBER_PRESSURE_OFFSET, &settings.ChamberPressOffset, 2);
}

void SaveSettings() {

	EEPROM_Write(EPA_T1_OFFSET, &settings.T1Offset, 2);
	EEPROM_Write(EPA_T2_OFFSET, &settings.T2Offset, 2);
	EEPROM_Write(EPA_CHAMBER_PRESSURE_OFFSET, &settings.ChamberPressOffset, 2);
}

//s_SettingItem settings[] = {
//		{0x00000000, UINT8, "LOG_ACTIVE", {.u8 = 1}},
//		{0x00000000, UINT8, "LOG_ACTIVE_LEVEL", {.u8 = LOG_LEVEL_DEBUG} },
//
//		{0x00000000, UINT16, "DOOR_SOLENOID_IMPULSE", {.u16 = 1000} },
//};
//
//void* GetSetting(const char* settingName) {
//
//	uint8_t size = sizeof(settings) / sizeof(s_SettingItem);
//	void* ret;
//
//	for (int i = 0; i < size; i++) {
//		if (!strcmp(settingName, settings[i].name)) {
//			switch (settings[i].dataType) {
//				case UINT8:
//					ret = (uint8_t*)&settings[i].value.u8;
//					break;
//				case UINT16:
//					ret = (uint16_t*)&settings[i].value.u16;
//					break;
//				case UINT32:
//					ret = (uint32_t*)&settings[i].value.u32;
//					break;
//				case FLOAT:
//					ret = (float*)&settings[i].value.flt;
//					break;
//			}
//		}
//	}
//
//	return ret;
//}
//
//uint8_t SetSetting(const char* settingName, void* value) {
//
//	uint8_t size = sizeof(settings) / sizeof(s_SettingItem);
//	uint8_t ret = 0;
//
//	for (int i = 0; i < size; i++) {
//		if (!strcmp(settingName, settings[i].name)) {
//			ret = 1;
//			switch (settings[i].dataType) {
//				case UINT8:
//					settings[i].value.u8 = *(uint8_t*)value;
//					break;
//				case UINT16:
//					settings[i].value.u16 = *(uint16_t*)value;
//					break;
//				case UINT32:
//					settings[i].value.u32 = *(uint32_t*)value;
//					break;
//				case FLOAT:
//					settings[i].value.flt = *(float*)value;
//					break;
//			}
//		}
//	}
//
//	return ret;
//}


