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
 * biff8_string.h
 *
 *  Created on: Jun 20, 2011
 *      Author: Billy
 */

#ifndef BIFF8_STRING_H_
#define BIFF8_STRING_H_

#include "compound_document.h"
#include "biff8_record_header.h"

typedef enum {
	BIFF8_STRING_ENCODING_ASCII,
	BIFF8_STRING_ENCODING_UTF16,
	BIFF8_STRING_ENCODING_UTF8,
	BIFF8_STRING_ENCODING_UNSET = -1
} BIFF8_STRING_ENCODING;

typedef struct {
	unsigned char *string;
	unsigned int numBytes;
	BIFF8_STRING_ENCODING encoding;
} BIFF8_STRING;

typedef struct {
	int valid;
	unsigned int size0;
	unsigned int size1;
	int hasOpt;
} BIFF8_STRING_SPLIT;

typedef struct {
	BIFF8_RECORD_HEADER_RAW *owner;
	unsigned int index;
	BIFF8_STRING_SPLIT split;
} BIFF8_STRING_INDEX;

#define BIFF8_STRING_INDEX_INITIALIZER(record, index0) { .owner = record, .index = index0, .split.valid = 0 }

int biff8StringAllocate(BIFF8_STRING **string, unsigned int size);
void biff8StringFree(BIFF8_STRING **string);

int biff8StringGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_STRING_INDEX *index, unsigned int stringLengthSize, BIFF8_STRING **string);
int biff8StringIndexArrayNextGet(COMPOUND_DOCUMENT_INFO *compDocInfo, unsigned int stringLengthSize, BIFF8_STRING_INDEX *currentIndex, BIFF8_STRING_INDEX *nextIndex);

int biff8StringConvert(BIFF8_STRING_ENCODING encoding, BIFF8_STRING *string, char sub);

int biff8StringCharMatch(BIFF8_STRING *string, char c, int resetIndex, unsigned int *index, unsigned int *size);

#endif /* BIFF8_STRING_H_ */
