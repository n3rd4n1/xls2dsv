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
 * biff8_format_string.h
 *
 *  Created on: Jul 1, 2011
 *      Author: Billy
 */

#ifndef BIFF8_FORMAT_STRING_H_
#define BIFF8_FORMAT_STRING_H_

#include "biff8_string.h"
#include "biff8_workbook.h"

typedef struct {
	char *pos;
	char *neg;
	char *zero;
	char *text;
} BIFF8_FORMAT_STRING;

#define BIFF8_FORMAT_STRING_INITIALIZER { .pos = 0, .neg = 0, .zero = 0, .text = 0 }

int biff8FormatStringBuild(BIFF8_STRING *string, BIFF8_FORMAT_STRING *formatString);
void biff8FormatStringDestroy(BIFF8_FORMAT_STRING *formatString);

typedef enum {
	BIFF8_FORMAT_STRING_DECODE_DONE,
	BIFF8_FORMAT_STRING_DECODE_TEXT_PLACEHOLDER,
	BIFF8_FORMAT_STRING_DECODE_FAILED = -1
} BIFF8_FORMAT_STRING_DECODE;

BIFF8_FORMAT_STRING_DECODE biff8FormatStringDecode(BIFF8_WORKBOOK_GLOBALS *globals, char *format, double number, char *buffer);

#endif /* BIFF8_FORMAT_STRING_H_ */
