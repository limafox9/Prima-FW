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


FATFS fatfs;
FATFS *pfatfs;
// state Holders
uint8_t state_mount = 0;
uint8_t state_unmount = 0;
//
char currFilename[32];
char USBString[64];
uint8_t USBWrite ;

#define MAX_BUFFER 512
char Buffer[MAX_BUFFER];

void StartUSBTask() {
//	SdcardWriteAction("CIAO.txt" , "Hello\n");
volatile FRESULT fr;
	for (;;) {
		MX_USB_HOST_Process();
		if(USBWrite == 1)
		{
			if(f_mount(&USBHFatFS, "", 1) == FR_OK)
			{
				fr = f_open(&USBHFile, currFilename, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
				if(fr != FR_OK)
          continue;
				fr = f_lseek(&USBHFile,USBHFile.fsize);
				if(fr != FR_OK)
          continue;
				f_printf(&USBHFile,USBString);
				fr = f_close(&USBHFile);
				if(fr != FR_OK)
          continue;
				else USBWrite == 0;
			}
		}
		osDelay(50);
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

