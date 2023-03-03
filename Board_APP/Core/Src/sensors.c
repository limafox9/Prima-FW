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
#include "modes.h"
#include "display.h"
#include "7705.h"


//==============================================================================
// DEFINES
//==============================================================================

#define SENSOR_TEMP_STEAM_GENERATOR_ADC_CHANNEL	0 // Internal ADC
#define SENSOR_TEMPL_CHAMBER_HEATER_ADC_CHANNE	1 // Internal ADC
#define SENSOR_PRESS_CHAMBER_ADC_CHANNEL		2 // Internal ADC
#define SENSOR_CURRENT_ADC_CHANNEL				3 // Internal ADC

#define SENSOR_TEMP_T1_ADC_CHANNEL 				0 // External 16bit ADC
#define SENSOR_TEMP_T2_ADC_CHANNEL 				1 // External 16bit ADC

#define AMBIENT_PRESSURE_TOLERANCE_MIN			5	// +/- kPa to consider chamber pressure = ambient pressure
#define AMBIENT_PRESSURE_TOLERANCE_MAX 			20	// +/- kPa to consider chamber pressure = ambient pressure

#define ERROR_BEEP_NUMBER			1

#define SAMPLE_RATE					2			// Sample per second (SPS/Hz)
#define AVERAGE_SAMPLES				5			// N. of samples for the average
#define AVERAGE_SAMPLES_INTERVAL	10			// Interval between samples (milliseconds)
#define V_REF						3.3			// Power supply
#define ADC_BITS					12			// ADC resolution (8bit=256, 10bit=1024, 12bit=4096, 16bit=65536
#define T_FACTORY_OFFSET			2			// Fixed factory offset for T1 and T2
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
extern UART_HandleTypeDef huart2;
extern Settings_s settings;

extern uint8_t currProgram;
extern osSemaphoreId semaphoreSensorsHandle;

const float Z1 = -RTD_A;
const float Z2 = RTD_A * RTD_A - (4 * RTD_B);
const float Z3 = (4 * RTD_B) / 1000;
const float Z4 = 2 * RTD_B;

static uint16_t ReadADC(uint32_t channel);
static float ADCToTemp(uint16_t adcValue);
static float ADCToPress(uint16_t adcValue);
static float ADCToCurrent(uint16_t adcValue);

Sensors_s sensors;

float pressAmbientMin;
float pressAmbientMax;

#define PRESS_BUFFER_SIZE 5 // >= 2
float pressBuf[PRESS_BUFFER_SIZE];
float pressAvg;

float BufferAverage(float* buffer, uint8_t size, float newValue);

//==============================================================================
// DECLARATIONS
//==============================================================================

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

//	osSemaphoreWait(semaphoreSensorsHandle, 100);

	if(GPIO_Pin == SW_DOOR_Pin) {
		Beep(1, 100, 100);
		if ((machineStatus.Status == STATUS_READY) || (machineStatus.Status == STATUS_IDLE)) machineStatus.Status = (sensors.Door == CLOSED) ? STATUS_READY : STATUS_IDLE;
		if (machineStatus.Status == STATUS_IDLE) PhasePreheat(60.0, 90.0);
	}
//	if (GPIO_Pin == SW_CSH_Pin) {
//		osDelay(100);
//		if ((WaterReservoirLevelGet() & FULL) == FULL) Beep(3, 100, 100);
//	}
//	if (GPIO_Pin == SW_LSH_Pin) {
////		if (HAL_GPIO_ReadPin(SW_LSH_GPIO_Port, SW_LSH_Pin) == 0) {
//		osDelay(100);
//		if (WaterDumpLevelGet() == FULL) {
//			DashLed(DISPLAY_LED_WARNING_LSH, ON);
//			Beep(10, 1000, 500);
//		} else {
//			DashLed(DISPLAY_LED_WARNING_LSH, OFF);
//			Beep(1, 100, 100);
//		}
//	}
//	if (GPIO_Pin == SW_CSL_Pin) {
////		if (HAL_GPIO_ReadPin(SW_CSL_GPIO_Port, SW_CSL_Pin) == 0) {
//		osDelay(100);
//		if ((WaterReservoirLevelGet() & EMPTY) == EMPTY) {
//			DashLed(DISPLAY_LED_WARNING_CTL, ON);
//			Beep(10, 1000, 500);
//		} else {
//			DashLed(DISPLAY_LED_WARNING_CTL, OFF);
//			Beep(1, 100, 100);
//		}
//	}

//	osSemaphoreRelease(semaphoreSensorsHandle);
}

