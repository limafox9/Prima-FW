/*==============================================================================

Filename: usbkey.c

Created on: Mar 7, 2023

Original work: Copyright (c) 2023 Luca Favenzi
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
PURPOSE:
USAGE:
==============================================================================*/

#include "usbkey.h"

#include "system.h"
#include "main.h"
#include "usbh_core.h"
#include <string.h>
#include <stdio.h>
#include "settings.h"


#define usbCS0 HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET)
#define usbCS1 HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_SET)

#define CONFIG_FILENAME	"PRIMA.CFG"
#define CONFIG_FILE_VERSION 1
#define CONFIG_FILE_MAX_ROW_LENGHT 49


FATFS fatfs;
FATFS *pfatfs;
// state Holders
uint8_t state_mount = 0;
uint8_t state_unmount = 0;
//
char currFilename[32];
char USBString[USB_BUFFER_LENGHT];
uint8_t USBWrite ;


#define MAX_BUFFER 32
char Buffer[MAX_BUFFER];

char *ltrim(char *s) {
    while(isspace(*s)) s++;
    return s;
}

char *rtrim(char *s) {
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s) {
    return rtrim(ltrim(s));
}

uint8_t isnumber(char* strcheck) {

	for (int i=0;i<strlen(strcheck); i++) {
		if (!isdigit(strcheck[i])) {
			return 0;
		}
	}
	return 1;
}

const char* getfield(char* line, int num)
{
    const char* tok;
    for (tok = strtok(line, ":"); tok && *tok; tok = strtok(NULL, ":\n")) {
        if (!--num) return trim(tok);
    }
    return NULL;
}

uint8_t GetParam(char* sRow, s_ConfigParameter* param) {

	char parameter[CONFIG_FILE_MAX_ROW_LENGHT];
	char value = 0;
	char line[CONFIG_FILE_MAX_ROW_LENGHT];
	uint8_t retVal = 0;

	memset(param->param, 0, sizeof(param->param));
	memset(param->value, 0, sizeof(param->value));

	if (strchr(sRow, ':') != NULL) {
		strcpy(param->param, strtok(sRow,":"));
		strcpy(param->value, strtok(NULL,":"));
		retVal = 1;
	}
//
//	strcpy(line, sRow);
//	strcpy(param->param, getfield(line, 1));
//	strcpy(line, sRow);
//	strcpy(param->value, getfield(line, 2));

//	strcpy(line, sRow);
//	if (getfield(line, 2) != NULL) {
//		param->value = atoi(parameter);
//		retVal = 1;
//	}

	return retVal;

//	parameter = strtok(sRow, ":");
//	if (strlen(trim(sRow)) != 0) {
//		param->param = trim(sRow);
//		if (isnumber(trim(parameter))) {
//			param->value = atoi(trim(param));
//			retVal = 1;
//		}
//	}
//
//	return retVal;
}

