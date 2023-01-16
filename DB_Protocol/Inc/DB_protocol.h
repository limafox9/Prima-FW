/*==============================================================================

Filename: DB_Protocol.h

Created on: Jan 13, 2023

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



#ifndef DB_PROTOCOL_H_
#define DB_PROTOCOL_H_


#include <stdio.h>
#include "DB_globals.h"

#define POS_HEAD 		0
#define POS_COMMAND 	2
#define POS_PARAM1		4
#define POS_PARAM2 		8
#define POS_PARAM3 		12
#define POS_DIGITS 		16
#define POS_CHECKSUM 	24
#define POS_TAIL 		26
#define POS_EOF			28

#define DISPLAY_PROTOCOL_BUFFER_SIZE (POS_EOF + 1)

void Display_DataToProtocol(s_DisplayProtocolData* data, uint8_t* buffer);
void Display_ProtocolToData(uint8_t* bufferIn, s_DisplayProtocolData* dataOut);


#endif /* DB_PROTOCOL_H_ */