void StartSensorsTask() {

	static uint8_t prevLSH = NORMAL;
	static uint8_t prevCSL = NORMAL;
	static uint8_t prevCSH = NORMAL;
	static uint8_t prevTDS = OFF;
	// Fill buffer values
	for (int i = 0 ; i < (sizeof(pressBuf) / sizeof(float)); i++) {
		pressBuf[i] = ChamberPressureGet();
	}

	sensors.PressureAmbient = (float)100.0;

	for (;;) {
		osSemaphoreWait(semaphoreSensorsHandle, 100);
		sensors.Door = DoorPositionGet();
		sensors.Solenoid = DoorSolenoidKeyGet();
		sensors.TemperatureSteamGenerator = SteamGeneratorTempGet();
		sensors.TemperatureChamberHeater = ChamberHeaterTempGet();
		sensors.T1 = TM7705_GetTemp(1, (float)(settings.T1Offset / 10.0) + (float)T_FACTORY_OFFSET);
//		sensors.T2 = sensors.T1;
		sensors.T2 = TM7705_GetTemp(2, (float)(settings.T2Offset / 10.0) + (float)T_FACTORY_OFFSET);
		sensors.PressureChamber = BufferAverage(pressBuf, PRESS_BUFFER_SIZE, ChamberPressureGet());
		sensors.Current = CurrentGet();
		sensors.WaterDumpLevel = WaterDumpLevelGet();
		sensors.WaterReservoirLevel = WaterReservoirLevelGet();

//		Log(LOG_LEVEL_DEBUG, LOG_TIME_TIME, LOG_OUT_SERIAL, "%0.3f", Sensors.Current);
//		char d[8] = {0};
//		sprintf(d, "%0.1f", Sensors.TemperatureSteamGenerator);
//		SendCommand(COMMAND_WRITE_DIGITS_ROW_1, 0, 0, 0, d);

		if ((machineStatus.Status == STATUS_READY) || (machineStatus.Status == STATUS_IDLE)) machineStatus.Status = (sensors.Door == CLOSED) ? STATUS_READY : STATUS_IDLE;

		if (sensors.Door == OPEN) {
			sensors.PressureAmbient = sensors.PressureChamber;
		}

		pressAmbientMin = sensors.PressureAmbient - (float)AMBIENT_PRESSURE_TOLERANCE_MIN;
		pressAmbientMax = sensors.PressureAmbient + (float)AMBIENT_PRESSURE_TOLERANCE_MAX;

		if (machineStatus.Show == SHOW_PROGRAM) ShowProgram();
		else if (machineStatus.Show == SHOW_READINGS) ShowReading();
		else if (machineStatus.Show == SHOW_PARAMETERS) ShowParameter();

		if (sensors.WaterDumpLevel == FULL) {
			if (prevLSH == NORMAL) {
				DashLed(DISPLAY_LED_WARNING_LSH, ON);
				prevLSH = FULL;
				Beep(ERROR_BEEP_NUMBER, 1000, 500);
			}
		} else {
			if (prevLSH == FULL) {
				DashLed(DISPLAY_LED_WARNING_LSH, OFF);
				prevLSH = NORMAL;
				Beep(1, 100, 100);
			}
		}

		if (sensors.WaterReservoirLevel == EMPTY) {
			if ((prevCSL == NORMAL) || (prevCSL == FULL)) {
				DashLed(DISPLAY_LED_WARNING_CTL, ON);
				prevCSL = EMPTY;
				Beep(ERROR_BEEP_NUMBER, 1000, 500);
			}
		} else {
			if (prevCSL == EMPTY) {
				DashLed(DISPLAY_LED_WARNING_CTL, OFF);
				prevCSL = NORMAL;
//				sensors.WaterTDS = BAT2U_GetTDSValue(1, 100);
				Beep(1, 100, 100);
			}
		}

		if (sensors.WaterReservoirLevel == FULL) {
			if (prevCSH == NORMAL) {
				prevCSH = FULL;
				Beep(3, 100, 100);
			}
		} else {
			if (prevCSH == FULL) {
				prevCSH = NORMAL;
				Beep(1, 100, 100);
			}
		}

		if (sensors.WaterTDS > settings.WaterTDSMin) {
			if (prevTDS == OFF) {
				DashLed(DISPLAY_LED_WARNING_TDS, ON);
				prevTDS = ON;
				Beep(ERROR_BEEP_NUMBER, 1000, 500);
			}
		} else {
			if (prevTDS == ON) {
				DashLed(DISPLAY_LED_WARNING_TDS, OFF);
				prevTDS = OFF;
			}
		}

//		if (sensors.WaterReservoirLevel == EMPTY) {
//			SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, DISPLAY_LED_WARNING_CTL, ON, 0, "");
//		} else {
//			SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, DISPLAY_LED_WARNING_CTL, OFF, 0, "");
//		}

//		if (sensors.WaterDumpLevel == FULL) {
//			SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, DISPLAY_LED_WARNING_LSH, ON, 0, "");
//		} else {
//			SendCommand(DISPLAY_CMD_WRITE_DASHBOARD_LEDS, DISPLAY_LED_WARNING_LSH, OFF, 0, "");
//		}
		osSemaphoreRelease(semaphoreSensorsHandle);
		osDelay(1000 / SAMPLE_RATE);
	}
}

e_LockState DoorSolenoidKeyGet() {
	return (HAL_GPIO_ReadPin(SW_SOLENOID_GPIO_Port, SW_SOLENOID_Pin) == GPIO_PIN_SET) ? UNLOCK : LOCK;
}

e_OpenClosedState DoorPositionGet() {
	return (HAL_GPIO_ReadPin(SW_DOOR_GPIO_Port, SW_DOOR_Pin) == GPIO_PIN_SET) ? OPEN : CLOSED;
}

e_Level WaterDumpLevelGet() {
	return (HAL_GPIO_ReadPin(SW_LSH_GPIO_Port, SW_LSH_Pin) == GPIO_PIN_SET) ? FULL : NORMAL;
}

e_Level WaterReservoirLevelGet() {
	uint8_t level = NORMAL;
	uint8_t lvlH, lvlL;
	lvlH = HAL_GPIO_ReadPin(SW_CSH_GPIO_Port, SW_CSH_Pin);
	lvlL = HAL_GPIO_ReadPin(SW_CSL_GPIO_Port, SW_CSL_Pin);
	if (lvlH == LOW) level = FULL;
	if (lvlL == LOW) level = EMPTY;
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

	if (averageTotal < 1000) averageTotal = 5000;

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

	tCelsius += (settings.T1Offset / 10.0);

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


