/*==============================================================================

Filename: AD7705.h

Created on: 28 feb 2023

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



#ifndef DEVS_AD7705_H_
#define DEVS_AD7705_H_


#include <stdio.h>
#include "main.h"

// Communication register
typedef enum {
	COM_DRDY_BIT 			= 7,
	COM_RS_BIT 				= 4,
	COM_RW_BIT 				= 3,
	COM_STBY_BIT 			= 2,
	COM_CH1_BIT 			= 1,
	COM_CH0_BIT 			= 0,
} AD7705_CommRegister;

typedef enum {
    COMMS_REGISTER 			= 0b00000000,	// 0
    SETUP_REGISTER 			= 0b00010000,	// 1
    CLOCK_REGISTER 			= 0b00100000,	// 2
    DATA_REGISTER 			= 0b00110000,	// 3
    TEST_REGISTER 			= 0b01000000,	// 4
    NO_OP_REGISTER 			= 0b01010000,	// 5
    OFFSET_REGISTER 		= 0b01100000,	// 6
    GAIN_REGISTER 			= 0b01110000,	// 7
} AD7705_Register;

typedef enum {
	WRITE 					= 0b00000000,	// 0
	READ 					= 0b00001000,	// 1
} AD7705_ReadWrite;

typedef enum {
	POWER_ON 				= 0b00000000,	// 0
	POWER_OFF 				= 0b00000100,	// 1
} AD7705_Power;

typedef enum {
	CHANNEL_0 				= 0b00000000,	// 0
	CHANNEL_1 				= 0b00000001,	// 1
	CHANNEL_0_0 			= 0b00000010,	// 2
	CHANNEL_0_1 			= 0b00000011,	// 3
} AD7705_Channel;


// Setup register
typedef enum {
	SETUP_MODE_BIT 			= 6,
	SETUP_GAIN_BIT 			= 3,
	SETUP_BIPOLAR_MODE_BIT 	= 2,
	SETUP_BUFFER_CTRL_BIT 	= 1,
	SETUP_FSYNC_BIT 		= 0,
} AD7705_SetupRegister;

typedef enum {
	MODE_NORMAL 			= 0b00000000,	// 0
	MODE_SELF_CAL 			= 0b01000000,	// 1
	MODE_ZERO_SCALE_CAL 	= 0b10000000,	// 2
	MODE_FULL_SCALE_CAL 	= 0b11000000,	// 3
} AD7705_ADCMode;

typedef enum {
	GAIN_1 					= 0b00000000,	// 0
	GAIN_2 					= 0b00001000,	// 1
	GAIN_4 					= 0b00010000,	// 2
	GAIN_8 					= 0b00011000,	// 3
	GAIN_16 				= 0b00100000,	// 4
	GAIN_32 				= 0b00101000,	// 5
	GAIN_64 				= 0b00110000,	// 6
	GAIN_128 				= 0b00111000,	// 7
} AD7705_Gain;

typedef enum {
	POLARITY_BIPOLAR 		= 0b00000000,	// 0
	POLARITY_UNIPOLAR 		= 0b00000100,	// 1
} AD7705_Polarity;

typedef enum {
	BUFFER_DISABLE 			= 0b00000000,	// 0
	BUFFER_ENABLE 			= 0b00000010,	// 1
} AD7705_Buffer;

typedef enum {
	CONVERSION_START 		= 0b00000000,	// 0 - FSYNC bit
	CONVERSION_STOP 		= 0b00000001,	// 1
} AD7705_Operation;


// Clock register
typedef enum {
	CLOCK_CLKDIS_BIT 		= 4,
	CLOCK_CLKDIV_BIT 		= 3,
	CLOCK_CLK_BIT 			= 2,
	CLOCK_FS 				= 0,
} AD7705_ClockRegister;

typedef enum {
	CLOCK_ENABLE 			= 0b00000000,	// 0
	CLOCK_DISABLE 			= 0b00010000,	// 1
} AD7705_ClockDis;

typedef enum {
	CLOCK_DIV_DISABLE 		= 0b00000000,	// 0
	CLOCK_DIV_ENABLE 		= 0b00001000,	// 1
} AD7705_ClockDiv;

typedef enum {								// -	Clock in, Out rate (first notch filter), filter cutoff -3db
	CLOCK_1_20				= 0b00000000,	// 0	1.0000 MHz, 20 Hz, 5.24 Hz
	CLOCK_1_25				= 0b00000001,	// 1	1.0000 MHz, 25 Hz, 6.55 Hz
	CLOCK_1_100				= 0b00000010,	// 2	1.0000 MHz, 100 Hz, 26.20 Hz
	CLOCK_1_200				= 0b00000011,	// 3	1.0000 MHz, 200 Hz, 52.40 Hz
	CLOCK_2_4676_50			= 0b00000100,	// 4	2.4567 MHz, 50 Hz, 13.10 Hz
	CLOCK_2_4676_60			= 0b00000101,	// 5	2.4567 MHz, 60 Hz, 15.70 Hz
	CLOCK_2_4676_250		= 0b00000110,	// 6	2.4567 MHz, 250 Hz, 65.50 Hz
	CLOCK_2_4676_500		= 0b00000111,	// 7	2.4567 MHz, 500 Hz, 131.00 Hz
	CLOCK_2_20				= 0b00001000,	// 8	2.0000 MHz, 20 Hz, 5.24 Hz
	CLOCK_2_25				= 0b00001001,	// 9	2.0000 MHz, 25 Hz, 6.55 Hz
	CLOCK_2_100				= 0b00001010,	// 10	2.0000 MHz, 100 Hz, 26.20 Hz
	CLOCK_2_200				= 0b00001011,	// 11	2.0000 MHz, 200 Hz, 52.40 Hz
	CLOCK_4_9152_50			= 0b00001100,	// 12	4.9152 MHz, 50 Hz, 13.10 Hz
	CLOCK_4_9152_60			= 0b00001101,	// 13	4.9152 MHz, 60 Hz, 15.70 Hz
	CLOCK_4_9152_250		= 0b00001110,	// 14	4.9152 MHz, 250 Hz, 65.50 Hz
	CLOCK_4_9152_500		= 0b00001111,	// 15	4.9152 MHz, 500 Hz, 131.00 Hz
} AD7705_ClockFrequencyFilter;

typedef enum {
	COM_NO_OP = 0,
	COM_WRITE_TO_INPUT_REGISTER = 1,
	COM_UPDATE_DAC_WITH_INPUT = 2,
	COM_WRITE_AND_UPDATE_DAC = 3,
	COM_POWER_DOWN_DAC = 4,
	COM_LDAC_MASK_REGISTER = 5,
	COM_SOFTWARE_RESET = 6,
	COM_REFERENCE_SETUP = 7,
	COM_ENABLE_DAISY_CHAIN = 8,
	COM_SETUP_READBACK_REG = 9
} AD7705_Command;

void AD7705_Init(SPI_HandleTypeDef* hspi);
uint16_t AD7705_GetValue(AD7705_Channel channel);
float AD7705_GetTemp(AD7705_Channel channel, float offset);

#endif /* DEVS_AD7705_H_ */
