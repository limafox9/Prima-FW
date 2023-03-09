/*==============================================================================

Filename: usbkey.h

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



#ifndef INC_USBKEY_H_
#define INC_USBKEY_H_

#include "stm32f1xx.h"
#include "fatfs.h"
#include "stdio.h"

#define SDCARD_ERROR_OPEN_NORMAL	 		0x1
#define SDCARD_ERROR_OPEN_WRITE_MODE	0x2
#define SDCARD_ERROR_OPEN_READ_MODE	 	0x3
#define SDCARD_ERROR_CLOSE 						0x4
#define SDCARD_ERROR_WRITE 						0x5
#define SDCARD_ERROR_READ  						0x6
#define SDCARD_ERROR_MOUNT 						0x7
#define SDCARD_ERROR_UNMOUNT 					0x8
#define SDCARD_ERROR_READ_SIZE 				0x9
#define SDCARD_ERROR_CAPACITY 				0x10

#define SDCARD_MOUNTED 								0x01
#define SDCARD_NOT_MOUNTED 						0x00
#define SDCARD_UNMOUNTED 							0x01
#define SDCARD_NOT_UNMOUNTED 					0x00

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {

		READ_FILE = 0 ,
		WRITE_FILE ,
		READ_WRITE_FILE,

} openType_t;

typedef enum {

		FILE_EXSITS ,
		FILE_NOT_EXISTS

} file_state_t;

typedef struct {

		uint32_t totalsize;
		uint32_t freesize;
		uint8_t percent;

} storage_info_t;

extern char currFilename[32];

void InitSdcard(void);
//
void _mountSdcard(void);
void _unmountSdcard(void);
//
void _writeString(char *str , FIL *fil);
void _readString(char *buffer , size_t read_size , FIL *fil);
//
void _openfile(char *file_name , openType_t type , FIL *fil);
void _closefile(FIL *fil);
//
void clearBuffer(void);
void insertBuffer(char *data);
//
void _makeDirectory(char *dir_name);
//
file_state_t getFileState(char *file);
storage_info_t getSpaceInfo();

void sdcardErrorHandle(uint8_t error_code , char *error_message);
//
void SdcardEjected(void);
void SdcardInserted(void);
void SdcardWriteAction(char *file_name , char *data);

void StartUSBTask();
void USB_Mount();

#ifdef __cplusplus
}
#endif



#endif /* INC_USBKEY_H_ */
