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
 * biff8_record_header.h
 *
 *  Created on: Jun 21, 2011
 *      Author: Billy
 */

#ifndef BIFF8_RECORD_HEADER_H_
#define BIFF8_RECORD_HEADER_H_

typedef struct {
	unsigned char id[2];
	unsigned char dataSize[2];
} BIFF8_RECORD_HEADER_RAW;

#define BIFF8_RECORD_HEADER_SIZE sizeof(BIFF8_RECORD_HEADER_RAW)

typedef struct {
	unsigned short id;
	unsigned short dataSize;
} BIFF8_RECORD_HEADER_STRUCT;

#endif /* BIFF8_RECORD_HEADER_H_ */