/*==============================================================================

Filename: actuators.c

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

#include "actuators.h"

#include "cmsis_os.h"
#include "globals.h"
#include "settings.h"
#include "pid.h"
#include "sensors.h"
#include "cmsis_os.h"

/* Controller parameters */
#define PID_KP  7.0f
#define PID_KI  11.0f
#define PID_KD  0.9f

#define PID_TAU 0.02f

#define PID_LIM_MIN  0.0f
#define PID_LIM_MAX  100.0f

#define PID_LIM_MIN_INT  0.0f
#define PID_LIM_MAX_INT  50.0f

#define SAMPLE_TIME_S 1.0f

extern s_Sensors Sensors;
extern osSemaphoreId semaphorePowerHandle;
osThreadId chamberPowerTaskHandle;
osThreadId steamPowerTaskHandle;

float heaterChamberTempSet;
float heaterSteamGeneratorTempSet;

PIDController pidHeaterChamber = {PID_KP, PID_KI, PID_KD, PID_TAU, PID_LIM_MIN, PID_LIM_MAX, PID_LIM_MIN_INT, PID_LIM_MAX_INT, SAMPLE_TIME_S};
PIDController pidHeaterSteamGenerator = {PID_KP, PID_KI, PID_KD, PID_TAU, PID_LIM_MIN, PID_LIM_MAX, PID_LIM_MIN_INT, PID_LIM_MAX_INT, SAMPLE_TIME_S};
void StartChPowerTask();
void StartSteamPowerTask();

void HeaterChamberTempSet(uint8_t tempC) {

	if (tempC > 0) {
		PIDController_Init(&pidHeaterChamber);
		heaterChamberTempSet = tempC;
		PWM_TIMER_HEATER_CHAMBER.Instance->ARR = 7200;
		HAL_TIM_Base_Start_IT(&PWM_TIMER_HEATER_CHAMBER);
	} else {
		HAL_TIM_Base_Stop_IT(&PWM_TIMER_HEATER_CHAMBER);
		HAL_GPIO_WritePin(HEATER_RING_GPIO_Port, HEATER_RING_Pin, OFF);
	}
}

void PWMPower_Init() {

	osThreadDef(chPowerTask, StartChPowerTask, osPriorityNormal, 1, 128);
	chamberPowerTaskHandle = osThreadCreate(osThread(chPowerTask), NULL);

	osThreadDef(stPowerTask, StartSteamPowerTask, osPriorityNormal, 1, 128);
	steamPowerTaskHandle = osThreadCreate(osThread(stPowerTask), NULL);

}

void StartChPowerTask() {

	for (;;) {
		float ret = PIDController_Update(&pidHeaterChamber, (float)heaterChamberTempSet, Sensors.TemperatureChamberHeater);
		// duty cycle %
		uint16_t ut = 1000 * ret / 100;
		uint16_t dt = 1000 - ut;
		osSemaphoreWait(semaphorePowerHandle, osWaitForever);
		HAL_GPIO_WritePin(HEATER_RING_GPIO_Port, HEATER_RING_Pin, ON);
			osDelay(ut);
			osSemaphoreRelease(semaphorePowerHandle);
			HAL_GPIO_WritePin(HEATER_RING_GPIO_Port, HEATER_RING_Pin, OFF);
			osDelay(dt);

	}
}

void StartSteamPowerTask() {
	for (;;) {
		float ret = PIDController_Update(&pidHeaterSteamGenerator, (float)heaterSteamGeneratorTempSet, Sensors.TemperatureSteamGenerator);
		// duty cycle %
		uint16_t ut = 1000 * ret / 100;
		uint16_t dt = 1000 - ut;
		osSemaphoreWait(semaphorePowerHandle, osWaitForever);
		HAL_GPIO_WritePin(HEATER_STEAM_GEN_GPIO_Port, HEATER_STEAM_GEN_Pin, ON);
			osDelay(ut);
			osSemaphoreRelease(semaphorePowerHandle);
			HAL_GPIO_WritePin(HEATER_STEAM_GEN_GPIO_Port, HEATER_STEAM_GEN_Pin, OFF);
			osDelay(dt);

	}
}