void StartUSBTask() {
//	SdcardWriteAction("CIAO.txt" , "Hello\n");
volatile FRESULT fr;

//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET);

//MX_FATFS_Init();
//MX_USB_HOST_Init();


//int i = 0;
//
//while(i < 1000)
//{
//	MX_USB_HOST_Process();
//	HAL_Delay(1);
//	i++;
//}
//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_SET);
//osDelay(1000);
//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET);
//osDelay(1000);

	for (;;) {
//		MX_USB_HOST_Process();
		if (xQueueReceive(usbQueue, USBString, portMAX_DELAY) == pdTRUE)	{
			if(f_mount(&USBHFatFS, "", 1) == FR_OK)	{
				if (f_open(&USBHFile, currFilename, FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {
					f_lseek(&USBHFile,USBHFile.fsize);
					f_printf(&USBHFile,USBString);
					f_close(&USBHFile);
				}
			}
		}
	}

}

void LoadUSBConfig()  {

	char rBuffer[CONFIG_FILE_MAX_ROW_LENGHT];
	s_ConfigParameter param;

	if(f_mount(&USBHFatFS, "", 1) == FR_OK)	{
		if (f_open(&USBHFile, CONFIG_FILENAME, FA_READ | FA_OPEN_EXISTING) == FR_OK) {
			if (f_eof(&USBHFile) == 0) {
				f_gets(rBuffer, CONFIG_FILE_MAX_ROW_LENGHT, &USBHFile);

				GetParam(rBuffer, &param);
				if (strcmp(param.param, "VERSION") == 0 && param.value >= CONFIG_FILE_VERSION) {
					while (f_eof(&USBHFile) == 0) {
						f_gets(rBuffer, CONFIG_FILE_MAX_ROW_LENGHT, &USBHFile);
						GetParam(rBuffer, &param);

						if (strcmp(param.param, "LAST_ERROR") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_LAST_ERROR, &bVal, 1);

						} else if (strcmp(param.param, "MACHINE_SERIAL_NUMBER") == 0) {
							WriteSetting(EPA_MACHINE_SERIAL_NUMBER, param.value, 12);

						} else if (strcmp(param.param, "MACHINE_MODEL") == 0) {
							WriteSetting(EPA_MACHINE_MODEL, param.value, 15);

						} else if (strcmp(param.param, "MACHINE_SIZE") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_MACHINE_SIZE, &bVal, 1);

						} else if (strcmp(param.param, "MACHINE_PROD_YEAR") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_MACHINE_PROD_YEAR, &bVal, 1);

						} else if (strcmp(param.param, "MACHINE_PROD_MONTH") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_MACHINE_PROD_MONTH, &bVal, 1);

						} else if (strcmp(param.param, "CYCLE_COUNTER") == 0) {
							uint16_t  bVal = (uint16_t)atoi(param.value);
							WriteSetting(EPA_CYCLE_COUNTER, &bVal, 2);

						} else if (strcmp(param.param, "CYCLE_COUNTER_PERMANENT") == 0) {
							uint16_t bVal = (uint16_t)atoi(param.value);
							WriteSetting(EPA_CYCLE_COUNTER_PERMANENT, &bVal, 2);

						} else if (strcmp(param.param, "T1_OFFSET") == 0) {
							int8_t bVal = (int8_t)atoi(param.value);
							WriteSetting(EPA_T1_OFFSET, &bVal, 1);

						} else if (strcmp(param.param, "T2_OFFSET") == 0) {
							int8_t bVal = (int8_t)atoi(param.value);
							WriteSetting(EPA_T2_OFFSET, &bVal, 1);

						} else if (strcmp(param.param, "T1_FACTORY_OFFSET") == 0) {
							int16_t bVal = (int16_t)atoi(param.value);
							WriteSetting(EPA_T1_FACTORY_OFFSET, &bVal, 2);

						} else if (strcmp(param.param, "T2_FACTORY_OFFSET") == 0) {
							int16_t bVal = (int16_t)atoi(param.value);
							WriteSetting(EPA_T2_FACTORY_OFFSET, &bVal, 2);

						} else if (strcmp(param.param, "CHAMBER_PRESSURE_OFFSET") == 0) {
							int8_t bVal = (int8_t)atoi(param.value);
							WriteSetting(EPA_CHAMBER_PRESSURE_OFFSET, &bVal, 1);


						} else if (strcmp(param.param, "FIRST_VACUUM") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_FIRST_VACUUM, &bVal, 1);

						} else if (strcmp(param.param, "OTHER_VACUUM") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_OTHER_VACUUM, &bVal, 1);

						} else if (strcmp(param.param, "VACUUM_THRESHOLD") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_VACUUM_THRESHOLD, &bVal, 1);

						} else if (strcmp(param.param, "OTHER_PRESSURE") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_OTHER_PRESSURE, &bVal, 1);

						} else if (strcmp(param.param, "TDS_MIN") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_TDS_MIN, &bVal, 1);

						} else if (strcmp(param.param, "VACUUM_NUMBER") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_VACUUM_NUMBER, &bVal, 1);

						} else if (strcmp(param.param, "CLEAN_CYCLES_INTERVAL") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_CLEAN_CYCLES_INTERVAL, &bVal, 1);

						} else if (strcmp(param.param, "SERVICE_CYCLES_INTERVAL") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_SERVICE_CYCLES_INTERVAL, &bVal, 1);


						} else if (strcmp(param.param, "LAST_CLEAN_CYCLE") == 0) {
							uint16_t bVal = (uint16_t)atoi(param.value);
							WriteSetting(EPA_LAST_CLEAN_CYCLE, &bVal, 2);

						} else if (strcmp(param.param, "LAST_SERVICE_CYCLE") == 0) {
							uint16_t bVal = (uint16_t)atoi(param.value);
							WriteSetting(EPA_LAST_SERVICE_CYCLE, &bVal, 2);

						} else if (strcmp(param.param, "USER_STERIL_TEMP") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_USER_STERIL_TEMP, &bVal, 1);

						} else if (strcmp(param.param, "USER_STERIL_TIME") == 0) {
							uint16_t bVal = (uint16_t)atoi(param.value);
							WriteSetting(EPA_USER_STERIL_TIME, &bVal, 2);

						} else if (strcmp(param.param, "USER_DRY_TIME") == 0) {
							uint16_t bVal = (uint16_t)atoi(param.value);
							WriteSetting(EPA_USER_DRY_TIME, &bVal, 2);

						} else if (strcmp(param.param, "USER_VACUUM_NUMBER") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_USER_VACUUM_NUMBER, &bVal, 1);

						} else if (strcmp(param.param, "FUN_NIGHT_CYCLE_TIME") == 0) {
							uint16_t bVal = (uint16_t)atoi(param.value);
							WriteSetting(EPA_FUN_NIGHT_CYCLE_TIME, &bVal, 2);

						} else if (strcmp(param.param, "FUN_PRINTER_ACTIVE") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_FUN_PRINTER_ACTIVE, &bVal, 1);

						} else if (strcmp(param.param, "FUN_USB_ACTIVE") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_FUN_USB_ACTIVE, &bVal, 1);

						} else if (strcmp(param.param, "FUN_WATER_LOAD_ACTIVE") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_FUN_WATER_LOAD_ACTIVE, &bVal, 1);

						} else if (strcmp(param.param, "FUN_TDS_ACTIVE") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_FUN_TDS_ACTIVE, &bVal, 1);

						} else if (strcmp(param.param, "FUN_DRY_TIME") == 0) {
							uint16_t bVal = (uint16_t)atoi(param.value);
							WriteSetting(EPA_FUN_DRY_TIME, &bVal, 2);

						} else if (strcmp(param.param, "FUN_PRESS_DISPLAY_OFFSET") == 0) {
							uint8_t bVal = (uint8_t)atoi(param.value);
							WriteSetting(EPA_FUN_PRESS_DISPLAY_OFFSET, &bVal, 1);
						}
