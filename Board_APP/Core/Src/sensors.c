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

================================================================================
PURPOSE: Modbus CRC16 calculation
USAGE:
==============================================================================*/

#include "sensors.h"

#include <math.h>

#include "globals.h"
#include "cmsis_os.h"
#include "TM7705.h"
#include "system.h"
#include "serial.h"
#include "settings.h"
#include "Inc/DB_protocol.h"


//==============================================================================
// DEFINES
//==============================================================================

#define SENSOR_TEMP_STEAM_GENERATOR_ADC_CHANNEL	0 // Internal ADC
#define SENSOR_TEMPL_CHAMBER_HEATER_ADC_CHANNE	1 // Internal ADC
#define SENSOR_PRESS_CHAMBER_ADC_CHANNEL		2 // Internal ADC
#define SENSOR_CURRENT_ADC_CHANNEL				3 // Internal ADC

#define SENSOR_TEMP_T1_ADC_CHANNEL 				0 // External 16bit ADC
#define SENSOR_TEMP_T2_ADC_CHANNEL 				1 // External 16bit ADC

#define SAMPLE_RATE					2			// Sample per second (SPS/Hz)
#define AVERAGE_SAMPLES				5			// N. of samples for the average
#define AVERAGE_SAMPLES_INTERVAL	10			// Interval between samples (milliseconds)
#define V_REF						3.3			// Power supply
#define ADC_BITS					12			// ADC resolution (8bit=256, 10bit=1024, 12bit=4096, 16bit=65536
//#define PTC_NOMINAL_RESISTANCE	1000	   	// Ω nominal resistance (typical @25°C)
//#define PTC_NOMINAL_TEMPERATURE	0.0			// temperature of nominal resistance (typical 25°C)
//#define PTC_BETA				3977		// The beta coefficient or the B value of the thermistor (usually 3000-4000) check the datasheet for the accurate value
#define R_REF 						1500		// Value of  resistor used for the voltage divider
#define RTD_A 						3.9083e-3
#define RTD_B 						-5.775e-7

//==============================================================================
// VARS
//==============================================================================

extern ADC_HandleTypeDef hadc1;
extern SPI_HandleTypeDef hspi1;
extern Settings_s settings;

const float Z1 = -RTD_A;
const float Z2 = RTD_A * RTD_A - (4 * RTD_B);
const float Z3 = (4 * RTD_B) / 1000;
const float Z4 = 2 * RTD_B;

static uint16_t ReadADC(uint32_t channel);
static float ADCToTemp(uint16_t adcValue);
static float ADCToPress(uint16_t adcValue);
static float ADCToCurrent(uint16_t adcValue);

s_Sensors Sensors;

#define PRESS_BUFFER_SIZE 5 // >= 2
float pressBuf[PRESS_BUFFER_SIZE];
float pressAvg;


//==============================================================================
// DECLARATIONS
//==============================================================================

e_LockState DoorSolenoidKeyGet() {
	return (HAL_GPIO_ReadPin(SW_SOLENOID_GPIO_Port, SW_SOLENOID_Pin) == GPIO_PIN_SET) ? LOCK : UNLOCK;
}

e_OpenClosedState DoorPositionGet() {
	return (HAL_GPIO_ReadPin(SW_DOOR_GPIO_Port, SW_DOOR_Pin) == GPIO_PIN_SET) ? CLOSED : OPEN;
}

e_Level WaterDumpLevelGet() {
	return (HAL_GPIO_ReadPin(SW_LSH_GPIO_Port, SW_LSH_Pin) == GPIO_PIN_SET) ? NORMAL : FULL;
}

e_Level WaterReservoirLevelGet() {
	uint8_t level = NORMAL;
	if (HAL_GPIO_ReadPin(SW_CSH_GPIO_Port, SW_CSH_Pin) == GPIO_PIN_SET) level = FULL;
	if (HAL_GPIO_ReadPin(SW_CSL_GPIO_Port, SW_CSL_Pin) == GPIO_PIN_SET) level = EMPTY;
	return level;
}

float SteamGeneratorTempGet() {
	return ADCToTemp(ReadADC(SENSOR_TEMP_STEAM_GENERATOR_ADC_CHANNEL));
}

float ChamberHeaterTempGet() {
	return ADCToTemp(ReadADC(SENSOR_TEMPL_CHAMBER_HEATER_ADC_CHANNE));
}

float ChamberPressureGet() {
	return ADCToPress(ReadADC(SENSOR_PRESS_CHAMBER_ADC_CHANNEL));
}

float CurrentGet() {
	return ADCToCurrent(ReadADC(SENSOR_CURRENT_ADC_CHANNEL));
}

//uint16_t TemperatureT1Get() {return TM7705_GetValue(SENSOR_TEMP_T1_ADC_CHANNEL);}
//uint16_t TemperatureT2Get() {return TM7705_GetValue(SENSOR_TEMP_T2_ADC_CHANNEL);}

//==============================================================================
// DEFINITIONS
//==============================================================================

float BufferAverage(float* buffer, uint8_t size, float newValue) {

	float retAvg = 0;

	// Shift values right
	for (int i = size - 1; i > 0; i--) {
		buffer[i] = buffer[i - 1];
	}

	// Leftmost add new read
	buffer[0] = newValue;

	// Calc average
	for (int i = 0; i < size; i++) {
		retAvg += buffer[i];
	}
	retAvg = retAvg / size;

	return retAvg;

}

