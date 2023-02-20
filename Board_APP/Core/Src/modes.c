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

s_MachineStatus machineStatus = {STATUS_IDLE, SHOW_PROGRAM};

uint8_t currProgram = 0;
uint8_t currTest = 0;
e_ProgramType currProgType = PROGTYPE_PROGRAM;
uint8_t currParameter = 0;
uint8_t currReading = 0;
uint8_t currTimeset = 0;

uint8_t progRunning = 0;
uint8_t progStart = 0;
uint8_t progStop = 0;

s_Program programs[] = {
	{1, PROGTYPE_PROGRAM, "Wrapped", "P. Un", 1, CLASS_B, STERILTEMP_134, },
	{2, PROGTYPE_PROGRAM, "Unwrapped", "P. Ur", 2, CLASS_S, STERILTEMP_134},
	{3, PROGTYPE_PROGRAM, "Prion", "P. Pr", 4, CLASS_B, STERILTEMP_134},
	{4, PROGTYPE_PROGRAM, "Gentle", "P. GE", 8, CLASS_B, STERILTEMP_121},
	{5, PROGTYPE_PROGRAM, "User", "P. US", 15, CLASS_USER, STERILTEMP_USER},
};

s_Program tests[] = {
	{1, PROGTYPE_TEST, "Helix Test", "t. HE", 16, CLASS_USER, STERILTEMP_USER},
	{2, PROGTYPE_TEST, "Vacuum Test", "t. UA", 32, CLASS_USER, STERILTEMP_USER},
	{3, PROGTYPE_TEST, "Wash cycle", "UASH", DISPLAY_LED_WASH_CYCLE, CLASS_USER, STERILTEMP_USER},
	{4, PROGTYPE_TEST, "Water fill", "----", DISPLAY_LED_WATER_FILL, CLASS_USER, STERILTEMP_USER},
};

Parameter_s parameters[] = {
	{1, "T1 Offset", " t1 ", EPA_T1_OFFSET, &settings.T1Offset, 1, -20, 20, 10},
	{2, "T2 Offset", " t2 ", EPA_T2_OFFSET, &settings.T2Offset, 1, -20, 20, 10},
	{3, "P1 Offset", " P1 ", EPA_CHAMBER_PRESSURE_OFFSET, &settings.ChamberPressOffset, 1, -15, 15, 1},
	{4, "First Vacuum", " PH1", EPA_FIRST_VACUUM, &settings.FirstVacuum, 1, 10, 50, 1},
	{5, "Other Vacuum", " PH ", EPA_OTHER_VACUUM, &settings.OtherVacuum, 1, 10, 60, 1},
	{6, "Vacuum Threshold", " PU ", EPA_VACUUM_THRESHOLD, &settings.VacuumThreshold, 1, 30, 60, 1},
	{7, "Other Pressure", "HIGH", EPA_OTHER_PRESSURE, &settings.OtherPressure, 1, 120, 200, 1},
	{8, "Min. TDS", " CO ", EPA_TDS_MIN, &settings.WaterTDSMin, 1, -20, 20, 1},
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

Timeset_s timeset[] = {
		{1, "Year", "yEAr", &RTCDateTime.Year, 22, 99},
		{2, "Month", "nnon", &RTCDateTime.Month, 1, 12},
		{3, "Day", " dAy", &RTCDateTime.Day, 1, 31},
		{4, "Hour", "Hour", &RTCDateTime.Hour, 0, 23},
		{5, "Minute", "nnin", &RTCDateTime.Minute, 0, 59},
};

const uint8_t monthDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void SelectProgram() {
	if (currProgType == PROGTYPE_PROGRAM) currProgram++;
	currProgType = PROGTYPE_PROGRAM;
	if (currProgram > ((sizeof(programs) / sizeof(s_Program)) - 1)) currProgram = 0;
//	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, &programs[currProgram].DigitsName);
////			SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, 255, OFF, 0, "");
//	SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, 63 + DISPLAY_LED_WASH_CYCLE + DISPLAY_LED_WATER_FILL, OFF, 0, "");
//	SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, programs[currProgram].DashLeds, ON, 0, "");
	ShowProgram();
}

void SelectTest() {
	if (currProgType == PROGTYPE_TEST) currTest++;
	currProgType = PROGTYPE_TEST;
	if (currTest > ((sizeof(tests) / sizeof(s_Program)) - 1)) currTest = 0;
//	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, &tests[currTest].DigitsName);
////			SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, 255, OFF, 0, "");
//	SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, tests[currTest].DashLeds, ON, 0, "");
	ShowProgram();
}

void ShowProgram() {

	if (lastError > 0) {
		char s[8];
		sprintf(s,"Er.%02d", lastError);
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, s);
	} else {
		if (machineStatus.Status != STATUS_RUNNING) {
			if (currProgType == PROGTYPE_PROGRAM) {
				SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, &programs[currProgram].DigitsName);
				SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, 63 + DISPLAY_LED_WASH_CYCLE + DISPLAY_LED_WATER_FILL, OFF, 0, "");
				SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, programs[currProgram].DashLeds, ON, 0, "");
			}
			if (currProgType == PROGTYPE_TEST) {
				SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, &tests[currTest].DigitsName);
				SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, 63 + DISPLAY_LED_WASH_CYCLE + DISPLAY_LED_WATER_FILL, OFF, 0, "");
				SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, tests[currTest].DashLeds, ON, 0, "");
			}
		}
	}

	char s[8];
	sprintf(s,"%5.1f", sensors.T1);
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 1, 0, 0, s);
	osDelay(20);
	sprintf(s,"%5.1f", sensors.PressureChamber);
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 2, 0, 0, s);
	osDelay(20);

}

void ShowParameter() {
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 1, 0, 0, &parameters[currParameter].DigitsName);
	char s[8];
	if (parameters[currParameter].Divider > 1) {
		sprintf(s,"%5.1f", (float)((float)*parameters[currParameter].Value / 10.0));
	} else {
		sprintf(s,"%4d", *parameters[currParameter].Value);
	}
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 2, 0, 0, s);
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "PArA");
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
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 1, 0, 0, &readings[currReading].DigitsName);
	char s[8];
	if (currReading == 6) {
		sprintf(s,"%4d", sensors.WaterTDS);
	} else {
		sprintf(s,"%5.1f", (float)(*readings[currReading].Value));
	}
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 2, 0, 0, s);
	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "rEAd");
}

void SelectReading() {
	currReading++;
	if (currReading > ((sizeof(readings) / sizeof(Reading_s)) - 1)) currReading = 0;
	ShowReading();
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


