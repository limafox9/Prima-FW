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


uint8_t dispString[32];
uint8_t readBuf[256];
uint32_t readNum;

//FATFS USBHFatFS;    /* File system object for USBH logical drive */
//FIL USBHFile;       /* File object for USBH */

uint8_t iap_usb(FIL *fil)
{
	uint32_t size;
	uint16_t packetNum;
	uint32_t programAddr = 0x0800E000;

	size = f_size(fil);

	eraseFlash(0x0800E000, 99);//

	if((size % 256) == 0 )
	  packetNum = size / 256;
	else
	  packetNum = size / 256 + 1;

	uint16_t i = 0;
	uint8_t tmp;

	uint8_t d = 0;

	while(i < packetNum)
	{
		memset(readBuf, 0 , 256);
		if(f_read(fil,readBuf, 256, &readNum) != 0)
		{
			HAL_NVIC_SystemReset();
		}
		tmp = program256B(programAddr, readBuf);
		if(tmp == 0)
		{
			HAL_Delay(200);
			break;
		}
		i++;
		programAddr += 256;


	}
	f_close(fil);

	HAL_Delay(200);
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
	uint8_t readBackBuf[256];

	HAL_FLASH_Unlock();
	for(uint16_t i = 0; i < 256 ; i += 4)
	{
		uint32_t tmp = (Buf[i + 3] << 24) + (Buf[i + 2] << 16) + (Buf[i + 1] << 8) + Buf[i];
		HAL_FLASH_Program(TYPEPROGRAM_WORD, startAddr + i, tmp);
	}
	HAL_FLASH_Lock();

	return 1;
	memcpy(readBackBuf,(uint8_t *)startAddr,256);
	uint8_t tmp = memcmp(readBackBuf,Buf,256);
	if(tmp == 0)
		return 1;
	else
		return 0;
}
