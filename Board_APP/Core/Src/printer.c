/*==============================================================================

Filename: printer.c

Created on: Jan 27, 2023

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

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "printer.h"

#include "time.h"
#include "globals.h"
#include "settings.h"
#include "stm32f1xx_hal.h"
#include "sensors.h"
#include "modes.h"

#define DC2 0x12
#define ESC	0x1B
#define FS	0x1C
#define GS  0x1D

extern UART_HandleTypeDef UART_PRINTER;
extern Settings_s settings;
extern MachineData_s machine;

const unsigned char logo[] = {
	0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xF0,
	0xF0, 0xF0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xE0, 0xF0, 0xF0, 0xF0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xC0, 0xE0, 0xE0, 0xE0, 0xF0, 0xF0,
	0xF0, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xFC, 0x7C, 0x1C, 0x0C, 0x3C, 0x7C, 0xF8, 0xF8, 0xF8, 0xF8,
	0xF8, 0xF8, 0xF0, 0xF0, 0xF0, 0xE0, 0xE0, 0xE0, 0xC0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void PrinterInit() {
	char d[10];

	Print("%c%c", ESC, '@'); // Reset
//	Print("%c%c", ESC, '5'); // Charset 2
//	Print("%c%c", FS, '.'); // Disable Chinese Print Mode
//	Print("%c%c", ESC, 0x54); // Upside-down print
//	Print("%c%c%c", ESC, 0x57, 1); // Font size
	Print("%c%c%c", ESC, '!', 0b00000101);
//	Print("%c%c%c", ESC, '3', 10); // Line spacing (n*0.125mm, def. 30)
//	Print("%c%c%c", ESC, 'a', 0); // Justify (0 = left, 1 = center, 2 = right)
//	Print("%c%c%c",  GS, '!', 0x00);
//	Print("%c%c%c",  FS, '.', 2);
//	Print("%c%c%c",  ESC, 't', 2);
//	Print("%c%c%c%c%c%c%c%c%c", ESC, '&', 64, 0, 96, 144, 144, 96, 0);
}

void PrintSize(uint8_t width, uint8_t height) {
	Print("%c%c%c", ESC, 'U', width); // Font width
//	Print("%c%c%c", ESC, 'W', size); // Font size
	Print("%c%c%c", ESC, 'V', height); // Font height
}

//void Print(const char* data) {
//	HAL_UART_Transmit(&huart4, data, strlen(data), 1000);
//}

void PrintTestPage() {
	Print("%c%c", DC2, 'T'); // Reset
}

void PrintError(const char* errorString, RTCTimerTime_s* tTime) {
	Print(errorString);
	Print("----    T1     T2    kPa    TIME\n");
	Print("      %5.1f  %5.1f  %5.1f %3d:%02d\n", sensors.T1, sensors.T2, sensors.PressureChamber, tTime->TimeMinute, tTime->TimeSecond);
	Print("        T3     T4    TDS   DO  LO\n");
	Print("      %5.1f  %5.1f %5d   %s  %s\n", 	sensors.TemperatureChamberHeater,
												sensors.TemperatureSteamGenerator,
												sensors.WaterTDS,
												(sensors.Door == OPEN) ? "OP" : "CL",
												(sensors.Solenoid == LOCK) ? "LK" : "UN"
												);
	Print("\n");
	Print("\n");
	Print("\n");
	Print("\n");
}

void Print(const char* format, ...) {

		  va_list args;
		  char* buf[80];
		  va_start(args, format);
		  vsprintf(buf, format, args);
		  //printf(buf);
		  va_end(args);

		  HAL_UART_Transmit(&UART_PRINTER, buf, strlen(buf), 1000);
//		  HAL_UART_Transmit(&huart1, "\n", 1, 100);

}

void PrintLogo() {
//	uint8_t logo[] = {0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	uint16_t width[2] = {0xC8, 0x00};

	uint8_t cmd[4 + sizeof(logo)] = {ESC, 'K', width[0], width[1]};

	for (int i = 0; i < sizeof(logo); i++) {
		cmd[i + 4] = logo[i];
	}
//	Print("%c%c%c", 0x1b, 'W', 4);
	HAL_UART_Transmit(&UART_PRINTER, cmd, sizeof(cmd), 1000);
//	Print("%c%c%c", 0x1b, 'W', 1);
//	Print("%s", cmd);
//	Print("\n");
}

void PrintHeader (const char* title, float t1, RTCDateTime_s* time) {

	Print("\n");
	Print("******************************************\n");
//	Print("123456789012345678901234567890123456789012\n");
	PrintSize(2, 2);
//	Print("%c%c%c", ESC, 'a', 1); // Justify (0 = left, 1 = center, 2 = right)
	Print("%s\n", title);
//	Print("%c%c%c", ESC, 'a', 0); // Justify (0 = left, 1 = center, 2 = right)
	PrintSize(1, 1);
	if (currProgType == PROGTYPE_PROGRAM) {
		uint8_t tSter = (programs[currProgram].SterilTemp == STERILTEMP_134) ? 134 : 121;
		uint8_t minSter = programs[currProgram].SterilTime / 60;
		uint8_t secSter = programs[currProgram].SterilTime % 60;
		uint8_t minDry = programs[currProgram].DryTime / 60;
		uint8_t secDry = programs[currProgram].DryTime % 60;
		Print("VAC.N.  Temp.   Press.   Time   Dry\n");
		//       3     134'C   310kPa  10:34  10:34
		Print(" %2d     %3d'C   %3dkPa  %2d:%02d  %2d:%02d", settings.VacNumber, tSter, programs[currProgram].SterilPress, minSter, secSter, minDry, secDry);
		Print("\n");
	} else if ((currProgType == PROGTYPE_TEST) && (currTest == 1)) {
		uint8_t minSter = 210 / 60;
		uint8_t secSter = 210 % 60;
		uint8_t minDry = 300 / 60;
		uint8_t secDry = 300 % 60;
		Print("VAC.N.  Temp.   Press.   Time   Dry\n");
		//       3     134'C   310kPa  10:34  10:34
		Print(" %2d     %3d'C   %3dkPa  %2d:%02d  %2d:%02d", settings.VacNumber, 134, 310, minSter, secSter, minDry, secDry);
		Print("\n");
	} else if ((currProgType == PROGTYPE_PROGRAM) && (currProgram == PROGRAM_USER)) {
		uint8_t tSter = (settings.UserSterilTemp == STERILTEMP_134) ? 134 : 121;
		uint8_t minSter = settings.UserSterilTime;
		uint8_t secSter = 0;
		uint8_t minDry = settings.UserDryTime;
		uint8_t secDry = 0;
		Print("VAC.N.  Temp.   Press.   Time   Dry\n");
		//       3     134'C   310kPa  10:34  10:34
		Print(" %2d     %3d'C   %3dkPa  %2d:%02d  %2d:%02d", settings.UserVacuumNumber, tSter, (settings.UserSterilTemp == STERILTEMP_134) ? 310.0 : 210.0, minSter, secSter, minDry, secDry);
		Print("\n");
	}
	Print("------------------------------------------\n");
	Print("\n");
	Print("S/N:          %s\n", machine.Serial);
	Print("UID:          %08X %08X %08X\n", machine.MCUID[2], machine.MCUID[1], machine.MCUID[0]);
	Print("Firmware:     v.%d.%02d.%1d (%d.%02d.%1d)\n", machine.FWVersionMajor, machine.FWVersionMinor, machine.FWVersionRevision, machine.DisplayFWVersionMajor, machine.DisplayFWVersionMinor, machine.DisplayFWVersionRevision);
	Print("\n");
	Print("Date (Y-M-D): %d-%02d-%02d\n", time->Year, time->Month, time->Day);
	Print("CYCLE N.      %d\n", settings.CyclesCount);
	Print("\n");
	Print("ATM (kPa)     %0.1f\n", sensors.PressureAmbient);
	Print("\n");
	Print("\n");
	Print("USER:\n");
	Print("\n");
	Print("------------------------------------------\n");
	Print("STEP    T1     T2   PRESS   TIME\n");
	Print("        'C     'C    kPa   mm:ss\n");
	Print("------------------------------------------\n");
	Print("START %5.1f             %02d:%02d:%02d\n", t1, time->Hour, time->Minute, time->Second);
}

void PrintFooter (const char* result, RTCDateTime_s* time, RTCTimerTime_s* timer) {
	Print("FINISH                  %02d:%02d:%02d\n", time->Hour, time->Minute, time->Second);
	Print("------------------------------------------\n");
	Print("TOTAL TIME                 %2d:%02d\n", timer->TimeMinute, timer->TimeSecond);
	Print("\n");
	PrintSize(3, 2);
	if (lastError == 0)	Print("%s\n", result);
	else Print("%s Err.%0d\n", result, lastError);
	PrintSize(1, 1);
	Print("\n");
	if (lastError != 0) PrintError("", time);
	Print("\n");
	Print("\n");
	Print("INDICATOR STRIP: PASS [] FAIL []\n");
	Print("\n");
	Print("\n");
	Print("OPERATOR: ________________________________\n");
	Print("\n");
	Print("******************************************\n");
	Print("\n");
	Print("\n");
	Print("\n");
	Print("\n");
}

void PrintStepLine(const char* step, float t1, float t2, float pressure, RTCTimerTime_s* time) {
	Print("%.4s  %5.1f  %5.1f  %5.1f %3d:%02d\n", step, t1, t2, pressure, time->TimeMinute, time->TimeSecond);
}

void PrintDivisor() {
	char* s = "******************************************\n";
	Print(s);
}

void PrintLF(uint8_t lines) {
	char data[lines + 1];
	for (int i = 0; i < lines; i++) {
		data[i] = 0x1b;
	}
	data[lines] = 0x0;
	Print(data);
}