void StartSensorsTask() {

	HAL_ADCEx_Calibration_Start(&hadc1);

	TM7705_Init(&hspi1);

	// Fill buffer values
	for (int i = 0 ; i < (sizeof(pressBuf) / sizeof(float)); i++) {
		pressBuf[i] = ChamberPressureGet();
	}

	for (;;) {


		Sensors.Door = DoorSolenoidKeyGet();
		Sensors.Solenoid = DoorPositionGet();
		Sensors.TemperatureSteamGenerator = SteamGeneratorTempGet();
		Sensors.TemperatureChamberHeater = ChamberHeaterTempGet();
		Sensors.T1 = TM7705_GetTemp(0);
//		Sensors.T2 = TemperatureT2Get();
		Sensors.PressureChamber = BufferAverage(pressBuf, PRESS_BUFFER_SIZE, ChamberPressureGet());
		Sensors.Current = CurrentGet();

//
//		Log(LOG_LEVEL_DEBUG, LOG_TIME_TIME, LOG_OUT_SERIAL, "%0.3f", Sensors.Current);
//		char d[8] = {0};
//		sprintf(d, "%0.1f", Sensors.TemperatureSteamGenerator);
//		SendCommand(COMMAND_WRITE_DIGITS_ROW_1, 0, 0, 0, d);

		char s[8];
		sprintf(s,"%5.1f", Sensors.PressureChamber);
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 2, 0, 0, s);
		osDelay(20);
		sprintf(s,"%5.1f", Sensors.T1);
		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 1, 0, 0, s);
		osDelay(20);
//		sprintf(s,"%5.1f", Sensors.TemperatureSteamGenerator);
//		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, s);

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

float ADCToTemp(uint16_t adcValue) {

	float readVolt, Rt, rRef, vRef, tCelsius;

	vRef = (float)V_REF;
	rRef = (float)R_REF;

	readVolt = (vRef / (float)(pow(2, ADC_BITS) - 1) * (float)adcValue); 	// Voltage read
	Rt = ((vRef - readVolt) * rRef / readVolt); // PTC ohm

	tCelsius = Z2 + (Z3 * Rt);
	tCelsius = (sqrt(tCelsius) + Z1) / Z4;

	tCelsius += (settings.T1Offset / 10);

	return tCelsius;
}

float ADCToPress(uint16_t adcValue) {

	float readVolt, vRef, pkPa;

	vRef = (float)V_REF;

	readVolt = (vRef / (float)(pow(2, ADC_BITS) - 1) * (float)adcValue); 	// Voltage read

	// Transfer function: Vout = Vs*(0.0012858*P+0.04) @Vs=5.0V
	//pkPa = (readVolt - (0,04 * vRefSensor)) / (0.0012858 * vRefSensor);

	pkPa = readVolt;
	pkPa = pkPa / 4.99;
	pkPa = pkPa - 0.04;
	pkPa = pkPa / 0.0012858;

	pkPa += settings.ChamberPressOffset;

	return pkPa;
}

float ADCToCurrent(uint16_t adcValue) {

	float readVolt, vRef, pkPa;

	const float VCC   = 4.9;// supply voltage 5V or 3.3V. If using PCB, set to 5V only.

	float cutOffLimit = 1.00;// reading cutt off current. 1.00 is 1 Amper

	/*
	          "ACS758LCB-050B",// for model use 0
	          "ACS758LCB-050U",// for model use 1
	          "ACS758LCB-100B",// for model use 2
	          "ACS758LCB-100U",// for model use 3
	          "ACS758KCB-150B",// for model use 4
	          "ACS758KCB-150U",// for model use 5
	          "ACS758ECB-200B",// for model use 6
	          "ACS758ECB-200U"// for model use  7
	sensitivity array is holding the sensitivy of the  ACS758
	current sensors. Do not change.
	*/
	const int model = 0;   // enter the model (see below)

	float sensitivity[] ={
	          40.0,// for ACS758LCB-050B
	          60.0,// for ACS758LCB-050U
	          20.0,// for ACS758LCB-100B
	          40.0,// for ACS758LCB-100U
	          13.3,// for ACS758KCB-150B
	          16.7,// for ACS758KCB-150U
	          10.0,// for ACS758ECB-200B
	          20.0,// for ACS758ECB-200U
	         };

	/*
	 *   quiescent Output voltage is factor for VCC that appears at output
	 *   when the current is zero.
	 *   for Bidirectional sensor it is 0.5 x VCC
	 *   for Unidirectional sensor it is 0.12 x VCC
	 *   for model ACS758LCB-050B, the B at the end represents Bidirectional (polarity doesn't matter)
	 *   for model ACS758LCB-100U, the U at the end represents Unidirectional (polarity must match)
	 *    Do not change.
	 */
	float quiescent_Output_voltage [] ={
	          0.5,// for ACS758LCB-050B
	          0.12,// for ACS758LCB-050U
	          0.5,// for ACS758LCB-100B
	          0.12,// for ACS758LCB-100U
	          0.5,// for ACS758KCB-150B
	          0.12,// for ACS758KCB-150U
	          0.5,// for ACS758ECB-200B
	          0.12,// for ACS758ECB-200U
	          };

	const float FACTOR = sensitivity[model]/1000;// set sensitivity for selected model
	const float QOV =   quiescent_Output_voltage [model] * VCC;// set quiescent Output voltage for selected model
	float voltage;// internal variable for voltage
	float cutOff = FACTOR/cutOffLimit;// convert current cut off to mV





	vRef = (float)V_REF;

	readVolt = (vRef / (float)(pow(2, ADC_BITS) - 1) * (float)adcValue); 	// Voltage read

	voltage =  readVolt - QOV + 0.007 ;// 0.007 is a value to make voltage zero when there is no current
	float current = voltage / FACTOR;

	return readVolt;
}


