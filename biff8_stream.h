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
 * biff8_stream.h
 *
 *  Created on: Jun 17, 2011
 *      Author: Billy
 */

#ifndef BIFF8_STREAM_H_
#define BIFF8_STREAM_H_

#include "compound_document.h"

#define BIFF8_STREAM_NAME_ROOT_ENTRY "R\x00o\x00o\x00t\x00 \x00""E\x00""n\x00t\x00r\x00y\x00\x00"
#define BIFF8_STREAM_NAME_SIZE_ROOT_ENTRY 22

#define BIFF8_STREAM_NAME_WORKBOOK "W\x00o\x00r\x00k\x00""b\x00""o\x00o\x00k\x00\x00"
#define BIFF8_STREAM_NAME_SIZE_WORKBOOK 18

#define BIFF8_STREAM_NAME_SUMMARY_INFORMATION "\x05\x00S\x00u\x00m\x00m\x00""a\x00""r\x00y\x00I\x00n\x00""f\x00""o\x00r\x00m\x00""a\x00""t\x00i\x00o\x00n\x00\x00"
#define BIFF8_STREAM_NAME_SIZE_SUMMARY_INFORMATION 40

#define BIFF8_STREAM_NAME_DOCUMENT_SUMMARY_INFORMATION "\x05\x00""D\x00""o\x00""c\x00""u\x00m\x00""e\x00""n\x00t\x00S\x00u\x00m\x00m\x00""a\x00""r\x00y\x00I\x00n\x00""f\x00""o\x00r\x00m\x00""a\x00""t\x00i\x00o\x00n\x00\x00"
#define BIFF8_STREAM_NAME_SIZE_DOCUMENT_SUMMARY_INFORMATION 56

#define BIFF8_STREAM_NAME_CTLS "C\x00t\x00l\x00s\x00\x00"
#define BIFF8_STREAM_NAME_SIZE_CTLS 10

#define BIFF8_STREAM_NAME_USER_NAMES "U\x00s\x00""e\x00""r\x00 \x00N\x00""a\x00""m\x00""e\x00""s\x00\x00"
#define BIFF8_STREAM_NAME_SIZE_USER_NAMES 22

#define BIFF8_STREAM_NAME_REVISION_LOG "R\x00""e\x00""v\x00i\x00s\x00i\x00o\x00n\x00 \x00L\x00o\x00g\x00\x00"
#define BIFF8_STREAM_NAME_SIZE_REVISION_LOG 26

typedef enum {
	BIFF8_STREAM_TYPE_ROOT_ENTRY, // not an actual BIFF8 stream; just for data integrity check
	BIFF8_STREAM_TYPE_WORKBOOK,
	BIFF8_STREAM_TYPE_SUMMARY_INFORMATION,
	BIFF8_STREAM_TYPE_DOCUMENT_SUMMARY_INFORMATION,
	BIFF8_STREAM_TYPE_CTLS,
	BIFF8_STREAM_TYPE_USER_NAMES,
	BIFF8_STREAM_TYPE_REVISION_LOG,
	BIFF8_STREAM_TYPE_UNKNOWN = -1
} BIFF8_STREAM_TYPE;

BIFF8_STREAM_TYPE biff8StreamTypeGet(COMPOUND_DOCUMENT_INFO *compDocInfo, int dirID);
int biff8StreamIsWorkbook(COMPOUND_DOCUMENT_INFO *compDocInfo, int dirID);

#endif /* BIFF8_STREAM_H_ */
