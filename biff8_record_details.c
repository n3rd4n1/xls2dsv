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
 * biff8_record_details.c
 *
 *  Created on: Jun 20, 2011
 *      Author: Billy
 */

#include "biff8_record_details.h"
#include "biff8_records.h"
#include "char_to_uint.h"
#include <stdlib.h>

int biff8RecordBofDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_BOF_RAW *record, BIFF8_RECORD_BOF_STRUCT *details)
{
	if(!compDocInfo || !record || !details)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.id, sizeof(record->header.id)) != BIFF8_RECORD_BOF)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.dataSize, sizeof(record->header.dataSize)) != BIFF8_RECORD_BOF_SIZE)
		return -1;

	if((details->biffVersion = charToUint(compDocInfo->byteOrder, record->biffVersion, sizeof(record->biffVersion))) != BIFF8_RECORD_BOF_BIFF_VERSION)
		return -1;

	details->type = charToUint(compDocInfo->byteOrder, record->type, sizeof(record->type));
	details->buildId = charToUint(compDocInfo->byteOrder, record->buildId, sizeof(record->buildId));
	details->buildYear = charToUint(compDocInfo->byteOrder, record->buildYear, sizeof(record->buildYear));
	details->fileHistoryFlags = charToUint(compDocInfo->byteOrder, record->fileHistoryFlags, sizeof(record->fileHistoryFlags));
	details->compatibility = charToUint(compDocInfo->byteOrder, record->compatibility, sizeof(record->compatibility));
	return 0;
}

void biff8RecordSheetDetailsDestroy(BIFF8_RECORD_SHEET_STRUCT *details)
{
	if(!details)
		return;

	if(details->name)
	{
		biff8StringFree(&details->name);
		details->name = 0;
	}
}

int biff8RecordSheetDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_SHEET_RAW *record, BIFF8_RECORD_SHEET_STRUCT *details)
{
	if(!compDocInfo || !record || !details)
		return -1;

	if(details->name)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.id, sizeof(record->header.id)) != BIFF8_RECORD_SHEET)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.dataSize, sizeof(record->header.dataSize)) < BIFF8_RECORD_SHEET_MIN_SIZE)
		return -1;

	BIFF8_STRING_INDEX index = BIFF8_STRING_INDEX_INITIALIZER(&record->header, (unsigned int)((void *)record->name - (void *)record));

	if(biff8StringGet(compDocInfo, &index, BIFF8_RECORD_SHEET_STRING_LENGTH_SIZE, &details->name))
		return -1;

	if(biff8StringConvert(BIFF8_STRING_ENCODING_UTF8, details->name, 0))
	{
		biff8RecordSheetDetailsDestroy(details);
		return -1;
	}

	details->offset = charToUint(compDocInfo->byteOrder, record->offset, sizeof(record->offset));
	details->state = charToUint(compDocInfo->byteOrder, record->state, sizeof(record->state));
	details->type = charToUint(compDocInfo->byteOrder, record->type, sizeof(record->type));
	return 0;
}

void biff8RecordSstDetailsDestroy(BIFF8_RECORD_SST_STRUCT *details)
{
	if(!details)
		return;

	if(details->index)
	{
		free(details->index);
		details->index = 0;
	}
}

