/*==============================================================================

Filename: TM7705.h

Created on: Jan 4, 2023

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

==================================================================================
PURPOSE: Modbus CRC16 calculation
USAGE:
================================================================================*/



#ifndef INC_TM7705_H_
#define INC_TM7705_H_

//register selection
//RS2 RS1 RS0
static const uint8_t REG_CMM = 0x0; //communication register 8 bit
static const uint8_t REG_SETUP = 0x1; //setup register 8 bit
static const uint8_t REG_CLOCK = 0x2; //clock register 8 bit
static const uint8_t REG_DATA = 0x3; //data register 16 bit, contains conversion result
static const uint8_t REG_TEST = 0x4; //test register 8 bit, POR 0x0
static const uint8_t REG_NOP = 0x5; //no operation
static const uint8_t REG_OFFSET = 0x6; //offset register 24 bit
static const uint8_t REG_GAIN = 0x7; // gain register 24 bit

//channel selection for AD7706 (for AD7705 use the first two channel definitions)
//CH1 CH0
static const uint8_t CHN_AIN1 = 0x0; //AIN1; calibration register pair 0
static const uint8_t CHN_AIN2 = 0x1; //AIN2; calibration register pair 1
static const uint8_t CHN_COMM = 0x2; //common; calibration register pair 0
static const uint8_t CHN_AIN3 = 0x3; //AIN3; calibration register pair 2

//output update rate
//CLK FS1 FS0
static const uint8_t UPDATE_RATE_20 = 0x0; // 20 Hz
static const uint8_t UPDATE_RATE_25 = 0x1; // 25 Hz
static const uint8_t UPDATE_RATE_100 = 0x2; // 100 Hz
static const uint8_t UPDATE_RATE_200 = 0x3; // 200 Hz
static const uint8_t UPDATE_RATE_50 = 0x4; // 50 Hz
static const uint8_t UPDATE_RATE_60 = 0x5; // 60 Hz
static const uint8_t UPDATE_RATE_250 = 0x6; // 250 Hz
static const uint8_t UPDATE_RATE_500 = 0x7; // 500 Hz

//operating mode options
//MD1 MD0
static const uint8_t MODE_NORMAL = 0x0; //normal mode
static const uint8_t MODE_SELF_CAL = 0x1; //self-calibration
static const uint8_t MODE_ZERO_SCALE_CAL = 0x2; //zero-scale system calibration, POR 0x1F4000, set FSYNC high before calibration, FSYNC low after calibration
static const uint8_t MODE_FULL_SCALE_CAL = 0x3; //full-scale system calibration, POR 0x5761AB, set FSYNC high before calibration, FSYNC low after calibration

//gain setting
static const uint8_t GAIN_1 = 0x0;
static const uint8_t GAIN_2 = 0x1;
static const uint8_t GAIN_4 = 0x2;
static const uint8_t GAIN_8 = 0x3;
static const uint8_t GAIN_16 = 0x4;
static const uint8_t GAIN_32 = 0x5;
static const uint8_t GAIN_64 = 0x6;
static const uint8_t GAIN_128 = 0x7;

static const uint8_t UNIPOLAR = 0x0;
static const uint8_t BIPOLAR = 0x1;

static const uint8_t CLK_DIV_1 = 0x1;
static const uint8_t CLK_DIV_2 = 0x2;


#endif /* INC_TM7705_H_ */
