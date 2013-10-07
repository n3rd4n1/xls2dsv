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
 * biff8_record_details.h
 *
 *  Created on: Jun 20, 2011
 *      Author: Billy
 */

#ifndef BIFF8_RECORD_DETAILS_H_
#define BIFF8_RECORD_DETAILS_H_

#include "biff8_record_header.h"
#include "biff8_string.h"
#include "compound_document.h"

typedef struct {
	BIFF8_RECORD_HEADER_RAW header;
	unsigned char data[30];
} BIFF8_RECORD_DETAILS;

// BOF

#define BIFF8_RECORD_BOF_SIZE 16

typedef struct {
	BIFF8_RECORD_HEADER_RAW header;
	unsigned char biffVersion[2];
	unsigned char type[2];
	unsigned char buildId[2];
	unsigned char buildYear[2];
	unsigned char fileHistoryFlags[4];
	unsigned char compatibility[4];
} BIFF8_RECORD_BOF_RAW;

#define BIFF8_RECORD_BOF_BIFF_VERSION 0x0600

typedef enum {
	BIFF8_RECORD_BOF_TYPE_WORKBOOK_GLOBALS = 0x0005,
	BIFF8_RECORD_BOF_TYPE_VISUAL_BASIC_MODULE = 0x0006,
	BIFF8_RECORD_BOF_TYPE_SHEET_OR_DIALOGUE = 0x0010,
	BIFF8_RECORD_BOF_TYPE_CHART = 0x0020,
	BIFF8_RECORD_BOF_TYPE_MACRO_SHEET = 0x0040,
	BIFF8_RECORD_BOF_TYPE_WORKSPACE = 0x0100
} BIFF8_RECORD_BOF_TYPE;

typedef struct {
	unsigned short biffVersion;
	BIFF8_RECORD_BOF_TYPE type;
	unsigned short buildId;
	unsigned short buildYear;
	unsigned int fileHistoryFlags;
	unsigned int compatibility;
} BIFF8_RECORD_BOF_STRUCT;

int biff8RecordBofDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_BOF_RAW *record, BIFF8_RECORD_BOF_STRUCT *details);

// SHEET

#define BIFF8_RECORD_SHEET_MIN_SIZE 7

#define BIFF8_RECORD_SHEET_STRING_LENGTH_SIZE 1

typedef struct {
	BIFF8_RECORD_HEADER_RAW header;
	unsigned char offset[4];
	unsigned char state[1];
	unsigned char type[1];
	unsigned char name[1];
} BIFF8_RECORD_SHEET_RAW;

typedef enum {
	BIFF8_RECORD_SHEET_STATE_VISIBLE = 0,
	BIFF8_RECORD_SHEET_STATE_HIDDEN = 1,
	BIFF8_RECORD_SHEET_STATE_VERY_HIDDEN = 2
} BIFF8_RECORD_SHEET_STATE;

typedef enum {
	BIFF8_RECORD_SHEET_TYPE_WORKSHEET = 0x00,
	BIFF8_RECORD_SHEET_TYPE_CHART = 0x02,
	BIFF8_RECORD_SHEET_TYPE_VISUAL_BASIC_MODULE = 0x06
} BIFF8_RECORD_SHEET_TYPE;

typedef struct {
	unsigned int offset;
	unsigned char state;
	unsigned char type;
	BIFF8_STRING *name;
} BIFF8_RECORD_SHEET_STRUCT;

int biff8RecordSheetDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_SHEET_RAW *record, BIFF8_RECORD_SHEET_STRUCT *details);
void biff8RecordSheetDetailsDestroy(BIFF8_RECORD_SHEET_STRUCT *details);

// SST

#define BIFF8_RECORD_SST_MIN_SIZE 8

#define BIFF8_RECORD_SST_STRING_LENGTH_SIZE 2

typedef struct {
	BIFF8_RECORD_HEADER_RAW header;
	unsigned char totalStrings[4];
	unsigned char numStrings[4];
	unsigned char base[1];
} BIFF8_RECORD_SST_RAW;

typedef struct {
	unsigned int totalStrings;
	unsigned int numStrings;
	BIFF8_STRING_INDEX *index;
} BIFF8_RECORD_SST_STRUCT;

#define BIFF8_RECORD_SST_STRUCT_INITIALIZER { .index = 0 }

int biff8RecordSstDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_SST_RAW *record, BIFF8_RECORD_SST_STRUCT *details);
void biff8RecordSstDetailsDestroy(BIFF8_RECORD_SST_STRUCT *details);

// DATEMODE

#define BIFF8_RECORD_DATEMODE_SIZE 2

typedef struct {
	BIFF8_RECORD_HEADER_RAW header;
	unsigned char base[2];
} BIFF8_RECORD_DATEMODE_RAW;

typedef struct {
	int valid;
	unsigned short baseYear;
	unsigned char baseMonth;
	unsigned char baseDay;
} BIFF8_RECORD_DATEMODE_STRUCT;

