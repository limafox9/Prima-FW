/*==============================================================================

Filename: print.c

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

#include "main.h"
#include "print.h"

extern UART_HandleTypeDef huart4;

void thermalPrint_line(char *string)
{
	uint8_t cmd[2] = {0x1b,0x40};
	uint8_t pl[1] = {0x0a}; //´òÓ¡ÐÂµÄÒ»ÐÐ£¡
	uint8_t cmd2[3] ={0x1b,0x63,0x00};

	// ³õÊ¼»¯´òÓ¡»ú
	HAL_UART_Transmit(&huart4,cmd,2,100);

//	if(switchValue.isCh == 1)
//	{
//		cmd[0] = 0x1c;
//		cmd[1] = 0x26;
//		HAL_UART_Transmit(&huart4,cmd,2,100);
//	}

	HAL_UART_Transmit(&huart4,(uint8_t *)string,16,100);
  HAL_UART_Transmit(&huart4,cmd2,3,100);
  HAL_UART_Transmit(&huart4,pl,1,100);
	osDelay(20);
	memset(string,0x20,16);
}

void thermalPrint_headLine(char *string)
{
	uint8_t cmd[3];
	uint8_t pl[1] = {0x0a};
	uint8_t cmd2[3] ={0x1b,0x63,0x00};

	cmd[0] = 0x1b;
	cmd[1] = 0x40;
	HAL_UART_Transmit(&huart4,cmd,2,100);

//	if(switchValue.isCh == 1)
//	{
//		cmd[0] = 0x1c;
//		cmd[1] = 0x26;
//		HAL_UART_Transmit(&huart4,cmd,2,100);
//	}

	cmd[0] = 0x1b;                                      // ·Å´óÁ½±¶ÅäÖÃ
	cmd[1] = 0x57;
	cmd[2] = 2;
  HAL_UART_Transmit(&huart4,cmd,3,100);
  HAL_UART_Transmit(&huart4,(uint8_t *)string,14,100);	//´òÓ¡Í·
  HAL_UART_Transmit(&huart4,cmd2,3,100);
  HAL_UART_Transmit(&huart4,pl,1,100);
	osDelay(20);
	memset(string,0x20,20);
}


