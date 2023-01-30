/*==============================================================================

Filename: sensors.h

Created on: Dec 16, 2022

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
PURPOSE: Modbus CRC16 calculation
USAGE:
================================================================================*/

#ifndef INC_SENSORS_H_
#define INC_SENSORS_H_

#include "globals.h"

void StartSensorsTask();

e_LockState DoorSolenoidKeyGet();
e_OpenClosedState DoorPositionGet();
e_Level WaterDumpLevelGet();			// LSH
e_Level WaterReservoirLevelGet(); 	// CSH - CSL

float SteamGeneratorTempGet();
float ChamberHeaterTempGet();
float ChamberTempT1Get();
//float ChamberTempT2Get();
//float BoardTempGet();
//float CPUTempGet();
//float AmbientHumidityGet();

float AmbientPressureGet();
float ChamberPressureGet();

float CurrentGet();
uint16_t WaterQualityGet();

typedef struct s_Sensors{
	e_OpenClosedState Door;
	e_LockState Solenoid;
	float TemperatureSteamGenerator;
	float TemperatureChamberHeater;
	float T1;
	float T2;
	float PressureChamber;
	float Current;
	uint16_t WaterTDS;
} s_Sensors;


#endif /* INC_SENSORS_H_ */
