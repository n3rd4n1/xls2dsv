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
 * biff8_convert.c
 *
 *  Created on: Jun 23, 2011
 *      Author: Billy
 */

#include "biff8_convert.h"
#include "biff8_record_details.h"
#include "biff8_error_codes.h"
#include "biff8_format_string.h"
#include <string.h>

static int blankToStringConvert(BIFF8_CELL *cell, BIFF8_STRING **string)
{
	if(!cell || !string)
		return -1;

	if(*string)
		return -1;

	if(cell->type != BIFF8_CELL_TYPE_BLANK)
		return -1;

	if(biff8StringAllocate(string, 1))
		return -1;

	(*string)->numBytes = 0;
	*(*string)->string = 0;
	(*string)->encoding = BIFF8_STRING_ENCODING_UTF8;
	return 0;
}

static int stringGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_WORKBOOK_GLOBALS *globals, unsigned int stringLengthSize, BIFF8_CELL *cell, BIFF8_FORMAT_STRING *formatString, BIFF8_STRING **string)
{
	if(!compDocInfo || !cell || !string)
		return -1;

	if(formatString && formatString->text)
	{
		char buffer[100];
		BIFF8_FORMAT_STRING_DECODE ret;

		if((ret = biff8FormatStringDecode(0, formatString->text, 0, buffer)) == BIFF8_FORMAT_STRING_DECODE_FAILED)
			return -1;

		if(ret == BIFF8_FORMAT_STRING_DECODE_DONE)
		{
			int len = strlen(buffer);

			if(biff8StringAllocate(string, len + 1))
				return -1;

			(*string)->numBytes = len;
			strcpy((char *)(*string)->string, buffer);
			(*string)->encoding = BIFF8_STRING_ENCODING_UTF8;
			return 0;
		}
	}

	BIFF8_STRING_INDEX index = BIFF8_STRING_INDEX_INITIALIZER((BIFF8_RECORD_HEADER_RAW *)cell->arg, 0);
	BIFF8_STRING_INDEX *stringIndex;

	if(globals)
	{
		if(cell->arg >= globals->sst.numStrings)
			return -1;

		stringIndex = &globals->sst.index[cell->arg];
	}
	else
		stringIndex = &index;

	return biff8StringGet(compDocInfo, stringIndex, stringLengthSize, string);
}

static int fpToStringConvert(BIFF8_WORKBOOK_GLOBALS *globals, double value, BIFF8_FORMAT_STRING *formatString, BIFF8_STRING **string)
{
	if(!string)
		return -1;

	if(*string)
		return -1;
#if 0
	if(formatString)
	{
		if(formatString->pos)
		{
			printf("%s (%.lf) %ld %lf\n", formatString->pos, value, (long)value, value - (double)(long)value);
			char buffer[1000];
			biff8FormatStringDecode(globals, formatString->pos, value, buffer);
			printf(">>>> %s\n", buffer);
		}
	}
#endif

	char str[100];
	//int len = sprintf(str, "%.14lE/%lf", value,value);
	int len = sprintf(str, "%lf", value);

	if(len < 3)
		return -1;

	while(--len && (str[len] == '0'));

	if(!len)
		return -1;

	str[len += (str[len] != '.')] = 0;

	if(biff8StringAllocate(string, len + 1))
		return -1;

	(*string)->numBytes = len;
	strcpy((char *)(*string)->string, str);
	(*string)->encoding = BIFF8_STRING_ENCODING_UTF8;
	return 0;
}

static int rkToStringConvert(BIFF8_WORKBOOK_GLOBALS *globals, BIFF8_CELL *cell, BIFF8_FORMAT_STRING *formatString, BIFF8_STRING **string)
{
	if(!cell || !string)
		return -1;

	if(*string)
		return -1;

	if(cell->type != BIFF8_CELL_TYPE_RK)
		return -1;

	double val;

	if((cell->arg & 0x2))
		val = cell->arg >> 2;
	else
		*(long long *)&val = (long long)(cell->arg & ~0x3) << 32;

	if((cell->arg & 0x1))
		val /= 100.0;

	return fpToStringConvert(globals, val, formatString, string);
}

static int numberToStringConvert(BIFF8_WORKBOOK_GLOBALS *globals, BIFF8_CELL *cell, BIFF8_FORMAT_STRING *formatString, BIFF8_STRING **string)
{
	if(!cell || !string)
		return -1;

	if(*string)
		return -1;

	if(cell->type != BIFF8_CELL_TYPE_NUMBER)
		return -1;

	return fpToStringConvert(globals, *(double *)&cell->arg, formatString, string);
}