#define BIFF8_RECORD_DATEMODE_STRUCT_INITIALIZER { .valid = 0 }

int biff8RecordDatemodeDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_DATEMODE_RAW *record, BIFF8_RECORD_DATEMODE_STRUCT *details);
void biff8RecordDatemodeDetailsDestroy(BIFF8_RECORD_DATEMODE_STRUCT *details);

// DIMENSION

#define BIFF8_RECORD_DIMENSION_SIZE 14

typedef struct {
	BIFF8_RECORD_HEADER_RAW header;
	unsigned char firstRowIndex[2];
	unsigned char firstRowIndexIgnore[2];
	unsigned char lastRowIndex[2]; // plus one
	unsigned char lastRowIndexIgnore[2];
	unsigned char firstColIndex[2];
	unsigned char lastColIndex[2]; // plus one
	unsigned char notUsed[2];
} BIFF8_RECORD_DIMENSION_RAW;

typedef struct {
	int valid;
	int firstRowIndex;
	int lastRowIndex;
	unsigned int numRows;
	short firstColIndex;
	short lastColIndex;
	unsigned short numColumns;
} BIFF8_RECORD_DIMENSION_STRUCT;

#define BIFF8_RECORD_DIMENSION_STRUCT_INITIALIZER { .valid = 0 }

int biff8RecordDimensionDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_DIMENSION_RAW *record, BIFF8_RECORD_DIMENSION_STRUCT *details);
void biff8RecordDimensionDetailsDestroy(BIFF8_RECORD_DIMENSION_STRUCT *details);

// XF

#define BIFF8_RECORD_XF_SIZE 20

typedef struct {
	BIFF8_RECORD_HEADER_RAW header;
	unsigned char dontCare0[2];
	unsigned char formatRecIndex[2];
	unsigned char xfType[2];
	unsigned char dontCare1[3];
	unsigned char xfUsedAttrib[1];
} BIFF8_RECORD_XF_RAW;

#define BIFF8_RECORD_XF_TYPE_MASK (0x1 << 2)

#define BIFF8_RECORD_XF_USE_GET(x) (x & (0x1 << 2))
#define BIFF8_RECORD_XF_TYPE_GET(x) (x & BIFF8_RECORD_XF_TYPE_MASK)
#define BIFF8_RECORD_XF_PARENT_INDEX_GET(x) ((x >> 4) & 0xfff)

typedef enum {
	BIFF8_RECORD_XF_TYPE_CELL,
	BIFF8_RECORD_XF_TYPE_STYLE = BIFF8_RECORD_XF_TYPE_MASK
} BIFF8_RECORD_XF_TYPE;

typedef struct {
	BIFF8_RECORD_XF_TYPE type;
	int use;
	int index;
} BIFF8_RECORD_XF_STRUCT;

int biff8RecordXfDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_XF_RAW *record, BIFF8_RECORD_XF_STRUCT *details);

// FORMAT

#define BIFF8_RECORD_FORMAT_MIN_SIZE 5

#define BIFF8_RECORD_FORMAT_STRING_LENGTH_SIZE 2

typedef struct {
	BIFF8_RECORD_HEADER_RAW header;
	unsigned char index[2];
	unsigned char string[1];
} BIFF8_RECORD_FORMAT_RAW;

typedef struct {
	int index;
	unsigned char *string;
} BIFF8_RECORD_FORMAT_STRUCT;

int biff8RecordFormatDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_FORMAT_RAW *record, BIFF8_RECORD_FORMAT_STRUCT *details);

// cell record index

typedef struct {
	unsigned char row[2];
	unsigned char col[2];
} BIFF8_RECORD_CELL_INDEX_RAW;

typedef struct {
	unsigned short row;
	unsigned short col;
} BIFF8_RECORD_CELL_INDEX_STRUCT;

// LABELSST

#define BIFF8_RECORD_LABELSST_SIZE 10

#define BIFF8_RECORD_LABELSST_STRING_LENGTH_SIZE 2

typedef struct {
	BIFF8_RECORD_HEADER_RAW header;
	BIFF8_RECORD_CELL_INDEX_RAW cellIndex;
	unsigned char xfRecIndex[2];
	unsigned char sstRecIndex[4];
} BIFF8_RECORD_LABELSST_RAW;

typedef struct {
	BIFF8_RECORD_CELL_INDEX_STRUCT cellIndex;
	unsigned short xfIndex;
	unsigned int sstRecIndex;
} BIFF8_RECORD_LABELSST_STRUCT;

int biff8RecordLabelsstDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_LABELSST_RAW *record, BIFF8_RECORD_LABELSST_STRUCT *details);

// RK

#define BIFF8_RECORD_RK_SIZE 10

typedef struct {
	BIFF8_RECORD_HEADER_RAW header;
	BIFF8_RECORD_CELL_INDEX_RAW cellIndex;
	unsigned char xfRecIndex[2];
	unsigned char rkValue[4];
} BIFF8_RECORD_RK_RAW;

