/*==============================================================================

Filename: flash.c

Created on: Mar 30, 2023

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

#include "flash.h"

#include "Inc/DB_Protocol.h"
#include "bootloader.h"
#include "settings.h"

uint8_t dispString[32];
uint8_t readBuf[PACKET_SIZE];
uint32_t readNum;
uint8_t retval;

//FATFS USBHFatFS;    /* File system object for USBH logical drive */
//FIL USBHFile;       /* File object for USBH */

uint8_t iap_usb(FIL *fil) {
	uint32_t size;
	uint8_t sizef;
	uint16_t packetNum;
	uint32_t programAddr = PROGRAM_ADDRESS;
	char s[8];

	FRESULT fr;

	fr = f_open(&USBHFile, UPDATE_FILENAME, FA_READ);

	if (fr == FR_OK) {

		size = f_size(fil);
		sizef = size / 1024;

		#if (CPU_TYPE == 1)
			eraseFlash(PROGRAM_ADDRESS, 99);//
		#elif (CPU_TYPE == 2)
			eraseFlash(PROGRAM_ADDRESS, 102);//
		#endif

		if((size % PACKET_SIZE) == 0 )
		  packetNum = size / PACKET_SIZE;
		else
		  packetNum = size / PACKET_SIZE + 1;

//		sprintf(s, "P%03d", sizef);
//		SendCommand(DISPLAY_CMD_WRITE_DIGITS,1,0,0,s);
//		SendCommand(DISPLAY_CMD_WRITE_DIGITS,2,0,0,"    ");
//		SendCommand(DISPLAY_CMD_WRITE_DIGITS,3,0,0,"    ");
//		HAL_Delay(3000);

		uint16_t i = 0;
		uint8_t tmp;

		uint8_t d = 0;
		uint8_t p = 0;

		uint32_t pos = 0;

		while(1) {
	//		while(i < packetNum) {
			sprintf(s, "P%03d", i);
			SendCommand(DISPLAY_CMD_WRITE_DIGITS,1,0,0,s);

			memset(readBuf, 0 , PACKET_SIZE);
			retval = f_read(fil, readBuf, PACKET_SIZE, &readNum);

			if(retval != FR_OK) {
				sprintf(s, "Fr%02d", retval);
				SendCommand(DISPLAY_CMD_WRITE_DIGITS,1,0,0,s);
				HAL_Delay(3000);

				f_close(fil);

//				SendCommand(DISPLAY_CMD_WRITE_DIGITS,1,0,0,"    ");
//				SendCommand(DISPLAY_CMD_WRITE_DIGITS,2,0,0,"    ");
//				SendCommand(DISPLAY_CMD_WRITE_DIGITS,3,0,0,"    ");
//				HAL_Delay(3000);

				HAL_NVIC_SystemReset();
			}

			//tmp = program256B(programAddr, readBuf);


	//		if(tmp == 0) {
	//			HAL_Delay(200);
	//			break;
	//		}

			if (readNum < PACKET_SIZE) break;

			i++;
			programAddr += PACKET_SIZE;

			if (i > 30) {
				i = 0;
				pos = f_tell(fil);
				f_close(fil);
				f_open(fil, UPDATE_FILENAME, FA_READ);
				f_lseek(fil, pos);
			}

	//		if (d > 200) d = 0;
	//
	//		switch (d) {
	//		case 0:
			//			SendCommand(DISPLAY_CMD_WRITE_DIGITS,1,0,0,"o   ");
	//			break;
	//		case 50:
	//			SendCommand(DISPLAY_CMD_WRITE_DIGITS,1,0,0," o  ");
	//			break;
	//		case 100:
	//			SendCommand(DISPLAY_CMD_WRITE_DIGITS,1,0,0,"  o ");
	//			break;
	//		case 150:
	//			SendCommand(DISPLAY_CMD_WRITE_DIGITS,1,0,0,"   o");
	//			break;
	//		}
	//		d++;


		}

	} else {
		sprintf(s, "Hr%02d", fr);
		SendCommand(DISPLAY_CMD_WRITE_DIGITS,1,0,0,s);
		HAL_Delay(3000);
		HAL_NVIC_SystemReset();

	}
//	f_close(fil);

	HAL_Delay(3000);

	return 0;
}

void  eraseFlash(uint32_t startAddr, uint8_t pageNum)
{
	HAL_FLASH_Unlock();

	FLASH_EraseInitTypeDef f;

	f.TypeErase = FLASH_TYPEERASE_PAGES;
	f.PageAddress = startAddr;
	f.NbPages = pageNum;

	uint32_t PageError = 0;
	HAL_FLASHEx_Erase(&f, &PageError);

	HAL_FLASH_Lock();
}


uint8_t program256B(uint32_t startAddr, uint8_t *Buf)
{
	uint8_t readBackBuf[PACKET_SIZE];

	HAL_FLASH_Unlock();
	for(uint16_t i = 0; i < PACKET_SIZE ; i += 4)
	{
		uint32_t tmp = (Buf[i + 3] << 24) + (Buf[i + 2] << 16) + (Buf[i + 1] << 8) + Buf[i];
		HAL_FLASH_Program(TYPEPROGRAM_WORD, startAddr + i, tmp);
	}
	HAL_FLASH_Lock();


	return 1;

	memcpy(readBackBuf,(uint8_t *)startAddr,PACKET_SIZE);
	uint8_t tmp = memcmp(readBackBuf,Buf,PACKET_SIZE);
	if(tmp == 0)
		return 1;
	else
		return 0;
}
