/*==============================================================================

Filename: actuators.h

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


#ifndef INC_ACTUATORS_H_
#define INC_ACTUATORS_H_

#include "globals.h"
#include "settings.h"

extern TIM_HandleTypeDef htim2;
#define PWM_TIMER_HEATER_CHAMBER htim2
#define PWM_TIMER_HEATER_CHAMBER_PERIOD 7200

extern TIM_HandleTypeDef htim3;
#define PWM_TIMER_STEAM_GENERATOR htim3
#define PWM_TIMER_STEAM_GENERATOR_PERIOD 7200

#define DOOR_SOLENOID_IMPULSE 1000 	// Solenoid impulse duration (milliseconds)

void ChamberHeaterSet(e_PowerState status);
void SteamGeneratorHeaterSet(e_PowerState status);

void PumpWaterLoadSet(e_PowerState status);
void PumpVacuumSet(e_PowerState status);
void PumpSteamGeneratorSet(e_PowerState status);

void ValveVacuumSet(e_PowerState status);
void ValveDryIntakeSet(e_PowerState status);
void ValveDrainSteamSet(e_PowerState status);
void ValveWaterSteamSet(e_PowerState status);

void FanSet(e_PowerState status);

e_LockState DoorKeydSet(e_LockState status);

void ActuatorsSafeStatus();


#endif /* INC_ACTUATORS_H_ */