//	uint16_t ut = PWM_TIMER_HEATER_CHAMBER_PERIOD * ret / 100;
//	uint16_t dt = PWM_TIMER_HEATER_CHAMBER_PERIOD - ut;

//	osSemaphoreWait(semaphorePowerHandle, osWaitForever);

//	if (HAL_GPIO_ReadPin(HEATER_RING_GPIO_Port, HEATER_RING_Pin)) {
//		HAL_GPIO_WritePin(HEATER_RING_GPIO_Port, HEATER_RING_Pin, GPIO_PIN_RESET);
//		PWM_TIMER_HEATER_CHAMBER.Instance->ARR = dt; // load new timer period counter for OFF duty cycle
//	} else {
//		HAL_GPIO_WritePin(HEATER_RING_GPIO_Port, HEATER_RING_Pin, GPIO_PIN_SET);
//		PWM_TIMER_HEATER_CHAMBER.Instance->ARR = ut; // load new timer period counter for ON duty cycle
//	}

//	osSemaphoreRelease(semaphorePowerHandle);


//		PWM_TIMER_HEATER_CHAMBER.Instance->ARR = 100;
//	HAL_TIM_Base_Start_IT(&PWM_TIMER_HEATER_CHAMBER); // restart timer


void HeaterSteamGeneratorTempSet(uint8_t tempC) {

	if (tempC > 0) {
		PIDController_Init(&pidHeaterSteamGenerator);
		heaterSteamGeneratorTempSet = tempC;
		PWM_TIMER_STEAM_GENERATOR.Instance->ARR = 7200;
		HAL_GPIO_WritePin(HEATER_STEAM_GEN_GPIO_Port, HEATER_STEAM_GEN_Pin, GPIO_PIN_RESET);
		HAL_TIM_Base_Start_IT(&PWM_TIMER_STEAM_GENERATOR);
	} else {
		HAL_TIM_Base_Stop_IT(&PWM_TIMER_STEAM_GENERATOR);
		HAL_GPIO_WritePin(HEATER_STEAM_GEN_GPIO_Port, HEATER_STEAM_GEN_Pin, OFF);
	}
}

void PWMHeaterSteamGenerator() {

	float ret = PIDController_Update(&pidHeaterSteamGenerator, (float)heaterSteamGeneratorTempSet, Sensors.TemperatureSteamGenerator);
	// duty cycle %
	uint16_t ut = 1000 * ret / 100;
	uint16_t dt = 1000 - ut;
//	uint16_t ut = PWM_TIMER_STEAM_GENERATOR_PERIOD * ret / 100;
//	uint16_t dt = PWM_TIMER_STEAM_GENERATOR_PERIOD - ut;


//	if (HAL_GPIO_ReadPin(HEATER_STEAM_GEN_GPIO_Port, HEATER_STEAM_GEN_Pin)) {
//		HAL_GPIO_WritePin(HEATER_STEAM_GEN_GPIO_Port, HEATER_STEAM_GEN_Pin, GPIO_PIN_RESET);
//		PWM_TIMER_STEAM_GENERATOR.Instance->ARR = dt; // load new timer period counter for OFF duty cycle
//	} else {
//		HAL_GPIO_WritePin(HEATER_STEAM_GEN_GPIO_Port, HEATER_STEAM_GEN_Pin, GPIO_PIN_SET);
//		PWM_TIMER_STEAM_GENERATOR.Instance->ARR = ut; // load new timer period counter for ON duty cycle
//	}
//	HAL_TIM_Base_Start_IT(&PWM_TIMER_STEAM_GENERATOR); // restart timer
	HAL_GPIO_WritePin(HEATER_STEAM_GEN_GPIO_Port, HEATER_STEAM_GEN_Pin, GPIO_PIN_SET);
	osDelay(ut+1);
	HAL_GPIO_WritePin(HEATER_STEAM_GEN_GPIO_Port, HEATER_STEAM_GEN_Pin, GPIO_PIN_RESET);
	osDelay(dt+1);

	PWM_TIMER_STEAM_GENERATOR.Instance->ARR = 100;
	HAL_TIM_Base_Start_IT(&PWM_TIMER_STEAM_GENERATOR); // restart timer

}


