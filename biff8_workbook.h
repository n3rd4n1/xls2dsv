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
 * biff8_workbook.h
 *
 *  Created on: Jun 17, 2011
 *      Author: Billy
 */

#ifndef BIFF8_WORKBOOK_H_
#define BIFF8_WORKBOOK_H_

#include "compound_document.h"
#include "biff8_records.h"
#include "biff8_record_details.h"
#include "biff8_string.h"

typedef struct {
	BIFF8_RECORD_DATEMODE_STRUCT dateMode;

	BIFF8_RECORD_SST_STRUCT sst;

	BIFF8_RECORD_SHEET_STRUCT *sheet;
	unsigned int numSheets;

	BIFF8_RECORD_XF_STRUCT *xf;
	unsigned int numXf;

	BIFF8_RECORD_FORMAT_STRUCT *format;
	unsigned int numFormat;
} BIFF8_WORKBOOK_GLOBALS;

#define BIFF8_WORKBOOK_GLOBALS_INITIALIZER { BIFF8_RECORD_DATEMODE_STRUCT_INITIALIZER, BIFF8_RECORD_SST_STRUCT_INITIALIZER, .sheet = 0, .numSheets = 0, .xf = 0, .numXf = 0, .format = 0, .numFormat = 0 }

int biff8WorkbookLocate(COMPOUND_DOCUMENT_INFO *compDocInfo);
int biff8WorkbookGlobalsInfoBuild(COMPOUND_DOCUMENT_INFO *compDocInfo, unsigned char *workbook, unsigned int size, BIFF8_WORKBOOK_GLOBALS *globals);
void biff8WorkbookGlobalsInfoDestroy(BIFF8_WORKBOOK_GLOBALS *globals);
void biff8WorkbookGlobalsInfoPrint(COMPOUND_DOCUMENT_INFO *compDocInfo, unsigned char *workbook, unsigned int size);

int biff8WorkbookFormatStringGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_WORKBOOK_GLOBALS *globals, unsigned int formatIndex, BIFF8_STRING **formatString);
void biff8WorkbookFormatStringFree(BIFF8_STRING **formatString);


#endif /* BIFF8_WORKBOOK_H_ */
