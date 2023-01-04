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
#include "globals.h"
#include "cmsis_os.h"

extern ADC_HandleTypeDef hadc1;

uint16_t ReadADC(uint32_t channel);
uint16_t GetTempZQF();

float adcToTemp(float VRT);
float adcToTempBeta(float adcRead);

void StartSensorsTask() {
	float Rt;
	HAL_ADCEx_Calibration_Start(&hadc1);

	for (;;) {

		Rt = adcToTempBeta((float)ReadADC(ADC_CHANNEL_0));
		Log(LOG_LEVEL_RELEASE, "%0.1f", Rt);

		osDelay(500);
	}

}

uint16_t ReadADC(uint32_t channel) {

	uint16_t adcRead = 0;

	ADC_ChannelConfTypeDef sConfig = {0};

	sConfig.Channel = channel;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 5);
	adcRead = HAL_ADC_GetValue(&hadc1);
	HAL_Delay(1);
	//HAL_ADC_Stop(&hadc1);

	return adcRead;

}

//Thermometer with thermistor

/*thermistor parameters:
 * RT0:   10 000 Ω
 * B: 3977 K +- 0.75%
 * T0:  25 C
 * +- 5%
 */

//These   values are in the datasheet
#define Vcc					3.3			// Power supply
#define resolution			4096		// ADC resolution (8bit=256, 10bit=1024, 12bit=4096, 16bit=65536
#define nominal_resistance	10000   	// Ω nominal resistance (typical @25°C)
#define nominal_temeprature	25.0		// temperature of nominal resistance (typical 25°C)
#define beta 				3977		// The beta coefficient or the B value of the thermistor (usually 3000-4000) check the datasheet for the accurate value
#define Rref 				1500		// Value of  resistor used for the voltage divider

float adcToTemp(float adcRead) {

	float readVolt, readR, temperature;

	int Vo;
	float R1 = 1500;
	float logR2, R2, tKelvin, tCelsius, tFahrenheit;
	float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

	readVolt = (((float)Vcc / ((float)resolution-1)) * adcRead); 	// Voltage read

	R2 = R1 * ((float)resolution / adcRead - 1.0); // resistance of the Thermistor
	R2 = R1 * ((float)resolution / adcRead - 1.0); // resistance of the Thermistor
	logR2 = log(R2);
	tKelvin = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
	tCelsius = tKelvin - 273.15;
	tFahrenheit = (tCelsius * 9.0) / 5.0 + 32.0;

//	readR = readVolt*(float)Rref/((float)Vcc - readVolt);							// Sensor resistance (R)
//	temperature = log(readR / nominal_resistance);				// ln(R/Ro)
//	temperature /= beta;                   // 1/B * ln(R/Ro)
//	temperature += 1.0 / (nominal_temeprature + 273.15); // + (1/To)
//	temperature = 1.0 / temperature;                 // Invert
//	temperature -= 273.15;                         // convert absolute temp to C


	return tCelsius;

}

float adcToTempBeta(float adcRead) {

	float readVolt, readR, temperature;

	int Vo; // Holds the ADC Value
	float R2, logR2, tKelvin, tCelsius, tFahrenheit;
	const float Beta     = 3974.0;
	const float roomTemp = 298.15;   // room temperature in Kelvin
	const float Ro = 10000.0; // Resistance of the thermistor at roomTemp
	const float R1 = 1500.0;  // Resistnce of the known resistor
	float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
	const float Vdd = 3.3;

	readVolt = (((float)Vdd / ((float)resolution)) * adcRead); 	// Voltage read
//	Vo = analogRead(thermistorPin);
	R2 = ((Vcc-readVolt)*R1/readVolt);
//	R2 = R1 * (4095.0 / adcRead - 1.0); // Resistance of the Thermistor
	  logR2 = log(R2);
	  tKelvin = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
	  tCelsius = tKelvin - 273.15;
	  tFahrenheit = (tCelsius * 9.0) / 5.0 + 32.0;

//	readR = readVolt*(float)Rref/((float)Vcc - readVolt);							// Sensor resistance (R)
//	temperature = log(readR / nominal_resistance);				// ln(R/Ro)
//	temperature /= beta;                   // 1/B * ln(R/Ro)
//	temperature += 1.0 / (nominal_temeprature + 273.15); // + (1/To)
//	temperature = 1.0 / temperature;                 // Invert
//	temperature -= 273.15;                         // convert absolute temp to C


	return tCelsius;

}

//float adcToTemp(float VRT) {
//	float RT, VR, ln, TX, T1;
//	T1 = 25.0 + 273.15;
//	VRT   = (3.3 / 4096.00) * VRT;      //Conversion to voltage
//	VR = VCC - VRT;
//	RT = VRT / (VR / R);               //Resistance of RT
//
//	ln = log(RT / RT0);
//	TX = (1 / ((ln / B) + (1 / T1))); //Temperature from thermistor
//
//	TX =   TX - 273.15;                 //Conversion to Celsius
//	return TX;
//
//}