int biff8RecordSstDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_SST_RAW *record, BIFF8_RECORD_SST_STRUCT *details)
{
	if(!compDocInfo || !record || !details)
		return -1;

	if(details->index)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.id, sizeof(record->header.id)) != BIFF8_RECORD_SST)
		return -1;

	unsigned int dataSize = charToUint(compDocInfo->byteOrder, record->header.dataSize, sizeof(record->header.dataSize));

	if(dataSize < BIFF8_RECORD_SST_MIN_SIZE)
		return -1;

	details->totalStrings = charToUint(compDocInfo->byteOrder, record->totalStrings, sizeof(record->totalStrings));
	details->numStrings = charToUint(compDocInfo->byteOrder, record->numStrings, sizeof(record->numStrings));

	if(details->numStrings)
	{
		if(!(details->index = (BIFF8_STRING_INDEX *)calloc(details->numStrings, sizeof(BIFF8_STRING_INDEX))))
			return -1;

		details->index[0].owner = &record->header;
		details->index[0].index = (unsigned int)((void *)record->base - (void *)record);
		details->index[0].split.valid = 0;

		int i = 1;

		while(i < details->numStrings)
		{
			if(biff8StringIndexArrayNextGet(compDocInfo, BIFF8_RECORD_SST_STRING_LENGTH_SIZE, &details->index[i - 1], &details->index[i]))
			{
				biff8RecordSstDetailsDestroy(details);
				return -1;
			}

			++i;
		}
	}

	return 0;
}

void biff8RecordDatemodeDetailsDestroy(BIFF8_RECORD_DATEMODE_STRUCT *details)
{
	if(!details)
		return;

	details->valid = 0;
}

int biff8RecordDatemodeDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_DATEMODE_RAW *record, BIFF8_RECORD_DATEMODE_STRUCT *details)
{
	if(!compDocInfo || !record || !details)
		return -1;

	if(details->valid)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.id, sizeof(record->header.id)) != BIFF8_RECORD_DATEMODE)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.dataSize, sizeof(record->header.dataSize)) != BIFF8_RECORD_DATEMODE_SIZE)
		return -1;

	int base = charToUint(compDocInfo->byteOrder, record->base, sizeof(record->base));

	if(base)
	{
		details->baseYear = 1904;
		details->baseMonth = 1;
		details->baseDay = 1;
	}
	else
	{
		details->baseYear = 1899;
		details->baseMonth = 12;
		details->baseDay = 30;
	}

	details->valid = 1;
	return 0;
}

void biff8RecordDimensionDetailsDestroy(BIFF8_RECORD_DIMENSION_STRUCT *details)
{
	if(!details)
		return;

	details->valid = 0;
}

int biff8RecordDimensionDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_DIMENSION_RAW *record, BIFF8_RECORD_DIMENSION_STRUCT *details)
{
	if(!compDocInfo || !record || !details)
		return -1;

	if(details->valid)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.id, sizeof(record->header.id)) != BIFF8_RECORD_DIMENSION)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.dataSize, sizeof(record->header.dataSize)) != BIFF8_RECORD_DIMENSION_SIZE)
		return -1;

	details->firstRowIndex = charToUint(compDocInfo->byteOrder, record->firstRowIndex, sizeof(record->firstRowIndex));
	details->lastRowIndex = charToUint(compDocInfo->byteOrder, record->lastRowIndex, sizeof(record->lastRowIndex));

	if(details->lastRowIndex < details->firstRowIndex)
		return -1;

	details->numRows = details->lastRowIndex - details->firstRowIndex;
	details->lastRowIndex -= !!details->numRows;

	details->firstColIndex = charToUint(compDocInfo->byteOrder, record->firstColIndex, sizeof(record->firstColIndex));
	details->lastColIndex = charToUint(compDocInfo->byteOrder, record->lastColIndex, sizeof(record->lastColIndex));

	if(details->lastColIndex < details->firstColIndex)
		return -1;

	details->numColumns = details->lastColIndex - details->firstColIndex;
	details->lastColIndex -= !!details->numColumns;

	details->valid = 1;
	return 0;
}

