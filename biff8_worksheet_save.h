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
 * biff8_worksheet_save.h
 *
 *  Created on: Jun 23, 2011
 *      Author: Billy
 */

#ifndef BIFF8_WORKSHEET_SAVE_H_
#define BIFF8_WORKSHEET_SAVE_H_

#include "compound_document.h"
#include "biff8_cell.h"
#include "biff8_workbook.h"
#include "biff8_worksheet.h"
#include "biff8_string.h"

typedef enum {
	BIFF8_WORKSHEET_SAVE_PRESERVE, // retain dimension
	BIFF8_WORKSHEET_SAVE_COMPACT_0, // remove empty rows and columns after non-empty ones
	BIFF8_WORKSHEET_SAVE_COMPACT_1, // remove empty rows and columns before and after non-empty ones
	BIFF8_WORKSHEET_SAVE_COMPACT_2 // remove all empty cells
} BIFF8_WORKSHEET_SAVE_MODE;

int biff8WorksheetSaveToFile(FILE *file, COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_WORKBOOK_GLOBALS *globals, BIFF8_WORKSHEET *worksheet, char delimiter, char textDelimiter, BIFF8_STRING_ENCODING encoding, char replacement, int crlf, int enclose, BIFF8_WORKSHEET_SAVE_MODE saveMode);

#endif /* BIFF8_WORKSHEET_SAVE_H_ */
