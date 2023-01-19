/*==============================================================================

Filename: 24Cxx.c

Created on: Jan 10, 2023

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

#include "24Cxx.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "stm32f1xx_hal.h"

// Define the I2C
extern I2C_HandleTypeDef hi2c1;
#define EEPROM_I2C hi2c1

// EEPROM ADDRESS (8bits)
#define EEPROM_ADDR 0x50

// Define EEPROM specifications (from datasheet)
#define EEPROM_PAGE_SIZE 8     		// Page size (bytes)
#define EEPROM_PAGE_NUM  32    		// Total pages
#define EEPROM_ADDRESS_BYTES 1 		// Address width (bytes)
#define EEPROM_RW_TIMEOUT 1000		// Read/write timeout (milliseconds)
#define EEPROM_EXECUTION_DELAY 5	// Read/write operation, time wait to complete (milliseconds)

static uint16_t BytesToWrite (uint16_t size, uint16_t offset);
static void Float2Bytes(uint8_t * ftoa_bytes_temp,float float_variable);
static float Bytes2Float(uint8_t * ftoa_bytes_temp);

/*****************************************************************************************************************************************/

void EEPROM_Read(uint16_t address, uint8_t* data, uint16_t size) {
	uint8_t pageAddressBit = log(EEPROM_PAGE_SIZE)/log(2);

	uint16_t startPage = address / EEPROM_PAGE_SIZE;
	uint16_t endPage = (address + size - 1) / EEPROM_PAGE_SIZE;
	uint16_t totPages = (endPage - startPage) + 1;
	uint16_t offset = address - (startPage * EEPROM_PAGE_SIZE);

	uint16_t index = 0;

	for (int i = 0; i < totPages; i++) {
		uint16_t memAddress = startPage << pageAddressBit | offset;
		uint16_t bytesRemaining = BytesToWrite(size, offset);  // calculate the remaining bytes to be written

		HAL_I2C_Mem_Read(&EEPROM_I2C, EEPROM_ADDR << 1, memAddress, EEPROM_ADDRESS_BYTES, &data[index], bytesRemaining, EEPROM_RW_TIMEOUT);

		startPage += 1;  // increment the page, so that a new page address can be selected for further write
		offset = 0;   // since we will be writing to a new page, so offset will be 0
		size -= bytesRemaining;  // reduce the size of the bytes
		index += bytesRemaining;  // update the position for the data buffer
	}
}

float EEPROM_Read_Float (uint16_t address) {
	uint8_t buffer[4];
	EEPROM_Read(address, buffer, 4);
	return (Bytes2Float(buffer));
}

void EEPROM_Write(uint16_t address, uint8_t* data, uint16_t size) {

	// Find out the number of bit, where the page addressing starts
	uint8_t pageAddressBit = log(EEPROM_PAGE_SIZE)/log(2);

	uint16_t startPage = address / EEPROM_PAGE_SIZE;
	uint16_t endPage = (address + size -1) / EEPROM_PAGE_SIZE;
	uint16_t totPages = (endPage - startPage) + 1;
	uint16_t offset = address - (startPage * EEPROM_PAGE_SIZE);

	uint16_t index = 0;

	for (int i = 0; i < totPages; i++) {
		uint16_t memAddress = startPage << pageAddressBit | offset;
		uint16_t bytesRemaining = BytesToWrite(size, offset);  // calculate the remaining bytes to be written

		HAL_I2C_Mem_Write(&EEPROM_I2C, EEPROM_ADDR << 1, memAddress, EEPROM_ADDRESS_BYTES, &data[index], bytesRemaining, EEPROM_RW_TIMEOUT);
		HAL_Delay(5);

		startPage += 1;  // increment the page, so that a new page address can be selected for further write
		offset = 0;   // since we will be writing to a new page, so offset will be 0
		size -= bytesRemaining;  // reduce the size of the bytes
		index += bytesRemaining;  // update the position for the data buffer
	}
}

void EEPROM_Write_Float (uint16_t address, float data) {
	uint8_t buffer[4];
	Float2Bytes(buffer, data);
	EEPROM_Write(address, buffer, 4);
}

// function to determine the remaining bytes
static uint16_t BytesToWrite (uint16_t size, uint16_t offset) {
	if ((size+offset)<EEPROM_PAGE_SIZE) return size;
	else return EEPROM_PAGE_SIZE-offset;
}


static void Float2Bytes(uint8_t* ftoa_bytes_temp, float float_variable) {
    union {
      float a;
      uint8_t bytes[4];
    } thing;

    thing.a = float_variable;

    for (uint8_t i = 0; i < 4; i++) {
      ftoa_bytes_temp[i] = thing.bytes[i];
    }
}

static float Bytes2Float(uint8_t* ftoa_bytes_temp) {
    union {
      float a;
      uint8_t bytes[4];
    } thing;

    for (uint8_t i = 0; i < 4; i++) {
    	thing.bytes[i] = ftoa_bytes_temp[i];
    }

   float float_variable =  thing.a;
   return float_variable;
}

void EEPROM_EraseAll () {
	uint8_t pageData[EEPROM_PAGE_SIZE];
	memset(pageData,0xff,EEPROM_PAGE_SIZE);

	for (int page = 0; page < EEPROM_PAGE_NUM; page++) {
		EEPROM_Write(page * EEPROM_PAGE_SIZE, pageData, EEPROM_PAGE_SIZE);
	}
}

//void EEPROM_PageErase (uint16_t page)
//{
//	// calculate the memory address based on the page number
//	int paddrposition = log(PAGE_SIZE)/log(2);
//	uint16_t MemAddress = page<<paddrposition;
//
//	// create a buffer to store the reset values
//	uint8_t data[PAGE_SIZE];
//	memset(data,0x01,PAGE_SIZE);
//
//	// write the data to the EEPROM
//	HAL_I2C_Mem_Write(EEPROM_I2C, EEPROM_ADDR, MemAddress, 2, data, PAGE_SIZE, 1000);
//
//	HAL_Delay (5);  // write cycle delay
//}