//						if (strcmp(param.param, "FUN_PRESS_DISPLAY_OFFSET") == 0) {
//							bVal = (uint8_t)param.value;
//							WriteSetting(EPA_FUN_PRESS_DISPLAY_OFFSET, &bVal, 1);
//						}

	//				printf("%s", rBuffer);
					}
				}
			}



//				param = strtok(rBuffer, ":");
//				if (strcmp(trim(rBuffer), "VERSION") == 0) {
//					param = strtok(NULL, ":");
//					if (isdigit(trim(param)))
//					printf("%s_\n", trim(param));
//				}


//			f_lseek(&USBHFile,USBHFile.fsize);
//			f_printf(&USBHFile,USBString);
			f_close(&USBHFile);
			HAL_NVIC_SystemReset();
		}
	}

}


void SdcardWriteAction(char *file_name , char *data)
{

	sprintf(USBString, data);
	sprintf(currFilename, file_name);
	USBWrite = 1;
//	FRESULT fr;
//	static FATFS USBHFatFS;
//	static FIL USBHFile;
//
//
//	if (f_mount(&USBHFatFS, "", 1) == FR_OK) {
////		  osSemaphoreWait(semaphoreSensorsHandle, 100);
//
//		fr = f_open(&USBHFile, file_name, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
//
//		if (fr == FR_OK) fr = f_lseek(&USBHFile,USBHFile.fsize);
//
//		if (fr == FR_OK) fr = f_printf(&USBHFile, data);
//
//		fr = f_close(&USBHFile);
////		  osSemaphoreRelease(semaphoreSensorsHandle);
//	}

//		FIL fil;
//		FRESULT fr;
//
//		clearBuffer();
//		insertBuffer(data);
//
//		_mountSdcard();
//		_openfile(file_name , WRITE_FILE , &fil);
//
////		fr = f_lseek(&fil , f_size(&fil));
////		if(fr != FR_OK)
////			return;
//
//		_writeString(Buffer , &fil);
//		_closefile(&fil);
//		_unmountSdcard();
}
//void clearBuffer(void)
//{
//	 for(int s = 0; s <= MAX_BUFFER; s++)
//			Buffer[s] = (char) 0;
//}
//
//void insertBuffer(char *data)
//{
//	 sprintf(Buffer , "%s" , data);
//}
//
//void InitSdcard(void)
//{
////		initGpio();
////		initSpi();
////		MX_FATFS_Init();
//}
//
//void _mountSdcard(void)
//{
//
//	if(state_mount == SDCARD_MOUNTED)
//			return;
//	else
//	{
//			if(f_mount(&fatfs , "" , 0) != FR_OK)
//			{
//				sdcardErrorHandle(SDCARD_ERROR_MOUNT , "Failed to mount sdcard");
//				state_mount = SDCARD_NOT_MOUNTED;
//			}
//			else
//			{
//				state_mount = SDCARD_MOUNTED;
//				state_unmount = SDCARD_NOT_UNMOUNTED;
//			}
//	}
//}
//
//void _unmountSdcard(void)
//{
//
//	if(state_unmount == SDCARD_UNMOUNTED)
//			return;
//	else
//	{
//			if(f_mount(NULL , "" , 1) != FR_OK)
//			{
//				sdcardErrorHandle(SDCARD_ERROR_MOUNT , "Failed to unmount sdcard");
//				state_unmount = SDCARD_NOT_UNMOUNTED;
//			}
//			else
//			{
//				state_unmount = SDCARD_UNMOUNTED;
//				state_mount = SDCARD_NOT_MOUNTED;
//			}
//	}
//}
//
//void _writeString(char *str , FIL *fil)
//{
//		f_puts(str , fil);
//}
//
//void _readString(char *buffer , size_t read_size , FIL *fil)
//{
//
//}
//
//void _openfile(char *file_name , openType_t type , FIL *fil)
//{
//
//		if(type == READ_WRITE_FILE)
//		{
//
//				if(f_open(fil , file_name , FA_OPEN_ALWAYS | FA_READ | FA_WRITE))
//					sdcardErrorHandle(SDCARD_ERROR_MOUNT , "Failed to open -R/W file");
//
//		}
//		else if(type == READ_FILE) {
//
//			if(f_open(fil , file_name , FA_READ))
//				sdcardErrorHandle(SDCARD_ERROR_MOUNT , "Failed to open -R file");
//		}
//		else if(type == WRITE_FILE)
//		{
//				if(f_open(fil , file_name , FA_OPEN_ALWAYS | FA_WRITE))
//					sdcardErrorHandle(SDCARD_ERROR_MOUNT , "Failed to open -W file");
//		}
//		else return;
//}
//
//void _closefile(FIL *fil)
//{
//	 if(f_close(fil) != FR_OK)
//		 sdcardErrorHandle(SDCARD_ERROR_MOUNT , "Failed to close file");
//}
//
//storage_info_t getSpaceInfo()
//{
//	 storage_info_t info;
//	 DWORD fre_clus;
//	 //
//	 if(f_getfree("" , &fre_clus , &pfatfs) != FR_OK)
//		 sdcardErrorHandle(SDCARD_ERROR_MOUNT , "Failed to get space info");
//
//	 info.totalsize = (uint32_t)((pfatfs->n_fatent - 2) * pfatfs->csize * 0.5);
//	 info.freesize = (uint32_t)(fre_clus * pfatfs->csize * 0.5);
//
//	 if(info.freesize < 1)
//		 sdcardErrorHandle(SDCARD_ERROR_MOUNT , "Low space in memory");
//
//	 return info;
//}
//
//void _makeDirectory(char *dir_name)
//{
//		_mountSdcard();
//
//		if(f_mkdir(dir_name) != FR_OK)
//		 sdcardErrorHandle(SDCARD_ERROR_MOUNT , "Failed to get space info");
//
//		_unmountSdcard();
//}
//
//void sdcardErrorHandle(uint8_t error_code , char *error_message)
//{
//		// Handle Errors
//}
//
//file_state_t getFileState(char *file)
//{
//		file_state_t file_state;
//		FRESULT fr;
//		FILINFO fno;
//
//		fr = f_stat(file , &fno);
//
//		if(fr == FR_OK)
//			file_state = FILE_EXSITS;
//		else
//			file_state = FILE_NOT_EXISTS;
//
//		return file_state;
//}

