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
 * biff8_worksheet.c
 *
 *  Created on: Jun 20, 2011
 *      Author: Billy
 */

#include "biff8_worksheet.h"
#include "biff8_next_record.h"
#include "biff8_cell.h"

#include "biff8_convert.h"

void biff8WorksheetInfoDestroy(BIFF8_WORKSHEET *worksheet)
{
	if(!worksheet)
		return;

	biff8RecordDimensionDetailsDestroy(&worksheet->dimension);
	biff8CellFree(&worksheet->dimension, &worksheet->cell);
}

#define ROW_BLOCK_MAX_ROWS 32

static int worksheetCellSet(BIFF8_WORKSHEET *worksheet, unsigned int row, unsigned int col, BIFF8_CELL_TYPE type, int xfIndex, unsigned long long arg)
{
	if(!worksheet)
		return -1;

	if(!worksheet->cell)
		return -1;

	if(row < worksheet->dimension.firstRowIndex || row > worksheet->dimension.lastRowIndex)
		return -1;

	if(col < worksheet->dimension.firstColIndex || col > worksheet->dimension.lastColIndex)
		return -1;

	row -= worksheet->dimension.firstRowIndex;
	col -= worksheet->dimension.firstColIndex;

	if(worksheet->cell[row][col].type != BIFF8_CELL_TYPE_BLANK)
		return -1;

	worksheet->cell[row][col].type = type;
	worksheet->cell[row][col].xfIndex = xfIndex;
	worksheet->cell[row][col].arg = arg;
	return 0;
}

