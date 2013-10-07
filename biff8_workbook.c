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
 * biff8_workbook.c
 *
 *  Created on: Jun 17, 2011
 *      Author: Billy
 */

#include "biff8_workbook.h"
#include "biff8_record_header.h"
#include "biff8_stream.h"
#include "char_to_uint.h"
#include "biff8_next_record.h"
#include <stdlib.h>

int biff8WorkbookLocate(COMPOUND_DOCUMENT_INFO *compDocInfo)
{
	if(!compDocInfo)
		return -1;
	
	if(!compDocInfo->dirEntryTbl || !compDocInfo->dirEntryTblSize)
		return -1;
	
	if(biff8StreamTypeGet(compDocInfo, 0) != BIFF8_STREAM_TYPE_ROOT_ENTRY)
		return -1;
	
	int i = 1;
	int workbook = 0;
	
	while(i < compDocInfo->dirEntryTblSize)
	{
		if(biff8StreamTypeGet(compDocInfo, i) == BIFF8_STREAM_TYPE_WORKBOOK)
		{
			if(workbook)
				return -1;
			
			workbook = i;
		}
		
		++i;
	}
	
	return (workbook ? workbook : -1);
}

void biff8WorkbookGlobalsInfoDestroy(BIFF8_WORKBOOK_GLOBALS *globals)
{
	if(!globals)
		return;

	biff8RecordDatemodeDetailsDestroy(&globals->dateMode);

	biff8RecordSstDetailsDestroy(&globals->sst);

	if(globals->format)
	{
		free(globals->format);
		globals->format = 0;
		globals->numFormat = 0;
	}

	if(globals->xf)
	{
		free(globals->xf);
		globals->xf = 0;
		globals->numXf = 0;
	}

	if(!globals->sheet || !globals->numSheets)
	{
		globals->numSheets = 0;
		return;
	}

	while(globals->numSheets)
		biff8RecordSheetDetailsDestroy(&globals->sheet[--globals->numSheets]);

	free(globals->sheet);
	globals->sheet = 0;
}

int biff8WorkbookGlobalsInfoBuild(COMPOUND_DOCUMENT_INFO *compDocInfo, unsigned char *workbook, unsigned int size, BIFF8_WORKBOOK_GLOBALS *globals)
{
	if(!compDocInfo || !workbook || !size || !globals)
		return -1;

	if(globals->sheet || globals->numSheets)
		return -1;

	BIFF8_RECORD_HEADER_RAW *header;
	BIFF8_RECORD_HEADER_STRUCT info = { .id = BIFF8_RECORD_BOF };
	int i = 0;
	enum { PHASE_BOF, PHASE_EOF } phase = PHASE_BOF;
	BIFF8_RECORD_BOF_STRUCT bof;
	int stop = 0;
	void *new;

	while(!stop && i < size)
	{
		if(phase == PHASE_BOF)
		{
			if(biff8RecordBofDetailsGet(compDocInfo, (BIFF8_RECORD_BOF_RAW *)&workbook[i], &bof))
				return -1;

			if(bof.type != BIFF8_RECORD_BOF_TYPE_WORKBOOK_GLOBALS)
				return -1;

			phase = PHASE_EOF;
		}
		else
		{
			header = (BIFF8_RECORD_HEADER_RAW *)&workbook[i];
			info.id = charToUint(compDocInfo->byteOrder, (unsigned char *)header->id, sizeof(header->id));
			info.dataSize = charToUint(compDocInfo->byteOrder, (unsigned char *)header->dataSize, sizeof(header->dataSize));
			
			switch(info.id)
			{
			case BIFF8_RECORD_BOF:
			case BIFF8_RECORD_EOF:
				stop = 1;
				break;

			case BIFF8_RECORD_SHEET:
				if((stop = !(new = realloc(globals->numSheets ? globals->sheet : 0, (globals->numSheets + 1) * sizeof(BIFF8_RECORD_SHEET_STRUCT)))))
					break;

				globals->sheet = (BIFF8_RECORD_SHEET_STRUCT *)new;
				globals->sheet[globals->numSheets].name = 0;

				++globals->numSheets;

				stop = biff8RecordSheetDetailsGet(compDocInfo, (BIFF8_RECORD_SHEET_RAW *)&workbook[i], &globals->sheet[globals->numSheets - 1]);
				break;

			case BIFF8_RECORD_SST:
				stop = biff8RecordSstDetailsGet(compDocInfo, (BIFF8_RECORD_SST_RAW *)&workbook[i], &globals->sst);
				break;

			case BIFF8_RECORD_DATEMODE:
				stop = biff8RecordDatemodeDetailsGet(compDocInfo, (BIFF8_RECORD_DATEMODE_RAW *)&workbook[i], &globals->dateMode);
				break;

			case BIFF8_RECORD_XF:
				if((stop = !(new = realloc(globals->xf, (globals->numXf + 1) * sizeof(BIFF8_RECORD_XF_STRUCT)))))
					break;

				globals->xf = (BIFF8_RECORD_XF_STRUCT *)new;

				stop = biff8RecordXfDetailsGet(compDocInfo, (BIFF8_RECORD_XF_RAW *)&workbook[i], &globals->xf[globals->numXf]);

				++globals->numXf;
				break;

			case BIFF8_RECORD_FORMAT:
				if((stop = !(new = realloc(globals->format, (globals->numFormat + 1) * sizeof(BIFF8_RECORD_FORMAT_STRUCT)))))
					break;

				globals->format = (BIFF8_RECORD_FORMAT_STRUCT *)new;

				stop = biff8RecordFormatDetailsGet(compDocInfo, (BIFF8_RECORD_FORMAT_RAW *)&workbook[i], &globals->format[globals->numFormat]);

				++globals->numFormat;
				break;

			default:
				break;
			}
		}

		i += biff8NextRecord(compDocInfo, &workbook[i]);
	}

	if(info.id == BIFF8_RECORD_EOF)
		return 0;

	biff8WorkbookGlobalsInfoDestroy(globals);
	return -1;
}


