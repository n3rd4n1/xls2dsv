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
 * char_to_uint.h
 *
 *  Created on: Jun 18, 2011
 *      Author: Billy
 */

#ifndef CHAR_TO_UINT_H_
#define CHAR_TO_UINT_H_

typedef enum {
	CHAR_TO_UINT_BYTE_ORDER_LITTLE_ENDIAN,
	CHAR_TO_UINT_BYTE_ORDER_BIG_ENDIAN
} CHAR_TO_UINT_BYTE_ORDER;

#define CHAR_TO_UINT_SWAP_BYTES CHAR_TO_UINT_BYTE_ORDER_BIG_ENDIAN

unsigned long long charToUint(CHAR_TO_UINT_BYTE_ORDER byteOrder, unsigned char *value, int numBytes);

#endif // CHAR_TO_UINT_H_
