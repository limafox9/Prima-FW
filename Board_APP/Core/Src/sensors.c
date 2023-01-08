/*==============================================================================

Filename: sensors.c

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


#include "sensors.h"

#include <math.h>

#include "globals.h"
#include "cmsis_os.h"

#define SENSOR_TEMP_ADC_CHANNEL_STEAM_GENERATOR 0
#define SENSOR_TEMP_ADC_CHANNEL_CHAMBER_HEATER 1

#define SAMPLE_RATE					5			// Sample per second (SPS/Hz)
#define AVERAGE_SAMPLES				5			// N. of samples for the average
#define AVERAGE_SAMPLES_INTERVAL	5			// Interval between samples (milliseconds)
#define V_REF						3.3			// Power supply
#define ADC_BITS					12			// ADC resolution (8bit=256, 10bit=1024, 12bit=4096, 16bit=65536
//#define PTC_NOMINAL_RESISTANCE	1000	   	// Ω nominal resistance (typical @25°C)
//#define PTC_NOMINAL_TEMPERATURE	0.0			// temperature of nominal resistance (typical 25°C)
//#define PTC_BETA				3977		// The beta coefficient or the B value of the thermistor (usually 3000-4000) check the datasheet for the accurate value
#define R_REF 						1500		// Value of  resistor used for the voltage divider
#define RTD_A 						3.9083e-3
#define RTD_B 						-5.775e-7

extern ADC_HandleTypeDef hadc1;

const float Z1 = -RTD_A;
const float Z2 = RTD_A * RTD_A - (4 * RTD_B);
const float Z3 = (4 * RTD_B) / 1000;
const float Z4 = 2 * RTD_B;

struct {
	e_OpenClosedState Door;
	e_LockState Solenoid;
	float TemperatureSteamGenerator;
	float TemperatureChamberHeater;
	float T1;
	float T2;
	float PressureChamber;
} Sensors;

static uint16_t ReadADC(uint32_t channel);
static float ADCToTemp(float adcValue);

osMutexId mutexSensors;
osMutexDef(mutexSensors);

//==============================================================================

e_LockState DoorSolenoidPositionGet() {return HAL_GPIO_ReadPin(SW_SOLENOID_GPIO_Port, SW_SOLENOID_Pin);}
e_OpenClosedState DoorPositionGet() {return HAL_GPIO_ReadPin(SW_DOOR_GPIO_Port, SW_DOOR_Pin);}

float TemperatureSteamGeneratorGet() {return ADCToTemp((float)ReadADC(SENSOR_TEMP_ADC_CHANNEL_STEAM_GENERATOR));}
float TemperatureChamberHeaterGet() {return ADCToTemp((float)ReadADC(SENSOR_TEMP_ADC_CHANNEL_CHAMBER_HEATER));}

void StartSensorsTask() {

	mutexSensors = osMutexCreate(osMutex(mutexSensors));

	HAL_ADCEx_Calibration_Start(&hadc1);

	for (;;) {
		osMutexWait(mutexSensors, osWaitForever);

		Sensors.Door = DoorSolenoidPositionGet();
		Sensors.Solenoid = DoorPositionGet();
		Sensors.TemperatureSteamGenerator = TemperatureSteamGeneratorGet();
		Sensors.TemperatureChamberHeater = TemperatureChamberHeaterGet();

		osMutexRelease(mutexSensors);
		osDelay(1000 / SAMPLE_RATE);
	}
}

uint16_t ReadADC(uint32_t channel) {

	uint16_t adcRead = 0;
	float averageTotal = 0;

	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = channel;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5; // Maximum sampling time (239.5 cycles) divided by ADCClock (Mhz)
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}

	for (int i = 0; i < AVERAGE_SAMPLES; i++) {
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 5);
		adcRead = HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);
		averageTotal += adcRead;
		osDelay(AVERAGE_SAMPLES_INTERVAL);
	}

	return averageTotal / AVERAGE_SAMPLES;

}

float ADCToTemp(float adcValue) {

	float readVolt, Rt, tCelsius;

	readVolt = ((float)V_REF / pow(2, ADC_BITS) * adcValue); 	// Voltage read
	Rt = ((V_REF - readVolt) * R_REF / readVolt); // PTC ohm

	tCelsius = Z2 + (Z3 * Rt);
	tCelsius = (sqrt(tCelsius) + Z1) / Z4;

	return tCelsius;

}