int biff8RecordXfDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_XF_RAW *record, BIFF8_RECORD_XF_STRUCT *details)
{
	if(!compDocInfo || !record || !details)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.id, sizeof(record->header.id)) != BIFF8_RECORD_XF)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.dataSize, sizeof(record->header.dataSize)) != BIFF8_RECORD_XF_SIZE)
		return -1;

	int formatRecIndex = charToUint(compDocInfo->byteOrder, record->formatRecIndex, sizeof(record->formatRecIndex));
	int xfType = charToUint(compDocInfo->byteOrder, record->xfType, sizeof(record->xfType));

	details->type = BIFF8_RECORD_XF_TYPE_GET(xfType);
	details->use = BIFF8_RECORD_XF_USE_GET(charToUint(compDocInfo->byteOrder, record->xfUsedAttrib, sizeof(record->xfUsedAttrib)));

	if(details->type == BIFF8_RECORD_XF_TYPE_CELL)
		details->index = details->use ? formatRecIndex : BIFF8_RECORD_XF_PARENT_INDEX_GET(xfType);
	else
		details->index = details->use ? -1 : 0;

	return 0;
}

int biff8RecordFormatDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_FORMAT_RAW *record, BIFF8_RECORD_FORMAT_STRUCT *details)
{
	if(!compDocInfo || !record || !details)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.id, sizeof(record->header.id)) != BIFF8_RECORD_FORMAT)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.dataSize, sizeof(record->header.dataSize)) < BIFF8_RECORD_FORMAT_MIN_SIZE)
		return -1;

	details->index = charToUint(compDocInfo->byteOrder, record->index, sizeof(record->index));
	details->string = record->string;

	return 0;
}

int biff8RecordLabelsstDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_LABELSST_RAW *record, BIFF8_RECORD_LABELSST_STRUCT *details)
{
	if(!compDocInfo || !record || !details)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.id, sizeof(record->header.id)) != BIFF8_RECORD_LABELSST)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.dataSize, sizeof(record->header.dataSize)) != BIFF8_RECORD_LABELSST_SIZE)
		return -1;

	details->cellIndex.row = charToUint(compDocInfo->byteOrder, record->cellIndex.row, sizeof(record->cellIndex.row));
	details->cellIndex.col = charToUint(compDocInfo->byteOrder, record->cellIndex.col, sizeof(record->cellIndex.col));
	details->xfIndex = charToUint(compDocInfo->byteOrder, record->xfRecIndex, sizeof(record->xfRecIndex));

	details->sstRecIndex = charToUint(compDocInfo->byteOrder, record->sstRecIndex, sizeof(record->sstRecIndex));

	return 0;
}

int biff8RecordRkDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_RK_RAW *record, BIFF8_RECORD_RK_STRUCT *details)
{
	if(!compDocInfo || !record || !details)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.id, sizeof(record->header.id)) != BIFF8_RECORD_RK)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.dataSize, sizeof(record->header.dataSize)) != BIFF8_RECORD_RK_SIZE)
		return -1;

	details->cellIndex.row = charToUint(compDocInfo->byteOrder, record->cellIndex.row, sizeof(record->cellIndex.row));
	details->cellIndex.col = charToUint(compDocInfo->byteOrder, record->cellIndex.col, sizeof(record->cellIndex.col));
	details->xfIndex = charToUint(compDocInfo->byteOrder, record->xfRecIndex, sizeof(record->xfRecIndex));

	details->rkValue = charToUint(compDocInfo->byteOrder, record->rkValue, sizeof(record->rkValue));

	return 0;
}

int biff8RecordMulrkDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_MULRK_RAW *record, BIFF8_RECORD_MULRK_STRUCT *details)
{
	if(!compDocInfo || !record || !details)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.id, sizeof(record->header.id)) != BIFF8_RECORD_MULRK)
		return -1;

	int dataSize = charToUint(compDocInfo->byteOrder, record->header.dataSize, sizeof(record->header.dataSize));

	if(dataSize < BIFF8_RECORD_MULRK_MIN_SIZE)
		return -1;

	details->cellIndex.row = charToUint(compDocInfo->byteOrder, record->cellIndex.row, sizeof(record->cellIndex.row));
	details->cellIndex.col = charToUint(compDocInfo->byteOrder, record->cellIndex.col, sizeof(record->cellIndex.col));

	details->xfRkList = record->xfRkList;
	details->xfRkListSize = (dataSize - sizeof(record->cellIndex) - 2) / BIFF8_RECORD_MULRK_ELEMENT_SIZE;

	return 0;
}

