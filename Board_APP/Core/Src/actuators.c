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

void Beep(uint8_t hits, uint16_t onDelay, uint16_t offDelay) {
	for (uint8_t i = 0; i < hits; i++) {
		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, ON);
		osDelay(onDelay);
		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, OFF);
		osDelay(onDelay);
	}
}

void HeaterChamberSet(e_PowerState status) {HAL_GPIO_WritePin(HEATER_RING_GPIO_Port, HEATER_RING_Pin, status);}
void HeaterSteamGeneratorSet(e_PowerState status) {HAL_GPIO_WritePin(HEATER_STEAM_GEN_GPIO_Port, HEATER_STEAM_GEN_Pin, status);}

void PunpWaterLoadSet(e_PowerState status) {HAL_GPIO_WritePin(PUMP_WATER_LOAD_GPIO_Port, PUMP_WATER_LOAD_Pin, status);}
void PumpVacuumSet(e_PowerState status) { HAL_GPIO_WritePin(PUMP_VACUUM_GPIO_Port, PUMP_VACUUM_Pin, status);}
void PunpSteamGeneratorSet(e_PowerState status) {HAL_GPIO_WritePin(PUMP_STEAM_GEN_GPIO_Port, PUMP_STEAM_GEN_Pin, status);}

void ValveVacuumSet(e_PowerState status) {HAL_GPIO_WritePin(VAL_VACUUM_GPIO_Port, VAL_VACUUM_Pin, status);}
void ValveDryIntakeSet(e_PowerState status) {HAL_GPIO_WritePin(VAL_DRY_AIR_GPIO_Port, VAL_DRY_AIR_Pin, status);}
void ValveDrainSteamSet(e_PowerState status) {HAL_GPIO_WritePin(VAL_DRAIN_STEAM_GPIO_Port, VAL_DRAIN_STEAM_Pin, status);}
void ValveWaterSteamSet(e_PowerState status) {HAL_GPIO_WritePin(VAL_WATER_STEAM_GPIO_Port, VAL_WATER_STEAM_Pin, status);}

void FanSet(e_PowerState status) {HAL_GPIO_WritePin(FAN_GPIO_Port, FAN_Pin, status);}

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

	osDelay(*(int16_t*)GetSetting("DOOR_SOLENOID_IMPULSE"));

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

