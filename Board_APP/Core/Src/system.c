/*==============================================================================

Filename: system.c

Created on: Dec 17, 2022

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

#include <24Cxx.h>
#include <phases.h>
#include "system.h"
#include "main.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "globals.h"
#include "serial.h"
#include "cmsis_os.h"
#include "settings.h"
#include "time.h"
#include "actuators.h"
#include "keys.h"
#include "sensors.h"
#include "serial.h"
#include "TM7705.h"
#include "BAT2U.h"
#include "BMP280.h"
#include "Inc/DB_Protocol.h"
#include "sensors.h"
#include "phases.h"
#include "printer.h"
#include "programs.h"
#include "modes.h"
#include "display.h"
#include "settings.h"
#include "AD7705.h"
#include "usbkey.h"
#include "usbh_conf.h"
#include "usb_host.h"

//#define USE_RTOS_SYSTICK

//#define RTD_A 3.9083e-3
//#define RTD_B -5.775e-7

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern SPI_HandleTypeDef hspi1;
extern ADC_HandleTypeDef hadc1;

extern Sensors_s sensors;
extern Settings_s settings;


extern USBH_HandleTypeDef hUsbHostFS;

//uint8_t UART_rxBuffer[UART_DISPLAY_BUFFER_SIZE] = {0};

RTCDateTime_s dt;

RTCDateTime_s RTCDateTime;

osThreadId keysTaskHandle;
osThreadId serialTaskHandle;
osThreadId sensorsTaskHandle;
osThreadId programTaskHandle;
osThreadId displayInfoTaskHandle;

osThreadId usbTaskHandle;

osSemaphoreId semaphoreSensorsHandle;
osSemaphoreId semaphoreSerialHandle;
xQueueHandle usbQueue;
xQueueHandle serialQueue;

//float calTemp(float Rt) {
//	float Z1, Z2, Z3, Z4, temp;
//
//	Z1 = -RTD_A;
//	Z2 = RTD_A * RTD_A - (4 * RTD_B);
//	Z3 = (4 * RTD_B) / 1000;
//	Z4 = 2 * RTD_B;
//
//	temp = Z2 + (Z3 * Rt);
//	temp = (sqrt(temp) + Z1) / Z4;
//
//	return temp;
//}

void SysInit() {


//	settings.T1Offset = 200;
//	settings.ChamberPressOffset = 200;
//	SaveSettings();

//	LoadSettings();

//	osDelay(1000);
//
//	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 1, 0, 0, "8.8.:8.8");
////	osDelay(10);
//	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 2, 0, 0, "8.8.:8.8");
////	osDelay(10);
//	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "8.8.:8.8");

//	SerialInit(&huart1, 115200);


//	MX_DriverVbusFS(0);
//	HAL_Delay(1000);
//	MX_DriverVbusFS(1);
//	HAL_Delay(1000);
//	MX_DriverVbusFS(0);
//	HAL_Delay(1000);

//	  MX_FATFS_Init();
//	  MX_USB_HOST_Init();

//	while (1) {
//		TestPressNorm();
//
//	}

	  osSemaphoreDef(semaphoreSensors);
	  semaphoreSensorsHandle = osSemaphoreCreate(osSemaphore(semaphoreSensors), 1);
	  osSemaphoreRelease(semaphoreSensorsHandle);
	  osSemaphoreDef(semaphoreSerial);
	  semaphoreSerialHandle = osSemaphoreCreate(osSemaphore(semaphoreSerial), 1);
	  osSemaphoreRelease(semaphoreSerialHandle);


	  usbQueue = xQueueCreate(50, USB_BUFFER_LENGHT);
	  serialQueue = xQueueCreate(5, sizeof(struct __s_DisplayProtocolData));



//		osDelay(500);


	SerialInit(&UART_DISPLAY, 115200);



	  osThreadDef(serialTask, StartSerialTask, osPriorityHigh, 1, 1024);
		serialTaskHandle = osThreadCreate(osThread(serialTask), NULL);

	machine.DisplayFWVersionMajor = 0;
	machine.DisplayFWVersionMinor = 0;
	machine.DisplayFWVersionRevision = 0;

	osDelay(500);

	do {
		SendCommand(DISPLAY_CMD_FW_VER, 0, 0, 0, "");
		osDelay(50);
	} while (machine.DisplayFWVersionMajor == 0 && machine.DisplayFWVersionMinor == 0 && machine.DisplayFWVersionRevision == 0);

	DigitsWrite(1, "8.8.8.8");
	DigitsWrite(2, "8.8.8.8");
	DigitsWrite(3, "8.8.:8.8");
	DashLed(0xFFFF, ON);
	ProgressLed(0xFFFF);

	osDelay(1000);

	LoadSettings();

	ClockInit(&hi2c1);
	PWMPower_Init();
	PrinterInit();

//	PrintLogo();
//	return;

//
	HAL_ADCEx_Calibration_Start(&hadc1);

//	TM7705_Init(&hspi1);
	TM7705_Init();
	TM7705_SytemCalibFull(1);
	TM7705_SytemCalibZero(1);
	TM7705_CalibSelf(1);		// this takes about 180ms
	HAL_Delay(200);
	TM7705_SytemCalibFull(2);
	TM7705_SytemCalibZero(2);
	TM7705_CalibSelf(2);		// this takes about 180ms
	HAL_Delay(200);

//	while (1) {
//		TM7705_GetTemp(1, 0.0);
//		TM7705_GetTemp(2, 0.0);
////		TM7705_ReadAdc(1);
////		TM7705_ReadAdc(2);
////		AD7705_GetTemp(CHANNEL_0, 0.0);
////		AD7705_GetTemp(CHANNEL_1, 0.0);
//		osDelay(100);
//	}



	BAT2U_Init(&huart2);
	sensors.WaterTDS = WaterQualityGet();



	DigitsWrite(1, "");
	DigitsWrite(2, "");
	DigitsWrite(3, "");
	DashLed(0xFFFF, OFF);
	ProgressLed(0x0000);



	char s[8];
	int16_t nextClean = settings.CyclesCount - (settings.CleanCyclesInterval + settings.LastCleanCycle);
	int16_t nextService = settings.CyclesCount - (settings.ServiceCyclesInterval + settings.LastServiceCycle);
//	sprintf(s, "%1d.%1d.%02d", FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION);
//	DigitsWrite(1, "Fir.");
	sprintf(s,"%4d", nextService);
	DigitsWrite (1, s);
	sprintf(s, "%1d.%1d.%02d", machine.FWVersionMajor, machine.FWVersionMinor, machine.FWVersionRevision);
	DigitsWrite (2, s);
	sprintf(s, "%1d.%1d.%02d", machine.DisplayFWVersionMajor, machine.DisplayFWVersionMinor, machine.DisplayFWVersionRevision);
	DigitsWrite (3, s);

	HAL_Delay(2000);


	if (nextClean >= 0) {
		sprintf(s,"%4d", nextClean);
		DigitsWrite (1, s);
		DigitsWrite (2, "CLEA");
		DigitsWrite (3, "----");
		Beep(3, 1000, 100);
	} else if (nextService >= 0) {
		sprintf(s,"%4d", nextService);
		DigitsWrite (1, s);
		DigitsWrite (2, "SErU");
		DigitsWrite (3, "----");
		Beep(3, 1000, 100);
	}

	HAL_Delay(2000);

	RTCDateTime_s RTCDateTime;
	GetDateTime(&RTCDateTime);
//
//		settings.T1Offset = 0;
//		settings.ChamberPressOffset = 0;
//		SaveSettings();

	osThreadDef(keysTask, StartKeysTask, osPriorityAboveNormal, 1, 512);
	keysTaskHandle = osThreadCreate(osThread(keysTask), NULL);

	osThreadDef(sensorsTask, StartSensorsTask, osPriorityNormal, 1, 512);
	sensorsTaskHandle = osThreadCreate(osThread(sensorsTask), NULL);

	osThreadDef(programTask, StartProgramTask, osPriorityNormal, 1, 1024);
	programTaskHandle = osThreadCreate(osThread(programTask), NULL);

	osThreadDef(displayInfoTask, StartDisplayInfoTask, osPriorityNormal, 1, 512);
	displayInfoTaskHandle = osThreadCreate(osThread(displayInfoTask), NULL);
	osThreadSuspend(displayInfoTaskHandle);

//	osThreadDef(usbTask, StartUSBTask, osPriorityNormal, 1, 4096);
//	usbTaskHandle = osThreadCreate(osThread(usbTask), NULL);



//		PrintError("ERROR ERROR\n");

		EEPROM_Read(EPA_LAST_ERROR, &lastError, 1);
//
//		osDelay(1000);
//
//		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 1, 0, 0, "8.8.:8.8");
//	//	osDelay(10);
//		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 2, 0, 0, "8.8.:8.8");
//	//	osDelay(10);
//		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, "8.8.:8.8");

		ProgramsInit();

		currProgram = 1;
		currProgType = PROGTYPE_PROGRAM;

//		lastError = 0;

		DisplaySetLongPressInterval(DISPLAY_LONG_PRESS_INTERVAL);

		sensors.Door = DoorPositionGet();
		machineStatus.Status = (sensors.Door == CLOSED) ? STATUS_READY : STATUS_IDLE;

//		lastDoorStatus = sensors.Door;
		doorOpened = (sensors.Door == CLOSED) ? 0 : 1;

		if (lastError == ERROR_NONE) {
			Beep(3, 100, 100);
		} else {
			machineStatus.Status = STATUS_ERROR;
			Beep(1, 1000, 100);
		}

		sensors.Solenoid == DoorSolenoidKeyGet();
		if (lastError == ERROR_NONE && sensors.Solenoid == LOCK) {
			if (DoorSolenoidSet(UNLOCK) != UNLOCK) {
				DigitsWrite(3, "Er.11");
				Beep(1, 1000, 100);
			}
		}
//		if (sensors.Door == CLOSED) DigitsWrite(3, "OPEn");

		osThreadDef(usbTask, StartUSBTask, osPriorityNormal, 1, 2048);
		usbTaskHandle = osThreadCreate(osThread(usbTask), NULL);

		if (settings.PreheatRingTemp > 120) settings.PreheatRingTemp = 120;
		if (settings.PreheatSteamTemp > 180) settings.PreheatSteamTemp = 180;
		ResetTimer(TIMER_NIGHT_CYCLE);
		PhasePreheat(settings.PreheatRingTemp, settings.PreheatSteamTemp);

//		settings.FunNightCycleTime = 2; //remove

//		PowerTest();
//		LoadUSBConfig();

//		for (int i = 0; i < 16; i++) {
//			ProgressLed(1<<i);
//			osDelay(500);
//		}
//				ProgressLed(0b0100000000000000);
//		SdcardWriteAction("test99.txt" , "Hello \r\n");

__NOP();
//		ProgressLed(LED_PROGRESS_HEATING);
//		osDelay(500);
//		ProgressLed(LED_PROGRESS_1STVACUUM);
//		osDelay(500);
//		ProgressLed(LED_PROGRESS_1STPRESSURE);
//		osDelay(500);
//		ProgressLed(LED_PROGRESS_NVACUUM);
//		osDelay(500);
//		ProgressLed(LED_PROGRESS_NPRESSURE);
//		osDelay(500);
//		ProgressLed(LED_PROGRESS_NRELEASE);
//		osDelay(500);
//		ProgressLed(LED_PROGRESS_STERILVACUUM);
//		osDelay(500);
//		ProgressLed(LED_PROGRESS_STERILPRESSURE);
//		osDelay(500);
//		ProgressLed(LED_PROGRESS_STERILMAINTAIN);
//		osDelay(500);
//		ProgressLed(LED_PROGRESS_STERILRELEASE);
//		osDelay(500);
//		ProgressLed(LED_PROGRESS_DRY);
//		osDelay(500);

//		ProgressLed(0xFF);
//		osDelay(500);


//		printf("CIAO");
//
//	PrintHeader("HELIX TEST", 24.2, &d);
//	PrintStepLine("VAC-----", 134.2, 2.1, 23.7, &a);
//	PrintStepLine("VACU-", 4.2, 21.1, 23.7, &a);

//	settings.T1Offset = 0;
//	settings.T2Offset = 0;
//	settings.ChamberPressOffset = 7;
//	SaveSettings();

//	PrintLogo();

//	PrintHeader("PROVA", 13.8, &d);
//	PrintTestPage();
//	PrintFooter("PASS", &d, &a);

//	int16_t cy = 334;
//	EEPROM_Write(EPA_CYCLE_COUNTER, cy, 2);

//	printf("CIAO");
//
//	SterilParam_s testParam = {
//			.chamberTemp = 50
//	};
//
//	PhaseTestPID(&testParam);
//
//	while (1) {
//		osDelay(100);
//	}

//	ProgramTestSteril();
//
//		while (1) {
//			osDelay(100);
//		}

//	while (1) {
//		HAL_UART_Transmit(&huart1, "12345678901234567890", 20, 100);
////		printf("123456789012345678901234567890");
//		osDelay(500);
//	}
//
//	printf("CIAO\n");

//	char s[8];
//	sprintf(s,"-b--");
//	SendCommand(DISPLAY_CMD_WRITE_DIGITS, 3, 0, 0, s);
//	osDelay(20);

//	__HAL_TIM_CLEAR_IT(&PWM_TIMER_HEATER_CHAMBER ,TIM_IT_UPDATE);
//	HAL_TIM_Base_Start_IT(&PWM_TIMER_HEATER_CHAMBER);

//Step_Preheat(40, 50, 180);

//	BMP280_HandleTypedef bmp280;
//	float pressure, temperature, humidity;
//	uint16_t size;
//	uint8_t Data[256];
//
//	bmp280_init_default_params(&bmp280.params);
//	bmp280.addr = BMP280_I2C_ADDRESS_0;
////	bmp280.i2c = &hi2c2;
//
//	while (!bmp280_init(&bmp280, &bmp280.params)) {
//			size = sprintf((char *)Data, "BMP280 initialization failed\n");
//			SerialSend(&UART_MONITOR, Data, size);
//			HAL_Delay(2000);
//	}
//	bool bme280p = bmp280.id == BME280_CHIP_ID;
//	size = sprintf((char *)Data, "BMP280: found %s\n", bme280p ? "BME280" : "BMP280");
//	SerialSend(&UART_MONITOR, Data, size);



//	BAT2U_Init(&UART_TDS);
//	char s[32];
//	int16_t refRes = BAT2U_GetTDSValue(1);
//	if (refRes < 0xFFFF) {
//		sprintf(s,"%d\n", refRes);
//		SerialSend(&UART_MONITOR, s, 32);
//	}


//	char* q = "CIAO";
//	s_DisplayProtocolData a = {DISPLAY_CMD_WRITE_DIGITS, 0x1234, 0x5678, 0x00, q, 1};
//	a.Command = DISPLAY_CMD_WRITE_DIGITS;
//	a.Param1 = 0x1234;
//	a.Param2 = 0x5678;
//	memcpy(a.Digits, q, 4);
//	uint8_t buffer[DISPLAY_PROTOCOL_BUFFER_SIZE];
//	Display_DataToProtocol(&a,buffer);
//	uint8_t pps = sizeof(buffer);



//	TM7705_Init(&hspi1);


//	HeaterChamberTempSet(130);
//	HeaterSteamGeneratorTempSet(200);


//	while (1) {
//		if ((Sensors.TemperatureChamberHeater >= 130) && (Sensors.TemperatureSteamGenerator >= 200)) break;
//	}


//	char printLine[32];
//	RTCDateTime_s clockTime;
//	RTCTimerTime_s timerTime;




//	ProgramWrapped();



//	PCF8563_GetDateTime(&clockTime);
//	Print("********************************\n");
//	PrintSize(2);
//	Print("HELIX TEST\n");
//	PrintSize(1);
//	Print("\n");
//	Print("START %d-%02d-%02d           %02d:%02d\n", clockTime.Year, clockTime.Month, clockTime.Day, clockTime.Hour, clockTime.Minute);
//	Print("--------------------------------\n");
//	Print("STEP    T1     T2   PRESS   TIME\n");
//	Print("        oC     oC    kPa   mm:ss\n");
//	Print("--------------------------------\n");
//	Print("VACU  134.8  134.6  104.2  02:34\n");

//	while(1) {
//		osDelay(100);
//	}




//	osDelay(5000);
//	HeaterChamberTempSet(0);
//	HeaterSteamGeneratorTempSet(0);
	//Beep(2, 100, 100);

//	uint16_t val1;
//
//	char s[8], p[8];
//	double res;
//	uint16_t refRes = 2000;
//	float tmp0, tmp1, press, val0;

//	ValveVacuumSet(ON);
//	PumpVacuumSet(ON);


//	while (1) {
//		val0 = TM7705_GetTemp(0);
//		press = ChamberPressureGet();
////		val1 = TM7705_GetValue(CHANNEL_1);
//
////		res = ((double)val0) * refRes / 65535;
////		tmp0 = calTemp(res);
//
////		res = ((double)val1) * refRes / 65535;
////		tmp1 = calTemp(res);
//
////		sprintf(s,"%d - %0.2f - %0.2f\n", val0, tmp0, tmp0);
////		SerialSend(&UART_MONITOR, s, 32);
//		sprintf(s,"%5.1f", val0);
//		sprintf(p,"%5.1f", press);
//		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 1, 0, 0, s);
//		osDelay(100);
//		SendCommand(DISPLAY_CMD_WRITE_DIGITS, 2, 0, 0, p);
//		osDelay(500);
//	}

	//	SerialSend(&UART_MONITOR, "MAMMA\n", 6);

//
	//	uint16_t n = 0x5678;
	////	char a[6] = "ABCDEF";
	//	EEPROM_Write(2,0,&n,2);
//	EEPROM_Write(page, offset, data, size)3,0,a,4);
//
//	EEPROM_EraseAll();









	//===============================================================================
//	uint8_t n[32]= {[0 ... 31] = 0x08};
//	EEPROM_Write(2,&n,21);
//	float u = 3.14667;
//	EEPROM_Write_Float(12, u);
//
//
//	uint16_t pp = 0x4523;
//	EEPROM_Write(255,&pp,2);
//
//	SerialSend(&UART_MONITOR, "\n", 1);
//
//	EEPROM_EraseAll();
//
//	char s[10];
//	char r[8];
//	for (int p = 0; p<32; p++) {
//		EEPROM_Read(p*8,r,8);
//		for (int b = 0; b<8; b++) {
//			sprintf(s,"%02X ", r[b]);
//			SerialSend(&UART_MONITOR, s, 3);
//		}
//		SerialSend(&UART_MONITOR, "\n", 1);
//	}
//
//	char z[10];
//	float y;
//	sprintf(z,"%0.6f\n", EEPROM_Read_Float(12));
//	SerialSend(&UART_MONITOR, z, 10);
	//===============================================================================








//	EEPROM_Read(10, &r, 2);
//	sprintf(s,"%2X \n", r);
//	SerialSend(&UART_MONITOR, s, 4);

//	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "\n\n****************************************");
//	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "PRIMA Board APP v%d.%d.%d", FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_REVISION);
//
//	// Get MCU unique ID 96 bit
//	UID[0] = *(uint32_t*)(UID_BASE );
//	UID[1] = *(uint32_t*)(UID_BASE + 0x04);
//	UID[2] = *(uint32_t*)(UID_BASE + 0x08);
//	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "UID:    %08X %08X %08X", UID[0], UID[1], UID[2]);
//
//	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "SERIAL: %08X %08X %08X", UID[0], UID[1], UID[2]);
//
//	// Clock initialization and get system date/time
//	ClockInit(&hi2c1);
//	GetDateTime(&dt);
//	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "TIME:   %02d-%02d-%02d %02d:%02d\n", dt.Year, dt.Month, dt.Day, dt.Hour, dt.Minute);
//
//	Log(LOG_LEVEL_DEBUG, LOG_TIME_NONE, LOG_OUT_SERIAL, "Kernel Started");
//
//	// Set actuators in safe (disconnected) mode
//	ActuatorsSafeStatus();
//	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "Actuators initialized in safe mode");
//
//	// Create threads
//
//	Log(LOG_LEVEL_DEBUG, LOG_TIME_NONE, LOG_OUT_SERIAL, "Threads started");
//
//	// Display communication init and Start receiving DMA display UART
////	PanelSerialInit(&huart3);
////	HAL_UART_Receive_DMA(&huart3, UART_rxBuffer, UART_DISPLAY_BUFFER_SIZE);
////	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
//
//	Log(LOG_LEVEL_RELEASE, LOG_TIME_NONE, LOG_OUT_SERIAL, "System initialized\n");

}

//
//void Beep(uint8_t hits, uint16_t onDelay, uint16_t offDelay) {
//	for (uint8_t i = 0; i < hits; i++) {
//		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, ON);
//		osDelay(onDelay);
//		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, OFF);
//		osDelay(onDelay);
//	}
//}

void EmergencyStop() {
	ValveDrainSteamSet(OFF);
	ValveDryIntakeSet(OFF);
	ValveVacuumSet(OFF);
	ValveWaterSteamSet(OFF);

	PumpSteamGeneratorSet(OFF);
	PumpVacuumSet(OFF);
	PumpWaterLoadSet(OFF);

	HAL_GPIO_WritePin(SOLENOID_V1_GPIO_Port, SOLENOID_V1_Pin, OFF);
	HAL_GPIO_WritePin(SOLENOID_V2_GPIO_Port, SOLENOID_V2_Pin, OFF);

}

void Log(e_LogLevel logLevel, e_LogTimeStamp logTimeStamp, e_LogOutput logOutput, const char* format, ...) {

	if (*(int8_t*)GetSetting("LOG_ACTIVE") && logLevel >= *(int8_t*)GetSetting("LOG_ACTIVE_LEVEL")) {
		RTCDateTime_s r;
		RTCTimerTime_s t;
		char s[256] = {'\0'};
		char buffer[256];

		if ((logTimeStamp & LOG_TIME_ELAPSED_RESET) == LOG_TIME_ELAPSED_RESET) {
			ResetTimer(1);
		}
		if ((logTimeStamp & LOG_TIME_DATE) == LOG_TIME_DATE) {
			GetDateTime(&r);
			sprintf(buffer, "%02d-%02d-%02d ", r.Year, r.Month, r.Day);
			strcat(s, buffer);
		}
		if ((logTimeStamp & LOG_TIME_TIME) == LOG_TIME_TIME) {
			GetDateTime(&r);
			sprintf(buffer, "%02d:%02d:%02d ", r.Hour, r.Minute, r.Second);
			strcat(s, buffer);
		}
		if ((logTimeStamp & LOG_TIME_ELAPSED) == LOG_TIME_ELAPSED) {
			GetTimer(1, &t);
			sprintf(buffer, "%3d:%02d:%02d ", t.Hours, t.TimeMinute, t.TimeSecond);
			strcat(s, buffer);
		}

		va_list args;
		va_start (args, format);
		vsprintf (buffer,format, args);
		va_end (args);
		strcat(s, buffer);

//		HAL_UART_Transmit(&huart1, (uint8_t*)s, strlen(s), 100);
//		HAL_UART_Transmit(&huart1, (uint8_t*)"\n", 1, 100);

	}
}


