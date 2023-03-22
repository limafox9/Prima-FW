/*==============================================================================

Filename: keys.c

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
PURPOSE:
USAGE:
================================================================================*/

#include "keys.h"
#include "system.h"
#include "cmsis_os.h"
#include "serial.h"
#include "Inc/DB_Protocol.h"
#include "programs.h"
#include "modes.h"
#include "settings.h"
#include "cmsis_os.h"
#include "usbkey.h"
#include "sensors.h"

#define KEY_LONG_PRESS_MS 3000

uint8_t _keysBeepActive = 0;

extern osThreadId programTaskHandle;

static uint8_t GetPressedKeys();
static void KeysReceived(uint8_t keysCode);

// ==============================================================

void ParseCommand(s_DisplayProtocolData* data) {

	if (data->Command == DISPLAY_CMD_KEYS_PRESSED) {
		Beep(1, 100, 100);
		if (data->Param1 == KEY_PROG) {
			if ((machineStatus.Status == STATUS_IDLE) && (machineStatus.Show == SHOW_PROGRAM)) {
				SelectProgram();
			} else if ((machineStatus.Status == STATUS_READY) && (machineStatus.Show == SHOW_PROGRAM)) {
				SelectProgram();
			} else if (machineStatus.Status == STATUS_RUNNING) {
				machineStatus.Show = SHOW_READINGS;
			}

			if (machineStatus.Show == SHOW_PARAMETERS) {
				SelectParameter();
			} else if (machineStatus.Show == SHOW_READINGS) {
				SelectReading();
			} else if (machineStatus.Show == SHOW_USERSET) {
				SelectUserSetting();
			} else if (machineStatus.Show == SHOW_FUNCTIONS) {
				SelectFunction();
			} else if (machineStatus.Show == SHOW_TIMESET) {
				SelectTimeset();
			} else {
				__NOP();
			}

		} else if (data->Param1 == KEY_TEST) {
			if ((machineStatus.Status == STATUS_IDLE) && (machineStatus.Show == SHOW_PROGRAM)) {
				SelectTest();
			} else if ((machineStatus.Status == STATUS_READY) && (machineStatus.Show == SHOW_PROGRAM)) {
				SelectTest();
			} else if (machineStatus.Status == STATUS_RUNNING) {
				machineStatus.Show = SHOW_PROGRAM;
				ShowProgram();
			}

			if (machineStatus.Show == SHOW_PARAMETERS) {
				ChangeParameter(1);
			} else if (machineStatus.Show == SHOW_TIMESET) {
				ChangeTimeset(1);
			} else if (machineStatus.Show == SHOW_USERSET) {
				ChangeUserSetting(1);
			} else if (machineStatus.Show == SHOW_FUNCTIONS) {
				ChangeFunction(1);
			} else {
				__NOP();
			}

		} else if (data->Param1 == KEY_START) {
			if (machineStatus.Status == STATUS_ERROR) {
				currProgram = PROGRAM_RESUME_DRY;
//				machineStatus.Status == STATUS_READY;
//				machineStatus.Show = SHOW_PROGRAM;
				programStart = 1;
			} else {
				if (progRunning == 1 && currTest == 3) abortProgram = 1;
				if (machineStatus.Show == SHOW_PROGRAM) {
					if ((machineStatus.Status == STATUS_READY) || (machineStatus.Status == STATUS_IDLE)) {
						programStart = 1;
					}
				} else if (machineStatus.Show == SHOW_PARAMETERS) {
					ChangeParameter(-1);
				} else if (machineStatus.Show == SHOW_TIMESET) {
					ChangeTimeset(-1);
				} else if (machineStatus.Show == SHOW_USERSET) {
					ChangeUserSetting(-1);
				} else if (machineStatus.Show == SHOW_FUNCTIONS) {
						ChangeFunction(-1);
				} else {
					__NOP();
				}
			}

		} else if (data->Param1 == KEY_PROG + KEY_LONG_PRESS) {
//			if ((machineStatus.Status == STATUS_IDLE) || (machineStatus.Status == STATUS_IDLE)) {
//				if (machineStatus.Show == SHOW_USERSET) {
//					machineStatus.Show = SHOW_PROGRAM;
//					SaveSettings();
//					ShowProgram();
//				} else if (machineStatus.Show == SHOW_PROGRAM){
//					machineStatus.Show = SHOW_USERSET;
//					ShowUserSetting();
//					return;
//				}
//			}

			if (machineStatus.Show == SHOW_PROGRAM) {
				if (machineStatus.Status == STATUS_IDLE) {
					machineStatus.Show = SHOW_USERSET;
					ShowUserSetting();
				} else if (machineStatus.Status == STATUS_READY) {
					machineStatus.Show = SHOW_USERSET;
					ShowUserSetting();
				}
			} else if (machineStatus.Show == SHOW_PARAMETERS) {
				machineStatus.Show = SHOW_PROGRAM;
				SaveSettings();
				ShowProgram();
			} else if (machineStatus.Show == SHOW_USERSET) {
				machineStatus.Show = SHOW_PROGRAM;
				SaveSettings();
				ShowProgram();
			} else if (machineStatus.Show == SHOW_FUNCTIONS) {
				machineStatus.Show = SHOW_PROGRAM;
				SaveSettings();
				ShowProgram();
			} else if (machineStatus.Show == SHOW_READINGS) {
				machineStatus.Show = SHOW_PROGRAM;
				ShowProgram();
			} else if (machineStatus.Show == SHOW_TIMESET) {
				RTCDateTime.Year += 2000;
				PCF8563_SetDateTime(&RTCDateTime);
				machineStatus.Show = SHOW_PROGRAM;
				ShowProgram();
			} else {
				__NOP();
			}

		} else if (data->Param1 == KEY_TEST + KEY_LONG_PRESS) {
//			if ((machineStatus.Status == STATUS_IDLE) || (machineStatus.Status == STATUS_IDLE)) {
//				if (machineStatus.Show == SHOW_FUNCTIONS) {
//					machineStatus.Show = SHOW_PROGRAM;
//					ShowProgram();
//				} else {
//					machineStatus.Show = SHOW_FUNCTIONS;
//					ShowFunctions();
//					return;
//				}
//			}

			if (machineStatus.Show == SHOW_PROGRAM) {
				if (machineStatus.Status == STATUS_IDLE) {
					machineStatus.Show = SHOW_FUNCTIONS;
					ShowFunctions();
				} else if (machineStatus.Status == STATUS_READY) {
					machineStatus.Show = SHOW_FUNCTIONS;
					ShowFunctions();
				}
			} else if (machineStatus.Show == SHOW_USERSET) {
				machineStatus.Show = SHOW_PROGRAM;
				ShowProgram();
			} else if (machineStatus.Show == SHOW_PARAMETERS) {
				machineStatus.Show = SHOW_PROGRAM;
				ShowProgram();
			} else if (machineStatus.Show == SHOW_READINGS) {
				machineStatus.Show = SHOW_PROGRAM;
				ShowProgram();
			} else if (machineStatus.Show == SHOW_FUNCTIONS) {
				machineStatus.Show = SHOW_PROGRAM;
				ShowProgram();
			} else if (machineStatus.Show == SHOW_TIMESET) {
				machineStatus.Show = SHOW_PROGRAM;
				ShowProgram();
			} else {
				__NOP();
			}

		} else if (data->Param1 == KEY_START + KEY_LONG_PRESS) {
			if (currProgram == PROGRAM_RESUME_DRY ) {
				DashLedsOff();
				ProgressLedsOff();
				abortProgram = 1;
				programStart = 0;
				machineStatus.Status = STATUS_READY;
				machineStatus.Show = SHOW_PROGRAM;
				ShowProgram();
			} else {
				if (machineStatus.Show == SHOW_PROGRAM) {
					if (machineStatus.Status == STATUS_IDLE) {
						__NOP();
					} else if (machineStatus.Status == STATUS_READY) {
						__NOP();
					} else if (machineStatus.Status = STATUS_RUNNING) {
						DashLedsOff();
						ProgressLedsOff();
						abortProgram = 1;
						machineStatus.Status = STATUS_READY;
						machineStatus.Show = SHOW_PROGRAM;
						ShowProgram();
					} else {
						__NOP();
					}
				} else if (machineStatus.Show == SHOW_FUNCTIONS) {
					if (functions[currFunction].ParamNumber == 9) {
						LoadUSBConfig();
					} else if (functions[currFunction].ParamNumber == 10) {
						PowerTest();
					} else {
						__NOP();
					}
				} else {
					__NOP();
				}
			}

		} else if (data->Param1 == KEY_TEST + KEY_PROG) {
			if (machineStatus.Show == SHOW_PARAMETERS) {
				machineStatus.Show = SHOW_PROGRAM;
				ShowProgram();
			} else if (machineStatus.Show == SHOW_READINGS) {
				machineStatus.Show = SHOW_PROGRAM;
				ShowProgram();
			} else if (machineStatus.Show == SHOW_TIMESET) {
				machineStatus.Show = SHOW_PROGRAM;
				ShowProgram();
			} else if (machineStatus.Show == SHOW_FUNCTIONS) {

			} else {
				__NOP();
			}

		} else if (data->Param1 == KEY_START + KEY_PROG) {
			__NOP();
		} else if (data->Param1 == KEY_START + KEY_TEST) {
			__NOP();
		} else if (data->Param1 == KEY_START + KEY_TEST + KEY_PROG) {
			__NOP();
		} else if (data->Param1 == KEY_TEST + KEY_PROG + KEY_LONG_PRESS) {
			if (machineStatus.Show == SHOW_PROGRAM) {
				if ((machineStatus.Status == STATUS_IDLE) || (machineStatus.Status == STATUS_READY)) {
					machineStatus.Show = SHOW_TIMESET;
					PCF8563_GetDateTime(&RTCDateTime);
					RTCDateTime.Year -= 2000;
					ShowTimeset();
					__NOP();
				}
			}

		} else if (data->Param1 == KEY_START + KEY_TEST + KEY_LONG_PRESS) {
			if (machineStatus.Status == STATUS_IDLE) {
				machineStatus.Show = SHOW_PARAMETERS;
				ShowParameter();
			} else if (machineStatus.Status == STATUS_READY) {
				machineStatus.Show = SHOW_PARAMETERS;
				ShowParameter();
			} else {
				__NOP();
			}

		} else if (data->Param1 == KEY_START + KEY_PROG + KEY_LONG_PRESS) {
			if (machineStatus.Status == STATUS_IDLE) {
				machineStatus.Show = SHOW_READINGS;
				ShowReading();
			} else if (machineStatus.Status == STATUS_READY) {
				machineStatus.Show = SHOW_READINGS;
				ShowReading();
			} else {
				__NOP();
			}

		} else if (data->Param1 == KEY_START + KEY_TEST + KEY_PROG + KEY_LONG_PRESS) {
			__NOP();
		} else {
			__NOP();
		}
	} else if (data->Command == DISPLAY_CMD_OK) {
		machine.DisplayFWVersionMajor = (uint8_t)data->Param1;
		machine.DisplayFWVersionMinor = (uint8_t)data->Param2;
		machine.DisplayFWVersionRevision = (uint8_t)data->Param3;
	}
}

