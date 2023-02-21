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

#include "_ver.h"
#include "globals.h"
#include "settings.h"
#include "system.h"
#include "time.h"

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
int8_t ambientPressure;

void ReadSetting(uint8_t address, uint8_t* data, uint8_t size) {
	// EEPROM read
	EEPROM_Read(address, data, size);
}

void LoadSettings() {
	// Get MCU unique ID 96 bit
	machine.MCUID[0] = *(uint32_t*)(MCU_UID_ADDRESS);
	machine.MCUID[1] = *(uint32_t*)(MCU_UID_ADDRESS + 0x04);
	machine.MCUID[2] = *(uint32_t*)(MCU_UID_ADDRESS + 0x08);

	machine.FWVersionMajor = FW_VERSION_MAJOR;
	machine.FWVersionMinor = FW_VERSION_MINOR;
	machine.FWVersionRevision = FW_VERSION_REVISION;

//	uint32_t EEID;
//	EEPROM_Read(0x00, &EEID, 4);
//	if (EEID != 0x52504147) {
//		SaveDefaults();
//	}


	// EEPROM read
	EEPROM_Read(EPA_MACHINE_SERIAL_NUMBER, &machine.Serial, 12);
	EEPROM_Read(EPA_CYCLE_COUNTER_PERMANENT, &machine.ReadOnlyCyclesCount, 2);
	EEPROM_Read(EPA_CYCLE_COUNTER, &settings.CyclesCount, 2);

	EEPROM_Read(EPA_T1_OFFSET, &settings.T1Offset, 2);
	EEPROM_Read(EPA_T2_OFFSET, &settings.T2Offset, 2);
	EEPROM_Read(EPA_CHAMBER_PRESSURE_OFFSET, &settings.ChamberPressOffset, 2);
	EEPROM_Read(EPA_FIRST_VACUUM, &settings.FirstVacuum, 1);
	EEPROM_Read(EPA_OTHER_VACUUM, &settings.OtherVacuum, 1);
	EEPROM_Read(EPA_VACUUM_THRESHOLD, &settings.VacuumThreshold, 1);
	EEPROM_Read(EPA_OTHER_PRESSURE, &settings.OtherPressure, 1);
	EEPROM_Read(EPA_TDS_MIN, &settings.WaterTDSMin, 1);
	EEPROM_Read(EPA_VACUUM_NUMBER, &settings.VacNumber, 1);
	EEPROM_Read(EPA_CLEAN_CYCLES_INTERVAL, &settings.CleanCyclesInterval, 1);
	EEPROM_Read(EPA_SERVICE_CYCLES_INTERVAL, &settings.ServiceCyclesInterval, 1);
	EEPROM_Read(EPA_LAST_CLEAN_CYCLE, &settings.LastCleanCycle, 2);
	EEPROM_Read(EPA_LAST_SERVICE_CYCLE, &settings.LastServiceCycle, 2);

}

void WriteSetting(uint8_t address, uint8_t* data, uint8_t size) {
	// EEPROM read
	EEPROM_Write(address, data, size);
}

void SaveSettings() {

	EEPROM_Write(EPA_T1_OFFSET, &settings.T1Offset, 2);
	EEPROM_Write(EPA_T2_OFFSET, &settings.T2Offset, 2);
	EEPROM_Write(EPA_CHAMBER_PRESSURE_OFFSET, &settings.ChamberPressOffset, 2);
	EEPROM_Write(EPA_FIRST_VACUUM, &settings.FirstVacuum, 1);
	EEPROM_Write(EPA_OTHER_VACUUM, &settings.OtherVacuum, 1);
	EEPROM_Write(EPA_VACUUM_THRESHOLD, &settings.VacuumThreshold, 1);
	EEPROM_Write(EPA_OTHER_PRESSURE, &settings.OtherPressure, 1);
	EEPROM_Write(EPA_TDS_MIN, &settings.WaterTDSMin, 1);
	EEPROM_Write(EPA_VACUUM_NUMBER, &settings.VacNumber, 1);
	EEPROM_Write(EPA_CLEAN_CYCLES_INTERVAL, &settings.CleanCyclesInterval, 1);
	EEPROM_Write(EPA_SERVICE_CYCLES_INTERVAL, &settings.ServiceCyclesInterval, 1);

}

void SaveDefaults() {

	RTCDateTime_s tTime = {2023, 1, 1, 12, 0, 0};
	SetDateTime(&tTime);



	char sn[12] = "GP0000000000";
	EEPROM_Write(EPA_MACHINE_SERIAL_NUMBER, sn, 12);
	EEPROM_Write(EPA_CYCLE_COUNTER_PERMANENT, 0, 2);
	EEPROM_Write(EPA_CYCLE_COUNTER, 0, 2);

	EEPROM_Write(EPA_T1_OFFSET, 0, 2);
	EEPROM_Write(EPA_T2_OFFSET, 0, 2);
	EEPROM_Write(EPA_CHAMBER_PRESSURE_OFFSET, 0, 2);
	EEPROM_Write(EPA_FIRST_VACUUM, 18, 1);
	EEPROM_Write(EPA_OTHER_VACUUM, 70, 1);
	EEPROM_Write(EPA_VACUUM_THRESHOLD, 40, 1);
	EEPROM_Write(EPA_OTHER_PRESSURE, 170, 1);
	EEPROM_Write(EPA_TDS_MIN, 25, 1);

	char s[4] = {'G', 'A', 'P', 'R'};
	EEPROM_Write(0x00, s, 4); // Numeric ASCII for 'GAPR' (in little endian)

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


