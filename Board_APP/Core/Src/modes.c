/*==============================================================================

Filename: modes.c

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

#include "modes.h"

#include <stdio.h>

#include "serial.h"
#include "globals.h"
#include "sensors.h"
#include "settings.h"
#include "system.h"
#include "Inc/DB_Protocol.h"
#include "main.h"
#include "time.h"

s_MachineStatus machineStatus = {STATUS_IDLE, SHOW_PROGRAM};

uint8_t currProgram = 0;
uint8_t currTest = 0;
e_ProgramType currProgType = PROGTYPE_PROGRAM;
uint8_t currParameter = 0;
uint8_t currReading = 0;
uint8_t currTimeset = 0;
uint8_t currUser = 0;
uint8_t currFunction = 0;

uint8_t progRunning = 0;
uint8_t progStart = 0;
uint8_t progStop = 0;

s_Program programs[] = {
	{0, PROGTYPE_PROGRAM, "User", "P.USr", 15, CLASS_USER, STERILTEMP_134, 1, 310, 0, 0,},
	{1, PROGTYPE_PROGRAM, "Unwrapped", "P.Unr", 1, CLASS_B, STERILTEMP_134, 1, 310, 300, 240},
	{2, PROGTYPE_PROGRAM, "Wrapped", "P.UrP", 2, CLASS_S, STERILTEMP_134, 4, 310, 300, 540},
	{3, PROGTYPE_PROGRAM, "Prion", "P.PrI", 4, CLASS_B, STERILTEMP_134, 4, 310, 1000, 540},
	{4, PROGTYPE_PROGRAM, "Gentle", "P.GEn", 8, CLASS_B, STERILTEMP_121, 4, 220, 1200, 540},
};

s_Program tests[] = {
	{1, PROGTYPE_TEST, "Helix Test", "t.HEL", 16, CLASS_USER, STERILTEMP_134},
	{2, PROGTYPE_TEST, "Vacuum Test", "t.UAC", 32, CLASS_USER, STERILTEMP_134},
	{3, PROGTYPE_TEST, "Wash cycle", "CLEA", DISPLAY_LED_WASH_CYCLE, CLASS_USER, STERILTEMP_134},
	{4, PROGTYPE_TEST, "Water fill", " H2O", DISPLAY_LED_WATER_FILL, CLASS_USER, STERILTEMP_134},
};

Parameter_s parameters[] = {
	{1, "T1 Offset", " t1 ", EPA_T1_OFFSET, &settings.T1Offset, 1, -20, 20, 0.1},
	{2, "T2 Offset", " t2 ", EPA_T2_OFFSET, &settings.T2Offset, 1, -20, 20, 0.1},
	{3, "P1 Offset", " P1 ", EPA_CHAMBER_PRESSURE_OFFSET, &settings.ChamberPressOffset, 1, -15, 15, 1},
	{4, "First Vacuum", " PH1", EPA_FIRST_VACUUM, &settings.FirstVacuum, 1, 10, 50, 1},
	{5, "Other Vacuum", " PH ", EPA_OTHER_VACUUM, &settings.OtherVacuum, 1, 10, 60, 1},
	{6, "Vacuum Threshold", " PU ", EPA_VACUUM_THRESHOLD, &settings.VacuumThreshold, 1, 30, 60, 1},
	{7, "Other Pressure", "HIGH", EPA_OTHER_PRESSURE, &settings.OtherPressure, 1, 12, 20, 10},
	{8, "Min. TDS", " CO ", EPA_TDS_MIN, &settings.WaterTDSMin, 1, 5, 70, 1},
	{9, "Vacuum Number", " UA ", EPA_TDS_MIN, &settings.VacNumber, 1, 3, 5, 1},
//	{10, "Clean Interval", "CLEA", EPA_TDS_MIN, &settings.CleanCyclesInterval, 1, 3, 9, 10},
//	{11, "Service interval", "SErU", EPA_TDS_MIN, &settings.ServiceCyclesInterval, 1, 5, 10, 100},
};

Reading_s readings[] = {
	{1, "T1", " t1 ", &sensors.T1},
	{2, "T2", " t2 ", &sensors.T2},
	{3, "T3", " t3 ", &sensors.TemperatureChamberHeater},
	{4, "T4", " t4 ", &sensors.TemperatureSteamGenerator},
	{5, "P chamber", " P1 ", &sensors.PressureChamber},
	{6, "P ambient", " PA ", &sensors.PressureAmbient},
	{7, "TDS", " CO ", (uint16_t*)(&sensors.WaterTDS)},
};

UserSet_s user[] = {
	{0, "User Steril Temp", " tE ", EPA_USER_STERIL_TEMP, &settings.UserSterilTemp, 1, 0, 1},
	{1, "User Steril Time", " St ", EPA_USER_STERIL_TIME, &settings.UserSterilTime, 1, 4, 30},
	{2, "User Dry Time", " dr ", EPA_USER_DRY_CLASS, &settings.UserDryClass, 1, 0, 3},
	{3, "User Vacuum Number", " UA ", EPA_USER_VACUUM_NUMBER, &settings.UserVacuumNumber, 1, 0, 5},
};

Functions_s functions[] = {
	{1, "Night cycle time", 		"POtI", EPA_FUN_NIGHT_CYCLE_TIME, &settings.FunNightCycleTime, 2, 0, 120},
	{2, "Printer on", 				"Prin", EPA_FUN_PRINTER_ACTIVE, &settings.FunPrinterActive, 1, 0, 1},
	{3, "USB on", 					" USb", EPA_FUN_USB_ACTIVE, &settings.FunUSBActive, 1, 0, 1},
	{4, "Water load on", 			"PUnP", EPA_FUN_WATER_LOAD_ACTIVE, &settings.FunWaterLoadActive, 1, 0, 1},
	{5, "TDS on", 					" CO ", EPA_FUN_TDS_ACTIVE, &settings.FunTDSActive, 1, 0, 1},
	{6, "Pressure display", 		"PrES", EPA_FUN_PRESS_DISPLAY_OFFSET, &settings.FunPressureDisplay, 1, 0, 1},
	{7, "Dry Time", 				" dry", EPA_FUN_DRY_CLASS, &settings.FunDryClass, 1, 1, 3},
//	{8, "USB Export", 				"ESPr", 0, 0, 0, 0, 0, 0},
	{9, "USB Import parameters", 	"LoAd", 0, 0, 0, 0, 0},
	{10, "Power test", 		"tESt", 0, 0, 0, 0, 0},
//	{11, "Print user parameters", 	"PUPA", 0, 0, 0, 0, 0},
	{12, "Print tech parameters", 	"PtPA", 0, 0, 0, 0, 0},
//	{13, "Print last ticket", 		"PLtI", 0, 0, 0, 0, 0},
};


Timeset_s timeset[] = {
	{1, "Year", "yEAr", &RTCDateTime.Year, 22, 99},
	{2, "Month", "nnon", &RTCDateTime.Month, 1, 12},
	{3, "Day", " dAy", &RTCDateTime.Day, 1, 31},
	{4, "Hour", "Hour", &RTCDateTime.Hour, 0, 23},
	{5, "Minute", "nnin", &RTCDateTime.Minute, 0, 59},
};

const uint8_t monthDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void ProgramsInit() {

	for (int i = 2; i < ((sizeof(tests) / sizeof(s_Program)) - 1); i++) {  // 0 (user) and 1 (wrapped) excluded
		programs[i].NVacuum = settings.VacNumber;
	}
}

void SelectProgram() {
	if (currProgType == PROGTYPE_PROGRAM) currProgram++;
	else {
		currProgType = PROGTYPE_PROGRAM;
	}
	if (currProgram > ((sizeof(programs) / sizeof(s_Program)) - 1)) currProgram = 0;
//	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, &programs[currProgram].DigitsName);
////			SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, 255, OFF, 0, "");
//	SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, 63 + DISPLAY_LED_WASH_CYCLE + DISPLAY_LED_WATER_FILL, OFF, 0, "");
//	SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, programs[currProgram].DashLeds, ON, 0, "");
	ShowProgram();
}

void SelectTest() {
	if (currProgType == PROGTYPE_TEST) currTest++;
	else {
		currProgType = PROGTYPE_TEST;
	}
	if (currTest > ((sizeof(tests) / sizeof(s_Program)) - 1)) currTest = 0;
//	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, &tests[currTest].DigitsName);
////			SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, 255, OFF, 0, "");
//	SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, tests[currTest].DashLeds, ON, 0, "");
	ShowProgram();
}

void ShowProgram() {

	static lastProg, lastTest, lastProgType;

	if (machineStatus.Status == STATUS_ERROR) {
		char s[8];
		sprintf(s,"Er.%02d", lastError);
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, s);
	} else {
		if (machineStatus.Status != STATUS_RUNNING) {
			if (currProgType == PROGTYPE_PROGRAM) {
				if (machineStatus.Status == STATUS_PASS) {
					DigitsWrite(3, "PASS");
				} else {
					if (currProgram == PROGRAM_RESUME_DRY) {
						DigitsWrite(3, "----");
					} else {
						if (doorOpened == 0) {
							DigitsWrite(3, "OPEN");
						} else {
							DigitsWrite(3, (sensors.Door == OPEN) ? "LoAd" : &programs[currProgram].DigitsName);
						}
					}
				}
				if (lastProg != programs[currProgram].DashLeds || lastProgType != currProgType) {
					SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, 63 + DISPLAY_LED_WASH_CYCLE + DISPLAY_LED_WATER_FILL, OFF, 0, "");
	//				DashLedsOff();
	//				SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, ~programs[currProgram].DashLeds, OFF, 0, "");
					if (currProgram != PROGRAM_RESUME_DRY) SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, programs[currProgram].DashLeds, ON, 0, "");
					lastProg = programs[currProgram].DashLeds;
				}
				if (currProgram == PROGRAM_USER) {
					DashLed(DISPLAY_LED_BCLASS, ((settings.UserVacuumNumber >= 3) && ((settings.UserSterilTemp == STERILTEMP_134) || settings.UserSterilTime >= 18)) ? ON : OFF);
					DashLed(DISPLAY_LED_TEMP_134, (settings.UserSterilTemp == STERILTEMP_134) ? ON : OFF);
					DashLed(DISPLAY_LED_TEMP_121, (settings.UserSterilTemp == STERILTEMP_121) ? ON : OFF);
				} else {
					DashLed(DISPLAY_LED_BCLASS, (programs[currProgram].NVacuum >= 3) ? ON : OFF);
					DashLed(DISPLAY_LED_TEMP_134, (programs[currProgram].SterilTemp == STERILTEMP_134) ? ON : OFF);
					DashLed(DISPLAY_LED_TEMP_121, (programs[currProgram].SterilTemp == STERILTEMP_121) ? ON : OFF);
				}
			}
			if (currProgType == PROGTYPE_TEST) {
				if (machineStatus.Status == STATUS_PASS) {
					DigitsWrite(3, "PASS");
				} else {
					if (currProgram == PROGRAM_RESUME_DRY) {
						DigitsWrite(3, "----");
					} else {
						if (doorOpened == 0) {
							DigitsWrite(3, "OPEN");
						} else {
							DigitsWrite(3, (sensors.Door == OPEN) ? "LoAd" : &tests[currTest].DigitsName);
						}
					}
				}
				if (lastTest != tests[currTest].DashLeds || lastProgType != currProgType) {
	//				SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, &tests[currTest].DigitsName);
					SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, 63 + DISPLAY_LED_WASH_CYCLE + DISPLAY_LED_WATER_FILL, OFF, 0, "");
	//				DashLedsOff();
	//				SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, ~programs[currProgram].DashLeds, OFF, 0, "");
					if (currProgram != PROGRAM_RESUME_DRY) SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, tests[currTest].DashLeds, ON, 0, "");
					lastTest = tests[currTest].DashLeds;
				}
				DashLed(DISPLAY_LED_TEMP_134, (currTest == TEST_HELIX) ? ON : OFF);
				DashLed(DISPLAY_LED_TEMP_121, OFF);
				DashLed(DISPLAY_LED_BCLASS, (currTest == TEST_HELIX) ? ON : OFF);
			}
		}
	}

	lastProgType = currProgType;

	osSemaphoreWait(semaphoreSensorsHandle, 100);
	char s[8];
	sprintf(s,"%5.1f", sensors.T1);
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 1, 0, 0, s);
	osDelay(20);
	sprintf(s,"%5.1f", GetNormalizedPressure(sensors.PressureChamber)) ;
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 2, 0, 0, s);
	osDelay(20);
	osSemaphoreRelease(semaphoreSensorsHandle);
}

void ShowParameter() {
	osSemaphoreWait(semaphoreSensorsHandle, 100);
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 1, 0, 0, &parameters[currParameter].DigitsName);
	char s[8];
	if (parameters[currParameter].Increment < 1) {
		sprintf(s,"%5.1f", (float)((float)*parameters[currParameter].Value) * parameters[currParameter].Increment);
	} else {
//		sprintf(s,"%4d", *parameters[currParameter].Value * (uint8_t)parameters[currParameter].Increment);
		sprintf(s,"%4d", (int16_t)(*parameters[currParameter].Value * parameters[currParameter].Increment));
	}
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 2, 0, 0, s);
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "PArA");
	osSemaphoreRelease(semaphoreSensorsHandle);
}

void SelectParameter() {
	currParameter++;
	if (currParameter > ((sizeof(parameters) / sizeof(Parameter_s)) - 1)) currParameter = 0;
	ShowParameter();
}

void ChangeParameter(int8_t increment) {
	*parameters[currParameter].Value += increment;
	if (*parameters[currParameter].Value < parameters[currParameter].Min) *parameters[currParameter].Value = parameters[currParameter].Min;
	if (*parameters[currParameter].Value > parameters[currParameter].Max) *parameters[currParameter].Value = parameters[currParameter].Max;
//	WriteSetting(parameters[currParameter].EepromAddress, parameters[currParameter].Data, 1);
	ShowParameter();
}

void ShowReading() {
	osSemaphoreWait(semaphoreSensorsHandle, 100);
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 1, 0, 0, &readings[currReading].DigitsName);
	char s[8];
	if (currReading == 6) {
		sprintf(s,"%4d", sensors.WaterTDS);
	} else if (currReading == 4 || currReading == 5) {
		sprintf(s,"%5.1f", GetNormalizedPressure((float)(*readings[currReading].Value)));
	} else {
		sprintf(s,"%5.1f", (float)(*readings[currReading].Value));
	}
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 2, 0, 0, s);
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "rEAd");
	osSemaphoreRelease(semaphoreSensorsHandle);
}

void SelectReading() {
	currReading++;
	if (currReading > ((sizeof(readings) / sizeof(Reading_s)) - 1)) currReading = 0;
	ShowReading();
}

void ShowUserSetting() {
	osSemaphoreWait(semaphoreSensorsHandle, 100);
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 1, 0, 0, &user[currUser].DigitsName);
	char s[8];
	if (currUser == 0) {
		sprintf(s,"%4d", (*user[currUser].Value == 0) ? 121 : 134);
	} else if (currUser == 2){
		sprintf(s,"%4d", GetDryTime(*user[currUser].Value, settings.UserSterilTemp, settings.UserVacuumNumber));
	} else {
		sprintf(s,"%4d", *user[currUser].Value);
	}
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 2, 0, 0, s);
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "USEr");
	osSemaphoreRelease(semaphoreSensorsHandle);
}

void SelectUserSetting() {
	currUser++;
	if (currUser > ((sizeof(user) / sizeof(UserSet_s)) - 1)) currUser = 0;
	ShowUserSetting();
}

void ChangeUserSetting(int8_t increment) {
	*user[currUser].Value += increment;

	if (currUser == 3) { // 0 to 5, no 2
		if (*user[currUser].Value == 2) {
			if (increment > 0) *user[currUser].Value = 3;
			else *user[currUser].Value = 1;
		}
	}

	if (*user[currUser].Value < user[currUser].Min) *user[currUser].Value = user[currUser].Max;
	if (*user[currUser].Value > user[currUser].Max) *user[currUser].Value = user[currUser].Min;

	settings.UserDryTime = GetDryTime(settings.UserDryClass, settings.UserSterilTemp, settings.UserVacuumNumber);

//	WriteSetting(parameters[currParameter].EepromAddress, parameters[currParameter].Data, 1);
	ShowUserSetting();
}

void ShowFunctions() {
	osSemaphoreWait(semaphoreSensorsHandle, 100);
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 1, 0, 0, &functions[currFunction].DigitsName);
	char s[8];
	if ((currFunction >= 1) && (currFunction < 5)) {
		sprintf(s,"%4s", (*functions[currFunction].Value == 1) ? "On" : " OFF");
	} else if ((currFunction == 5)) {
		sprintf(s,"%4s", (*functions[currFunction].Value == 1) ? " 100" : "   0");
	} else if ((currFunction >= 7)) {
		sprintf(s,"Strt");
	} else {
		sprintf(s,"%4d", *functions[currFunction].Value);
	}
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 2, 0, 0, s);
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "FUnC");
	osSemaphoreRelease(semaphoreSensorsHandle);
}

void SelectFunction() {
	currFunction++;
	if (currFunction > ((sizeof(functions) / sizeof(UserSet_s)) - 1)) currFunction = 0;
	ShowFunctions();
}

void ChangeFunction(int8_t increment) {
	if (functions[currFunction].Min != 0 || functions[currFunction].Max != 0) {
		if (currFunction == 0) *functions[currFunction].Value += (increment * 30);
		else *functions[currFunction].Value += increment;
		if (*functions[currFunction].Value < functions[currFunction].Min) *functions[currFunction].Value = functions[currFunction].Max;
		if (*functions[currFunction].Value > functions[currFunction].Max) *functions[currFunction].Value = functions[currFunction].Min;
	}
//	WriteSetting(parameters[currParameter].EepromAddress, parameters[currParameter].Data, 1);
	ShowFunctions();
}

void ShowTimeset() {
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 1, 0, 0, &timeset[currTimeset].DigitsName);
	char s[8];
	sprintf(s," %02d", *timeset[currTimeset].Value);
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 2, 0, 0, s);
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "t.SEt");
}

void SelectTimeset() {
	currTimeset++;
	if (currTimeset > ((sizeof(timeset) / sizeof(Timeset_s)) - 1)) currTimeset = 0;
	ShowTimeset();
}

void ChangeTimeset(int8_t increment) {
	uint8_t index = *timeset[1].Value - 1;
	uint8_t daysMax = monthDays[(int)index];
	if (currTimeset == 2) timeset[currTimeset].Max = daysMax;
	*timeset[currTimeset].Value += increment;
	if (*timeset[currTimeset].Value < timeset[currTimeset].Min) *timeset[currTimeset].Value = timeset[currTimeset].Max;
	if (*timeset[currTimeset].Value > timeset[currTimeset].Max) *timeset[currTimeset].Value = timeset[currTimeset].Min;
	//	if (*timeset[currTimeset].Value < timeset[currTimeset].Min) *timeset[currTimeset].Value = timeset[currTimeset].Min;
	//	if (*timeset[currTimeset].Value > timeset[currTimeset].Max) *timeset[currTimeset].Value = timeset[currTimeset].Max;
//	WriteSetting(parameters[currParameter].EepromAddress, parameters[currParameter].Data, 1);
	ShowTimeset();
}

uint8_t GetDryTime(uint8_t class, uint8_t sterilTemp, uint8_t vacuumNumber) {

	uint8_t dryTime = 0;

	if (class != 0) {
		if (sterilTemp == STERILTEMP_121) {
			switch (class) {
			case 1:
				dryTime = 18;
				break;
			case 2:
				dryTime = 23;
				break;
			case 3:
				dryTime = 28;
				break;
			}
		} else if (vacuumNumber >= 3) {
			switch (class) {
			case 1:
				dryTime = 9;
				break;
			case 2:
				dryTime = 14;
				break;
			case 3:
				dryTime = 19;
				break;
			}

		} else {
			switch (class) {
			case 1:
				dryTime = 5;
				break;
			case 2:
				dryTime = 10;
				break;
			case 3:
				dryTime = 15;
				break;
			}
		}
	}

	return dryTime;
}