void StartKeysTask() {

	uint8_t pressKeys;
	uint32_t keyPressStart;

	for (;;) {
		pressKeys = GetPressedKeys();
		keyPressStart = HAL_GetTick();
		if (pressKeys != 0) {
			osDelay(50); // Debounce
			while (pressKeys == GetPressedKeys()) {
				if ((HAL_GetTick() - keyPressStart) > KEY_LONG_PRESS_MS) {
					pressKeys += KEY_LONG_PRESS;
					break;
				}
				osDelay(20);
			}
			KeysReceived(pressKeys);
		}
		osDelay(20);
	}
}

static uint8_t GetPressedKeys() {

	uint8_t pressKeys = 0;

	// Keys active low
	pressKeys = !(HAL_GPIO_ReadPin(KEY_S1_GPIO_Port, KEY_S1_Pin)); // bit 1
	pressKeys += !(HAL_GPIO_ReadPin(KEY_S2_GPIO_Port, KEY_S2_Pin)) << 1; // bit 2
	pressKeys += !(HAL_GPIO_ReadPin(KEY_S3_GPIO_Port, KEY_S3_Pin)) << 2; // bit 3

	return pressKeys;
}

static void KeysReceived(uint8_t keysCode) {

	if (keysCode == KEY_S1) {
		__NOP();
	} else if (keysCode == KEY_S2) {
////		SdcardWriteAction("CIAO.txt" , "Hello\n");
//		sprintf(currFilename, "Prova3.txt"); //remove
//		xQueueSend(usbQueue, "Hello2\n", 100); //remove
		__NOP();
	} else if (keysCode == KEY_S3) {
		__NOP();
	} else if (keysCode == KEY_S1 + KEY_LONG_PRESS) {
		EEPROM_Write(EPA_LAST_SERVICE_CYCLE, settings.CyclesCount, 2);
		__NOP();
	} else if (keysCode == KEY_S2 + KEY_LONG_PRESS) {
		__NOP();
	} else if (keysCode == KEY_S3 + KEY_LONG_PRESS) {
		__NOP();
	} else if (keysCode == KEY_S2 + KEY_S1) {
		__NOP();
	} else if (keysCode == KEY_S3 + KEY_S1) {
		__NOP();
	} else if (keysCode == KEY_S3 + KEY_S2) {
		__NOP();
	} else if (keysCode == KEY_S3 + KEY_S2 + KEY_S1) {
		__NOP();
	} else if (keysCode == KEY_S2 + KEY_S1 + KEY_LONG_PRESS) {
		uint16_t dum;
		BAT2U_TDSCalibration(500);
		dum = BAT2U_GetTDSValue(2, 200);
		dum = BAT2U_GetTDSValue(1, 200);
		sensors.WaterTDS = dum;
		Beep(1, 50, 50);
		__NOP();
	} else if (keysCode == KEY_S3 + KEY_S2 + KEY_LONG_PRESS) {
		machine.ReadOnlyCyclesCount = 0;
		EEPROM_Write(EPA_CYCLE_COUNTER, 0, 2);
		Beep(1, 50, 50);
		__NOP();
	} else if (keysCode == KEY_S3 + KEY_S1 + KEY_LONG_PRESS) {

		__NOP();
	} else if (keysCode == KEY_S3 + KEY_S2 + KEY_S1 + KEY_LONG_PRESS) {
		SaveDefaults(1);
		Beep(1, 50, 50);
		HAL_NVIC_SystemReset();
		__NOP();
	} else {
		__NOP();
	}
}