static int booleanToStringConvert(BIFF8_CELL *cell, BIFF8_STRING **string)
{
	if(!cell || !string)
		return -1;

	if(*string)
		return -1;

	if(cell->type != BIFF8_CELL_TYPE_BOOLEAN)
		return -1;

	char bool[6];

	strcpy(bool, cell->arg ? "TRUE" : "FALSE");

	int len = strlen(bool);

	if(biff8StringAllocate(string, len + 1))
		return -1;

	(*string)->numBytes = len;
	strcpy((char *)(*string)->string, bool);
	(*string)->encoding = BIFF8_STRING_ENCODING_UTF8;
	return 0;
}

static int errorToStringConvert(BIFF8_CELL *cell, BIFF8_STRING **string)
{
	if(!cell || !string)
		return -1;

	if(*string)
		return -1;

	if(cell->type != BIFF8_CELL_TYPE_ERROR)
		return -1;

	char error[10];

	switch(cell->arg)
	{
	case BIFF8_ERROR_CODE_NULL:
		strcpy(error, BIFF8_ERROR_CODE_STRING_NULL);
		break;
	case BIFF8_ERROR_CODE_DIV0:
		strcpy(error, BIFF8_ERROR_CODE_STRING_DIV0);
		break;
	case BIFF8_ERROR_CODE_VALUE:
		strcpy(error, BIFF8_ERROR_CODE_STRING_VALUE);
		break;
	case BIFF8_ERROR_CODE_REF:
		strcpy(error, BIFF8_ERROR_CODE_STRING_REF);
		break;
	case BIFF8_ERROR_CODE_NAME:
		strcpy(error, BIFF8_ERROR_CODE_STRING_NAME);
		break;
	case BIFF8_ERROR_CODE_NUM:
		strcpy(error, BIFF8_ERROR_CODE_STRING_NUM);
		break;
	case BIFF8_ERROR_CODE_NA:
		strcpy(error, BIFF8_ERROR_CODE_STRING_NA);
		break;
	default:
		return -1;
	}

	int len = strlen(error);

	if(biff8StringAllocate(string, len + 1))
		return -1;

	(*string)->numBytes = len;
	strcpy((char *)(*string)->string, error);
	(*string)->encoding = BIFF8_STRING_ENCODING_UTF8;
	return 0;
}

void biff8ConvertStringFree(BIFF8_STRING **string)
{
	biff8StringFree(string);
}

int biff8ConvertCellToString(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_WORKBOOK_GLOBALS *globals, BIFF8_CELL *cell, BIFF8_STRING **string)
{
	if(!compDocInfo || !globals || !cell || !string)
		return -1;

	if(*string)
		return -1;

	if(cell->type == BIFF8_CELL_TYPE_BLANK)
		return blankToStringConvert(cell, string);

	if(globals->xf[cell->xfIndex].type != BIFF8_RECORD_XF_TYPE_CELL)
		return -1;

	BIFF8_FORMAT_STRING formatString = BIFF8_FORMAT_STRING_INITIALIZER;

	if(biff8WorkbookFormatStringGet(compDocInfo, globals, globals->xf[cell->xfIndex].index, string))
		return -1;

	int ret = biff8FormatStringBuild(*string, &formatString);

	biff8WorkbookFormatStringFree(string);

	if(ret)
		return -1;

	switch(cell->type)
	{
	case BIFF8_CELL_TYPE_SST_STRING:
		ret = stringGet(compDocInfo, globals, BIFF8_RECORD_LABELSST_STRING_LENGTH_SIZE, cell, &formatString, string);
		break;

	case BIFF8_CELL_TYPE_RK:
		ret = rkToStringConvert(globals, cell, &formatString, string);
		break;

	case BIFF8_CELL_TYPE_NUMBER:
		ret = numberToStringConvert(globals, cell, &formatString, string);
		break;

	case BIFF8_CELL_TYPE_STRING:
		ret = stringGet(compDocInfo, 0, BIFF8_RECORD_STRING_STRING_LENGTH_SIZE, cell, &formatString, string);
		break;

	case BIFF8_CELL_TYPE_BOOLEAN:
		ret = booleanToStringConvert(cell, string);
		break;

	case BIFF8_CELL_TYPE_ERROR:
		ret = errorToStringConvert(cell, string);
		break;

	default:
		ret = -1;
		break;
	}

	biff8FormatStringDestroy(&formatString);
	return ret;
}