int biff8WorksheetInfoBuild(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_WORKBOOK_GLOBALS *globals, unsigned char *workbook, unsigned int size, unsigned int sheet, BIFF8_WORKSHEET *worksheet)
{
	if(!compDocInfo || !globals || !workbook || !size || !worksheet)
		return -1;

	if(!globals->sheet || !globals->numSheets)
		return -1;

	if(worksheet->cell)
		return -1;

	if(sheet >= globals->numSheets)
		return -1;

	if(globals->sheet[sheet].type != BIFF8_RECORD_SHEET_TYPE_WORKSHEET)
		return -1;

	int error = 0;
	BIFF8_RECORD_HEADER_RAW *header;
	BIFF8_RECORD_HEADER_STRUCT info = { .id = BIFF8_RECORD_BOF };
	int i = globals->sheet[sheet].offset;
	int j;

	BIFF8_RECORD_DETAILS details;
	BIFF8_RECORD_LABELSST_STRUCT *labelsst = (BIFF8_RECORD_LABELSST_STRUCT *)&details;
	BIFF8_RECORD_RK_STRUCT *rk = (BIFF8_RECORD_RK_STRUCT *)&details;
	BIFF8_RECORD_MULRK_STRUCT *mulrk = (BIFF8_RECORD_MULRK_STRUCT *)&details;
	BIFF8_RECORD_NUMBER_STRUCT *number = (BIFF8_RECORD_NUMBER_STRUCT *)&details;
	BIFF8_RECORD_FORMULA_STRUCT *formula = (BIFF8_RECORD_FORMULA_STRUCT *)&details;
	BIFF8_RECORD_BOOLERR_STRUCT *boolerr = (BIFF8_RECORD_BOOLERR_STRUCT *)&details;

	BIFF8_CELL_TYPE cellType;

	//enum { RECORD_SEQ_BOF, RECORD_SEQ_DIMENSION, RECORD_SEQ_EOF	} recordSeq;

	for( ; !error && i < size && info.id != BIFF8_RECORD_EOF; i += biff8NextRecord(compDocInfo, &workbook[i]))
	{
		header = (BIFF8_RECORD_HEADER_RAW *)&workbook[i];
		info.id = charToUint(compDocInfo->byteOrder, (unsigned char *)header->id, sizeof(header->id));
		info.dataSize = charToUint(compDocInfo->byteOrder, (unsigned char *)header->dataSize, sizeof(header->dataSize));

		switch(info.id)
		{
		case BIFF8_RECORD_DIMENSION:
			if((error = biff8RecordDimensionDetailsGet(compDocInfo, (BIFF8_RECORD_DIMENSION_RAW *)&workbook[i], &worksheet->dimension)))
				break;

			if(!worksheet->dimension.numRows || !worksheet->dimension.numColumns)
				return 0;

			error = biff8CellAllocate(&worksheet->dimension, &worksheet->cell);
			break;

		case BIFF8_RECORD_LABELSST:
			if((error = biff8RecordLabelsstDetailsGet(compDocInfo, (BIFF8_RECORD_LABELSST_RAW *)&workbook[i], labelsst)))
				break;

			error = worksheetCellSet(worksheet, labelsst->cellIndex.row, labelsst->cellIndex.col, BIFF8_CELL_TYPE_SST_STRING, labelsst->xfIndex, labelsst->sstRecIndex);
			break;

		case BIFF8_RECORD_RK:
			if((error = biff8RecordRkDetailsGet(compDocInfo, (BIFF8_RECORD_RK_RAW *)&workbook[i], rk)))
				break;

			error = worksheetCellSet(worksheet, rk->cellIndex.row, rk->cellIndex.col, BIFF8_CELL_TYPE_RK, rk->xfIndex, rk->rkValue);
			break;

		case BIFF8_RECORD_MULRK:
			if((error = biff8RecordMulrkDetailsGet(compDocInfo, (BIFF8_RECORD_MULRK_RAW *)&workbook[i], mulrk)))
				break;

			for(j = 0; j < mulrk->xfRkListSize; j++)
			{
				if((error = worksheetCellSet(worksheet, mulrk->cellIndex.row, mulrk->cellIndex.col + j, BIFF8_CELL_TYPE_RK, charToUint(compDocInfo->byteOrder, ((BIFF8_RECORD_MULRK_ELEMENT *)&mulrk->xfRkList[j * BIFF8_RECORD_MULRK_ELEMENT_SIZE])->xfRecIndex, sizeof(unsigned short)), charToUint(compDocInfo->byteOrder, ((BIFF8_RECORD_MULRK_ELEMENT *)&mulrk->xfRkList[j * BIFF8_RECORD_MULRK_ELEMENT_SIZE])->rkValue, sizeof(unsigned int)))))
					break;
			}

			break;

		case BIFF8_RECORD_NUMBER:
			if((error = biff8RecordNumberDetailsGet(compDocInfo, (BIFF8_RECORD_NUMBER_RAW *)&workbook[i], number)))
				break;

			error = worksheetCellSet(worksheet, number->cellIndex.row, number->cellIndex.col, BIFF8_CELL_TYPE_NUMBER, number->xfIndex, number->fpValue);
			break;

		case BIFF8_RECORD_FORMULA:
			if((error = biff8RecordFormulaDetailsGet(compDocInfo, (BIFF8_RECORD_FORMULA_RAW *)&workbook[i], formula)))
				break;

			switch(formula->resultId)
			{
			case BIFF8_RECORD_FORMULA_RESULT_STRING:
				formula->resultVal = (unsigned long long)&workbook[i + biff8NextRecord(compDocInfo, &workbook[i]) + BIFF8_RECORD_HEADER_SIZE];
				cellType = BIFF8_CELL_TYPE_STRING;
				break;
			case BIFF8_RECORD_FORMULA_RESULT_BOOLEAN:
				cellType = BIFF8_CELL_TYPE_BOOLEAN;
				break;
			case BIFF8_RECORD_FORMULA_RESULT_ERROR:
				cellType = BIFF8_CELL_TYPE_ERROR;
				break;
			case BIFF8_RECORD_FORMULA_RESULT_EMPTY:
				continue;
			default:
				cellType = BIFF8_CELL_TYPE_NUMBER;
				break;
			}

			error = worksheetCellSet(worksheet, formula->cellIndex.row, formula->cellIndex.col, cellType, formula->xfIndex, formula->resultVal);
			break;

		case BIFF8_RECORD_BOOLERR:
			if((error = biff8RecordBoolerrDetailsGet(compDocInfo, (BIFF8_RECORD_BOOLERR_RAW *)&workbook[i], boolerr)))
				break;

			error = worksheetCellSet(worksheet, boolerr->cellIndex.row, boolerr->cellIndex.col, (boolerr->type == BIFF8_RECORD_BOOLERR_TYPE_BOOLEAN) ? BIFF8_CELL_TYPE_BOOLEAN : BIFF8_CELL_TYPE_ERROR, boolerr->xfIndex, boolerr->value);
			break;

		case BIFF8_RECORD_LABEL:
		case BIFF8_RECORD_RSTRING:
			printf("warning: lost data -- ");
			biff8RecordNamePrint(info.id);
			break;

		default:
			break;
		}
	}

	if(info.id == BIFF8_RECORD_EOF)
		return 0;

	biff8WorksheetInfoDestroy(worksheet);
	return -1;
}

void biff8WorksheetInfoPrint(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_WORKBOOK_GLOBALS *globals, unsigned char *workbook, unsigned int size, unsigned int sheet)
{
	if(!compDocInfo || !globals || !workbook || !size)
		return;

	if(!globals->sheet || !globals->numSheets)
		return;

	if(sheet >= globals->numSheets)
		return;

	if(globals->sheet[sheet].type != BIFF8_RECORD_SHEET_TYPE_WORKSHEET)
		return;

	printf("\"%s\" records:\n", globals->sheet[sheet].name->string);

	BIFF8_RECORD_HEADER_RAW *header;
	BIFF8_RECORD_HEADER_STRUCT info = { .id = BIFF8_RECORD_BOF };
	int i = globals->sheet[sheet].offset;

	while(i < size && info.id != BIFF8_RECORD_EOF)
	{
		header = (BIFF8_RECORD_HEADER_RAW *)&workbook[i];
		info.id = charToUint(compDocInfo->byteOrder, (unsigned char *)header->id, sizeof(header->id));
		info.dataSize = charToUint(compDocInfo->byteOrder, (unsigned char *)header->dataSize, sizeof(header->dataSize));
		biff8RecordNamePrint(info.id);
		i += (BIFF8_RECORD_HEADER_SIZE + info.dataSize);
	}

	printf("\n");
}
