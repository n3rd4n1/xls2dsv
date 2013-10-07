/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2011-2013 Billy Millare
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * char_to_uint.c
 *
 *  Created on: Jun 18, 2011
 *      Author: Billy
 */

#include "char_to_uint.h"
#include <string.h>

unsigned long long charToUint(CHAR_TO_UINT_BYTE_ORDER byteOrder, unsigned char *value, int numBytes)
{
	if(!value)
		return 0;
	
	if(!numBytes || numBytes > sizeof(unsigned long long))
		return 0;

	switch(byteOrder)
	{
	case CHAR_TO_UINT_BYTE_ORDER_LITTLE_ENDIAN:
	case CHAR_TO_UINT_BYTE_ORDER_BIG_ENDIAN:
		break;
	default:
		return 0;
	}
	
	unsigned char buffer[sizeof(unsigned long long)];
	unsigned char *ordered = value;

	if(byteOrder == CHAR_TO_UINT_BYTE_ORDER_BIG_ENDIAN)
	{
		int i = 0, j = numBytes - 1;
		
		for( ; i < numBytes; i++, j--)
			buffer[i] = value[j];

		ordered = buffer;
	}

	if(numBytes == 1)
		return *(unsigned char *)ordered;

	if(numBytes == 2)
		return *(unsigned short *)ordered;
	
	if(numBytes < 5)
		return *(unsigned int *)ordered;
	
	return *(unsigned long long *)ordered;
}