int biff8RecordNumberDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_NUMBER_RAW *record, BIFF8_RECORD_NUMBER_STRUCT *details)
{
	if(!compDocInfo || !record || !details)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.id, sizeof(record->header.id)) != BIFF8_RECORD_NUMBER)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.dataSize, sizeof(record->header.dataSize)) != BIFF8_RECORD_NUMBER_SIZE)
		return -1;

	details->cellIndex.row = charToUint(compDocInfo->byteOrder, record->cellIndex.row, sizeof(record->cellIndex.row));
	details->cellIndex.col = charToUint(compDocInfo->byteOrder, record->cellIndex.col, sizeof(record->cellIndex.col));
	details->xfIndex = charToUint(compDocInfo->byteOrder, record->xfRecIndex, sizeof(record->xfRecIndex));

	details->fpValue = charToUint(compDocInfo->byteOrder, record->fpValue, sizeof(record->fpValue));

	return 0;
}

int biff8RecordFormulaDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_FORMULA_RAW *record, BIFF8_RECORD_FORMULA_STRUCT *details)
{
	if(!compDocInfo || !record || !details)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.id, sizeof(record->header.id)) != BIFF8_RECORD_FORMULA)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.dataSize, sizeof(record->header.dataSize)) < BIFF8_RECORD_FORMULA_MIN_SIZE)
		return -1;

	details->cellIndex.row = charToUint(compDocInfo->byteOrder, record->cellIndex.row, sizeof(record->cellIndex.row));
	details->cellIndex.col = charToUint(compDocInfo->byteOrder, record->cellIndex.col, sizeof(record->cellIndex.col));
	details->xfIndex = charToUint(compDocInfo->byteOrder, record->xfRecIndex, sizeof(record->xfRecIndex));

	BIFF8_RECORD_FORMULA_RESULT result;

	result.fpVal = charToUint(compDocInfo->byteOrder, record->result, sizeof(record->result));

	if(result.otherVal.uid == BIFF8_RECORD_FORMULA_RESULT_UID)
	{
		switch(result.otherVal.id)
		{
		case BIFF8_RECORD_FORMULA_RESULT_BOOLEAN:
		case BIFF8_RECORD_FORMULA_RESULT_ERROR:
			details->resultVal = result.otherVal.val;
		case BIFF8_RECORD_FORMULA_RESULT_STRING:
		case BIFF8_RECORD_FORMULA_RESULT_EMPTY:
			details->resultId = result.otherVal.id;
			return 0;

		default:
			break;
		}
	}

	details->resultId = BIFF8_RECORD_FORMULA_RESULT_NUMBER;
	details->resultVal = result.fpVal;

	return 0;
}

int biff8RecordBoolerrDetailsGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_RECORD_BOOLERR_RAW *record, BIFF8_RECORD_BOOLERR_STRUCT *details)
{
	if(!compDocInfo || !record || !details)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.id, sizeof(record->header.id)) != BIFF8_RECORD_BOOLERR)
		return -1;

	if(charToUint(compDocInfo->byteOrder, record->header.dataSize, sizeof(record->header.dataSize)) != BIFF8_RECORD_BOOLERR_SIZE)
		return -1;

	details->cellIndex.row = charToUint(compDocInfo->byteOrder, record->cellIndex.row, sizeof(record->cellIndex.row));
	details->cellIndex.col = charToUint(compDocInfo->byteOrder, record->cellIndex.col, sizeof(record->cellIndex.col));
	details->xfIndex = charToUint(compDocInfo->byteOrder, record->xfRecIndex, sizeof(record->xfRecIndex));

	details->type = *record->type;
	details->value = *record->value;

	return 0;
}