typedef struct {
	BIFF8_RECORD_CELL_INDEX_STRUCT cellIndex;
	int xfIndex;
	unsigned int rkValue;
} BIFF8_RECORD_RK_STRUCT;

int biff8RecordRkDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_RK_RAW *record, BIFF8_RECORD_RK_STRUCT *details);

// MULRK

#define BIFF8_RECORD_MULRK_MIN_SIZE 12

typedef struct {
	unsigned char xfRecIndex[2];
	unsigned char rkValue[4];
} BIFF8_RECORD_MULRK_ELEMENT;

#define BIFF8_RECORD_MULRK_ELEMENT_SIZE 6

typedef struct {
	BIFF8_RECORD_HEADER_RAW header;
	BIFF8_RECORD_CELL_INDEX_RAW cellIndex;
	unsigned char xfRkList[1];
} BIFF8_RECORD_MULRK_RAW;

typedef struct {
	BIFF8_RECORD_CELL_INDEX_STRUCT cellIndex;
	unsigned char *xfRkList;
	unsigned int xfRkListSize;
} BIFF8_RECORD_MULRK_STRUCT;

int biff8RecordMulrkDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_MULRK_RAW *record, BIFF8_RECORD_MULRK_STRUCT *details);

// NUMBER

#define BIFF8_RECORD_NUMBER_SIZE 14

typedef struct {
	BIFF8_RECORD_HEADER_RAW header;
	BIFF8_RECORD_CELL_INDEX_RAW cellIndex;
	unsigned char xfRecIndex[2];
	unsigned char fpValue[8];
} BIFF8_RECORD_NUMBER_RAW;

typedef struct {
	BIFF8_RECORD_CELL_INDEX_STRUCT cellIndex;
	int xfIndex;
	unsigned long long fpValue;
} BIFF8_RECORD_NUMBER_STRUCT;

int biff8RecordNumberDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_NUMBER_RAW *record, BIFF8_RECORD_NUMBER_STRUCT *details);

// FORMULA

#define BIFF8_RECORD_FORMULA_MIN_SIZE 21

typedef struct {
	BIFF8_RECORD_HEADER_RAW header;
	BIFF8_RECORD_CELL_INDEX_RAW cellIndex;
	unsigned char xfRecIndex[2];
	unsigned char result[8];
	unsigned char opts[2];
	unsigned char notUsed[4];
	unsigned char rpnTokenArray[1];
} BIFF8_RECORD_FORMULA_RAW;

typedef union {
	unsigned long long fpVal;
	struct {
		unsigned char id;
		unsigned char notUsed0;
		unsigned char val;
		unsigned char notUsed1[3];
		unsigned short uid;
	} otherVal;
} BIFF8_RECORD_FORMULA_RESULT;

#define BIFF8_RECORD_FORMULA_RESULT_UID 0xffff

typedef enum {
	BIFF8_RECORD_FORMULA_RESULT_STRING = 0x00,
	BIFF8_RECORD_FORMULA_RESULT_BOOLEAN = 0x01,
	BIFF8_RECORD_FORMULA_RESULT_ERROR = 0x02,
	BIFF8_RECORD_FORMULA_RESULT_EMPTY = 0x03,
	BIFF8_RECORD_FORMULA_RESULT_NUMBER = 0x04
} BIFF8_RECORD_FORMULA_RESULT_ID;

typedef struct {
	BIFF8_RECORD_CELL_INDEX_STRUCT cellIndex;
	int xfIndex;
	BIFF8_RECORD_FORMULA_RESULT_ID resultId;
	unsigned long long resultVal;
} BIFF8_RECORD_FORMULA_STRUCT;

int biff8RecordFormulaDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_FORMULA_RAW *record, BIFF8_RECORD_FORMULA_STRUCT *details);

// STRING

#define BIFF8_RECORD_STRING_STRING_LENGTH_SIZE 2

// BOOLERR

#define BIFF8_RECORD_BOOLERR_SIZE 9 // ??

typedef struct {
	BIFF8_RECORD_HEADER_RAW header;
	BIFF8_RECORD_CELL_INDEX_RAW cellIndex;
	unsigned char xfRecIndex[2];
	unsigned char value[1];
	unsigned char type[1];
} BIFF8_RECORD_BOOLERR_RAW;

typedef enum {
	BIFF8_RECORD_BOOLERR_TYPE_BOOLEAN = 0,
	BIFF8_RECORD_BOOLERR_TYPE_ERROR = 1
} BIFF8_RECORD_BOOLERR_TYPE;

typedef struct {
	BIFF8_RECORD_CELL_INDEX_STRUCT cellIndex;
	int xfIndex;
	BIFF8_RECORD_BOOLERR_TYPE type;
	unsigned char value;
} BIFF8_RECORD_BOOLERR_STRUCT;

int biff8RecordBoolerrDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_BOOLERR_RAW *record, BIFF8_RECORD_BOOLERR_STRUCT *details);

#endif /* BIFF8_RECORD_DETAILS_H_ */