void HeaterSteamGeneratorPWM(e_PowerState status) {
	HAL_GPIO_WritePin(HEATER_STEAM_GEN_GPIO_Port, HEATER_STEAM_GEN_Pin, status);
}

void PumpWaterLoadSet(e_PowerState status) {
	HAL_GPIO_WritePin(PUMP_WATER_LOAD_GPIO_Port, PUMP_WATER_LOAD_Pin, status);
}

void PumpVacuumSet(e_PowerState status) {
	HAL_GPIO_WritePin(PUMP_VACUUM_GPIO_Port, PUMP_VACUUM_Pin, status);
}

void PumpSteamGeneratorSet(e_PowerState status) {
	HAL_GPIO_WritePin(PUMP_STEAM_GEN_GPIO_Port, PUMP_STEAM_GEN_Pin, status);
}

void ValveVacuumSet(e_PowerState status) { // tre vie
	HAL_GPIO_WritePin(VAL_VACUUM_GPIO_Port, VAL_VACUUM_Pin, status);
}

void ValveDryIntakeSet(e_PowerState status) { // aria
	HAL_GPIO_WritePin(VAL_DRY_AIR_GPIO_Port, VAL_DRY_AIR_Pin, status);
}

void ValveDrainSteamSet(e_PowerState status) { // scarico
	HAL_GPIO_WritePin(VAL_DRAIN_STEAM_GPIO_Port, VAL_DRAIN_STEAM_Pin, status);
}

void ValveWaterSteamSet(e_PowerState status) { // acqua
	HAL_GPIO_WritePin(VAL_WATER_STEAM_GPIO_Port, VAL_WATER_STEAM_Pin, status);
}

void FanSet(e_PowerState status) {
	HAL_GPIO_WritePin(FAN_GPIO_Port, FAN_Pin, status);
}

e_LockState DoorSolenoidSet(e_LockState status) {
	// All off for safe polarity set
	HAL_GPIO_WritePin(SOLENOID_V1_GPIO_Port, SOLENOID_V1_Pin, OFF);
	HAL_GPIO_WritePin(SOLENOID_V2_GPIO_Port, SOLENOID_V2_Pin, OFF);

	// Reverse polarity for LOCK/UNLOCK
	if (status == LOCK) {
		HAL_GPIO_WritePin(SOLENOID_V2_GPIO_Port, SOLENOID_V2_Pin, ON);
	} else if (status == UNLOCK) {
		HAL_GPIO_WritePin(SOLENOID_V1_GPIO_Port, SOLENOID_V1_Pin, ON);
	}

	osDelay(DOOR_SOLENOID_IMPULSE);

	// All off afetr LOCK/UNLOCK impulse
	HAL_GPIO_WritePin(SOLENOID_V1_GPIO_Port, SOLENOID_V1_Pin, OFF);
	HAL_GPIO_WritePin(SOLENOID_V2_GPIO_Port, SOLENOID_V2_Pin, OFF);

	// Return solenoid switch status for position confirmation
	return HAL_GPIO_ReadPin(SW_SOLENOID_GPIO_Port, SW_SOLENOID_Pin);
}

void DoorSolenoidDisconnect() {
	// Power disconnect
	HAL_GPIO_WritePin(SOLENOID_V1_GPIO_Port, SOLENOID_V1_Pin, OFF);
	HAL_GPIO_WritePin(SOLENOID_V2_GPIO_Port, SOLENOID_V2_Pin, OFF);
}

void ActuatorsSafeStatus() {
	HeaterChamberSet(OFF);
	HeaterSteamGeneratorSet(OFF);

	PunpWaterLoadSet(OFF);
	PumpVacuumSet(OFF);
	PunpSteamGeneratorSet(OFF);

	ValveVacuumSet(OFF);
	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveWaterSteamSet(OFF);

	FanSet(OFF);

	DoorSolenoidDisconnect();
}








