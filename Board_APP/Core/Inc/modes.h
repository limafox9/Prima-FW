/*==============================================================================

Filename: modes.h

Created on: 9 feb 2023

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

#include <stdio.h>

#ifndef INC_MODES_H_
#define INC_MODES_H_

typedef enum {
	STATUS_IDLE = 1,
	STATUS_READY = 2,
	STATUS_RUNNING = 4,
	STATUS_FAIL = 8,
	STATUS_PASS = 16,
	STATUS_ERROR = 32,
} e_MachineStatus;

typedef enum {
	SHOW_PROGRAM,
	SHOW_PARAMETERS,
	SHOW_READINGS,
	SHOW_TIMESET,
	SHOW_USERSET,
	SHOW_FUNCTIONS,
	SHOW_ERROR,
} e_ShowStatus;

typedef enum {
	PROGTYPE_USER,
	PROGTYPE_PROGRAM,
	PROGTYPE_TEST,
	PROGTYPE_RESUME,
} e_ProgramType;

typedef enum {
	PROGRAM_USER,
	PROGRAM_UNWRAPPED,
	PROGRAM_WRAPPED,
	PROGRAM_PRION,
	PROGRAM_GENTLE,
	PROGRAM_STERIL_TEST,
	PROGRAM_RESUME_DRY = 99,
} e_Program;

typedef enum {
	TEST_HELIX,
	TEST_VACUUM,
	TEST_WASH,
	TEST_WATER_LOAD,
} e_Test;

typedef enum {
	CLASS_USER,
	CLASS_B,
	CLASS_S,
} e_ProgramClass;

typedef enum {
	STERILTEMP_121 = 0,
	STERILTEMP_134 = 1,
} e_ProgramTemp;

typedef struct UserSet_s {
	uint8_t ParamNumber;
	char Name[16];
	char DigitsName[8];
	uint8_t EepromAddress;
	int8_t* Value;
	uint8_t Size;
	int8_t Min;
	int8_t Max;
} UserSet_s;

typedef struct Functions_s {
	uint8_t ParamNumber;
	char Name[16];
	char DigitsName[8];
	uint8_t EepromAddress;
	int8_t* Value;
	uint8_t Size;
	int8_t Min;
	int8_t Max;
} Functions_s;

typedef enum {
	ERROR_NONE = 0,
	ERROR_STEAM_OVERTEMP = 1,
	ERROR_RING_OVERTEMP =  2,
	ERROR_CHAMBER_OVERTEMP = 3,
	ERROR_MAINTAIN_PRESSTEMP = 4,
	ERROR_NO_PRESSURE_RELEASE = 5,
	ERROR_OPEN_DOOR = 6,
	ERROR_PHASE_TIMEOUT = 7,
	ERROR_OVERPRESSURE = 8,
	ERROR_T1T2_MISMATCH = 9,
	ERROR_TEMP_PRESS_MISMATCH = 10,
	ERROR_PROTECTION_LOCK = 11,
	ERROR_VACUUM_TEST_FAILED  = 12,
	ERROR_WATER_HIGH_TDS = 14,
	ERROR_WATER_LOW = 15,
	ERROR_WATER_DUMP_HIGH = 16,
	ERROR_POWER_OFF = 98,
	ERROR_ABNORMAL_EXIT = 99,
} ProgramError_e;

typedef struct s_Program {
	uint8_t ProgNumber;
	e_ProgramType ProgType;
	char Name[16];
	char DigitsName[8];
	uint16_t DashLeds;
	e_ProgramClass Class;
	e_ProgramTemp SterilTemp;
	uint8_t NVacuum;
	uint16_t SterilPress;
	uint16_t SterilTime;
	uint16_t DryTime;
} s_Program;

typedef struct s_MachineStatus {
	e_MachineStatus Status;
	e_ShowStatus Show;
} s_MachineStatus;

typedef struct Parameter_s {
	uint8_t ParamNumber;
	char Name[16];
	char DigitsName[8];
	uint8_t EepromAddress;
	int8_t* Value;
	uint8_t Size;
	int8_t Min;
	int8_t Max;
	float Increment;
} Parameter_s;

typedef struct Reading_s {
	uint8_t ParamNumber;
	char Name[16];
	char DigitsName[8];
	float* Value;
} Reading_s;

typedef struct Timeset_s {
	uint8_t ParamNumber;
	char Name[16];
	char DigitsName[8];
	uint8_t* Value;
	uint8_t Min;
	uint8_t Max;
} Timeset_s;


extern s_MachineStatus machineStatus;

extern s_Program programs[];
extern Parameter_s parameters[];
extern UserSet_s user[];
extern Functions_s functions[];

extern uint8_t currProgram;
extern uint8_t currTest;
extern e_ProgramType currProgType;
extern uint8_t currParameter;
extern uint8_t currUser;
extern uint8_t currFunction;

extern uint8_t progRunning;
extern uint8_t progStart;
extern uint8_t progStop;
extern uint8_t errorCode;

void SelectProgram();
void SelectTest();
void ShowProgram();
void SelectParameter();
void ShowParameter();
void ChangeParameter(int8_t increment);
void ShowTimeset();
void SelectTimeset();
void ChangeTimeset11(int8_t increment);
void ShowUserSetting();
void SelectUserSetting();
void ChangeUserSetting(int8_t increment);
void ShowFunctions();
void SelectFunction();
void ChangeFunction(int8_t increment);
uint8_t GetDryTime(uint8_t class, uint8_t sterilTemp, uint8_t vacuumNumber);

#endif /* INC_MODES_H_ */
