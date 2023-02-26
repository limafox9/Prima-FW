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

#define DEFAULT_LOG_ACTIVE				0		// 1 = true, 0 = false
#define DEFAULT_LOG_ACTIVE_LEVEL 		0		// Log level 0 = all, 99 = none

#define DEFAULT_DOOR_SOLENOID_IMPULSE 	1000	// milliseconds

//#define UART_MONITOR		huart1
//#define UART_TDS			huart2
#define UART_DISPLAY		huart3
#define UART_PRINTER		huart4
#define UART_WIFI			//TBD

#define SPI_ADC_T1T2		hspi1
#define SPI_FLASH			hspi2

#define I2C_EEPROM_CLOCK	hi2c1
#define I2C_ATM_PRESSURE 	//TBD

//TODO: Long press interval
#define DISPLAY_LONG_PRESS_INTERVAL 3000

#define EPA_MACHINE_PROD_YEAR			0x0E	// 1 byte 0-199 after 2000
#define EPA_MACHINE_PROD_MONTH			0x0F	// 1 byte 0-52 week of the year
#define EPA_MACHINE_SERIAL_NUMBER	 	0x10	// 12 bytes
#define EPA_CYCLE_COUNTER_PERMANENT 	0x1C	// 2 bytes - Cycla counter permanent
#define EPA_CYCLE_COUNTER 				0x1E	// 2 bytes - Cycle counter resettable
#define EPA_T1_OFFSET 					0x20	// 1 byte +/- 127 in 0.1 steps
#define EPA_T2_OFFSET 					0x21	// 1 byte +/- 127 in 0.1 steps
#define EPA_CHAMBER_PRESSURE_OFFSET 	0x22	// 1 byte +/- 127 in 1 steps
#define EPA_LAST_ERROR					0x0A	// 1 byte

#define EPA_FIRST_VACUUM				0x23	// 1 byte
#define EPA_OTHER_VACUUM				0x24	// 1 byte
#define EPA_VACUUM_THRESHOLD			0x25	// 1 byte
#define EPA_OTHER_PRESSURE				0x26	// 1 byte
#define EPA_TDS_MIN						0x27	// 1 byte
#define EPA_VACUUM_NUMBER				0x28
#define EPA_CLEAN_CYCLES_INTERVAL		0x29
#define EPA_SERVICE_CYCLES_INTERVAL		0x2A
#define EPA_LAST_CLEAN_CYCLE			0x2C	// 2 byte
#define EPA_LAST_SERVICE_CYCLE			0x2E	// 2 byte

#define EPA_USER_STERIL_TEMP			0x30	// 1 byte
#define EPA_USER_STERIL_TIME			0x31	// 2 byte
#define EPA_USER_DRY_TIME				0x33	// 2 byte
#define EPA_USER_VACUUM_NUMBER			0x35	// 1 byte

#define EPA_FUN_NIGHT_CYCLE_TIME		0x40	// 2 byte
#define EPA_FUN_PRINTER_ACTIVE			0x42	// 1 byte
#define EPA_FUN_USB_ACTIVE				0x43	// 1 byte
#define EPA_FUN_WATER_LOAD_ACTIVE		0x44	// 1 byte
#define EPA_FUN_TDS_ACTIVE				0x45	// 1 byte
#define EPA_FUN_DRY_TIME				0x46	// 1 byte
#define EPA_FUN_PRESS_DISPLAY_OFFSET	0x48	// 2 byte


typedef struct Settings_s {
	uint16_t CyclesCount;
	int8_t T1Offset;				// -2..2 step 1 (0.1)
	int8_t T2Offset;				// -2..2 step 1 (0.1)
	int8_t ChamberPressOffset;		// -15..15 step 1
	int8_t WaterTDSMin;				// 5..70 step 1
	int8_t FirstVacuum;				// 10..50 step 1
	int8_t VacuumThreshold;			// 30..60 step 1
	int8_t OtherVacuum;				// 10..60 step 1
	int8_t OtherPressure;			// 120..200 step 1
	int8_t VacNumber;				// 3..5 step 1
	int8_t CleanCyclesInterval;		// 30, 60, 90
	int8_t ServiceCyclesInterval;	// 500, 1000 step 10
	int16_t LastCleanCycle;
	int16_t LastServiceCycle;
	int8_t UserSterilTemp;			// 121, 134
	int16_t UserSterilTime;			// 4..30
	int16_t UserDryTime;				// 0..3
	int8_t UserVacuumNumber;		// 0, 1, 3, 4, 5
	int16_t FunNightCycleTime;
	int8_t FunPrinterActive;	// On Off
	int8_t FunUSBActive;	// On Off
	int8_t FunWaterLoadActive;	// On Off
	int8_t FunTDSActive;	// On Off
	int16_t FunDryTime;
	int8_t FunPressureDisplay; // 0-100 kPa display @ATM
} Settings_s;

typedef enum {
	MACHINE_MODEL_18L,
	MACHINE_MODEL_23L,
} MachineModel_e;

typedef struct MachineData_s {
	uint32_t MCUID[3];
	uint16_t ReadOnlyCyclesCount;
	char Serial[12];
	uint8_t ProductionYear;
	uint8_t ProductionWeek;
	MachineModel_e Model;
	uint8_t FWVersionMajor;
	uint8_t FWVersionMinor;
	uint8_t FWVersionRevision;
	uint8_t DisplayFWVersionMajor;
	uint8_t DisplayFWVersionMinor;
	uint8_t DisplayFWVersionRevision;
} MachineData_s;

extern Settings_s settings;
extern MachineData_s machine;
extern int8_t ambientPressure;

void SaveSettings();
void LoadSettings();
void ReadSetting(uint8_t address, uint8_t* data, uint8_t size);
void WriteSetting(uint8_t address, uint8_t* data, uint8_t size);

//void* GetSetting(const char* settingName);
//uint8_t SetSetting(const char* settingName, void* value);

#endif /* INC_SETTINGS_H_ */