void biff8WorkbookGlobalsInfoPrint(COMPOUND_DOCUMENT_INFO *compDocInfo, unsigned char *workbook, unsigned int size)
{
	if(!compDocInfo || !workbook || !size)
		return;
	
	BIFF8_RECORD_HEADER_RAW *header;
	BIFF8_RECORD_HEADER_STRUCT info = { .id = BIFF8_RECORD_BOF };
	int i = 0;
	
	while(i < size && info.id != BIFF8_RECORD_EOF)
	{
		header = (BIFF8_RECORD_HEADER_RAW *)&workbook[i];
		info.id = charToUint(compDocInfo->byteOrder, (unsigned char *)header->id, sizeof(header->id));
		info.dataSize = charToUint(compDocInfo->byteOrder, (unsigned char *)header->dataSize, sizeof(header->dataSize));
		biff8RecordNamePrint(info.id);
		i += (BIFF8_RECORD_HEADER_SIZE + info.dataSize);
	}
}

void biff8WorkbookFormatStringFree(BIFF8_STRING **formatString)
{
	if(!formatString)
		return;

	biff8StringFree(formatString);
}

int biff8WorkbookFormatStringGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_WORKBOOK_GLOBALS *globals, unsigned int formatIndex, BIFF8_STRING **formatString)
{
	if(!globals || !formatString)
		return -1;

	if(*formatString)
		return -1;

	int i = 0;

	while(i < globals->numFormat)
	{
		if(globals->format[i].index == formatIndex)
		{

			BIFF8_STRING_INDEX stringIndex = BIFF8_STRING_INDEX_INITIALIZER((BIFF8_RECORD_HEADER_RAW *)globals->format[i].string, 0);

			if(biff8StringGet(compDocInfo, &stringIndex, BIFF8_RECORD_FORMAT_STRING_LENGTH_SIZE, formatString))
				return -1;

			if(biff8StringConvert(BIFF8_STRING_ENCODING_ASCII, *formatString, 0))
				return -1;

			return 0;
		}

		++i;
	}

	if(biff8StringAllocate(formatString, 1))
		return -1;

	*(*formatString)->string = 0;
	(*formatString)->numBytes = 0;
	return 0;
}
