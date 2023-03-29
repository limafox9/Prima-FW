/*==============================================================================

Filename: bootloader.c

Created on: Mar 29, 2023

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

#include "bootloader.h"

void bootloaderInit()
{
//	Flashed_offset = 0;
//	flashStatus = Unerased;
//	BootloaderMode bootloaderMode;
//	if(HAL_GPIO_ReadPin(BOOT1_GPIO_Port, BOOT1_Pin) == GPIO_PIN_SET)
//	{
//		bootloaderMode = FlashMode;
//		for(uint8_t i=0; i<10; i++)
//		{
//			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
//			HAL_Delay(10);
//			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
//			HAL_Delay(90);
//		}
//		HAL_GPIO_WritePin(USB_ENABLE_GPIO_Port, USB_ENABLE_Pin, GPIO_PIN_SET);
//	}else
//	{
//		bootloaderMode = JumpMode;
//		HAL_GPIO_WritePin(USB_ENABLE_GPIO_Port, USB_ENABLE_Pin, GPIO_PIN_RESET);
//	}
//
//	if(HAL_GPIO_ReadPin(App_GPIO_Port, App_Pin) == GPIO_PIN_RESET)
//		App = App1;
//	else
//		App = App2;
//
//	if(bootloaderMode == JumpMode)
//	{
//		if(App == App1)
//		{
//			//Check if the application is there
//			uint8_t emptyCellCount = 0;
//			for(uint8_t i=0; i<10; i++)
//			{
//				if(readWord(APP1_START + (i*4)) == -1)
//					emptyCellCount++;
//			}
//
//			if(emptyCellCount != 10)
//				jumpToApp(APP1_START);
//			else
//				errorBlink();
//		}
//		else
//		{
//			//Check if the application is there
//			uint8_t emptyCellCount = 0;
//			for(uint8_t i=0; i<10; i++)
//			{
//				if(readWord(APP2_START + (i*4)) == -1)
//					emptyCellCount++;
//			}
//
//			if(emptyCellCount != 10)
//				jumpToApp(APP2_START);
//			else
//				errorBlink();
//		}
//	}
}

void Beep(uint8_t hits, uint16_t onDelay, uint16_t offDelay) {

	uint32_t tick;

	for (uint8_t i = 0; i < hits; i++) {
		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, 1);
		HAL_Delay(onDelay);
		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, 0);
		HAL_Delay(offDelay);
	}
}

void jumpToApp(const uint32_t address)
{
	const JumpStruct* vector_p = (JumpStruct*)address;

	Beep(2, 50, 100);

	deinitEverything();
    SCB->VTOR = (uint32_t)address;

	/* let's do The Jump! */
    /* Jump, used asm to avoid stack optimization */
	/* Jump, used asm to avoid stack optimization */
	    asm("\n\
	        msr msp, %0; \n\
	        bx %1;" : : "r"(vector_p->stack_addr), "r"(vector_p->func_p));
}

void deinitEverything()
{


//	MX_FATFS_DeInit();
//	MX_USB_HOST_DeInit();


	//-- reset peripherals to guarantee flawless start of user application
	HAL_GPIO_DeInit(V_BUS_GPIO_Port, V_BUS_Pin);
	HAL_GPIO_DeInit(LED_HEART_GPIO_Port, LED_HEART_Pin);
	  __HAL_RCC_GPIOC_CLK_DISABLE();
	  __HAL_RCC_GPIOD_CLK_DISABLE();
	  __HAL_RCC_GPIOB_CLK_DISABLE();
	  __HAL_RCC_GPIOA_CLK_DISABLE();
	HAL_RCC_DeInit();
	HAL_DeInit();
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;
}
