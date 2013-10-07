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
 * biff8_worksheet_save.c
 *
 *  Created on: Jun 23, 2011
 *      Author: Billy
 */

#include "biff8_worksheet_save.h"
#include "biff8_convert.h"

static void specialCharsFree(BIFF8_STRING **charDelimiter, BIFF8_STRING **charTextDelimiter, BIFF8_STRING **charCr, BIFF8_STRING **charLf)
{
	biff8StringFree(charDelimiter);
	biff8StringFree(charTextDelimiter);
	biff8StringFree(charCr);
	biff8StringFree(charLf);
}

#define CHAR_CR 0x0d
#define CHAR_LF 0x0a

static inline int specialCharsAlloc(BIFF8_STRING_ENCODING encoding, BIFF8_STRING **charDelimiter, BIFF8_STRING **charTextDelimiter, BIFF8_STRING **charCr, BIFF8_STRING **charLf, char delimiter, char textDelimiter)
{
	*charDelimiter = 0;
	*charTextDelimiter = 0;
	*charCr = 0;
	*charLf = 0;

	do
	{
		if(biff8StringAllocate(charDelimiter, 1))
			break;

		*(*charDelimiter)->string = delimiter;
		(*charDelimiter)->numBytes = 1;
		(*charDelimiter)->encoding = BIFF8_STRING_ENCODING_UTF8;

		if(biff8StringConvert(encoding, *charDelimiter, 0))
			break;

		if(biff8StringAllocate(charTextDelimiter, 1))
			break;

		*(*charTextDelimiter)->string = textDelimiter;
		(*charTextDelimiter)->numBytes = 1;
		(*charTextDelimiter)->encoding = BIFF8_STRING_ENCODING_UTF8;

		if(biff8StringConvert(encoding, *charTextDelimiter, 0))
			break;

		if(biff8StringAllocate(charCr, 1))
			break;

		*(*charCr)->string = CHAR_CR;
		(*charCr)->numBytes = 1;
		(*charCr)->encoding = BIFF8_STRING_ENCODING_UTF8;

		if(biff8StringConvert(encoding, *charCr, 0))
			break;

		if(biff8StringAllocate(charLf, 1))
			break;

		*(*charLf)->string = CHAR_LF;
		(*charLf)->numBytes = 1;
		(*charLf)->encoding = BIFF8_STRING_ENCODING_UTF8;

		if(biff8StringConvert(encoding, *charLf, 0))
			break;

		return 0;
	}
	while(0);

	specialCharsFree(charDelimiter, charTextDelimiter, charCr, charLf);
	return -1;
}

static int insertChar(FILE *file, BIFF8_STRING *string, int *flag)
{
	if(*flag)
	{
		if(fwrite(string->string, sizeof(unsigned char), string->numBytes, file) != string->numBytes)
			return -1;

		--*flag;
	}

	return 0;
}

int biff8WorksheetSaveToFile(FILE *file, COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_WORKBOOK_GLOBALS *globals, BIFF8_WORKSHEET *worksheet, char delimiter, char textDelimiter, BIFF8_STRING_ENCODING encoding, char replacement, int crlf, int enclose, BIFF8_WORKSHEET_SAVE_MODE saveMode)
{
	if(!file || !compDocInfo || !globals || !worksheet)
		return -1;

	if(delimiter < 0 || textDelimiter < 0)
		return -1;

	if(!worksheet->dimension.numRows && !worksheet->dimension.numColumns)
		return 0;

	if(!worksheet->cell)
		return -1;

	if(!worksheet->dimension.numRows || !worksheet->dimension.numColumns)
		return -1;

	int firstRow = 0;
	int firstCol = 0;
	int lastRow = worksheet->dimension.numRows;
	int lastCol = worksheet->dimension.numColumns;
	int i, j;

	if(saveMode == BIFF8_WORKSHEET_SAVE_COMPACT_1 || saveMode == BIFF8_WORKSHEET_SAVE_COMPACT_2)
	{
		while(firstRow < lastRow)
		{
			for(i = 0; i < lastCol; i++)
			{
				if(worksheet->cell[firstRow][i].type != BIFF8_CELL_TYPE_BLANK)
					break;
			}

			if(i < lastCol)
				break;

			++firstRow;
		}

		if(firstRow == lastRow)
			return 0;

		while(firstCol)
		{
			for(i = 0; i < lastRow; i++)
			{
				if(worksheet->cell[i][firstCol].type != BIFF8_CELL_TYPE_BLANK)
					break;
			}

			if(i < lastRow)
				break;

			++firstCol;
		}

		firstRow += worksheet->dimension.firstRowIndex;
		firstCol += worksheet->dimension.firstColIndex;
	}

	if(saveMode != BIFF8_WORKSHEET_SAVE_PRESERVE)
	{
		while(lastRow)
		{
			for(i = 0; i < lastCol; i++)
			{
				if(worksheet->cell[lastRow - 1][i].type != BIFF8_CELL_TYPE_BLANK)
					break;
			}

			if(i < lastCol)
				break;

			--lastRow;
		}

		if(!lastRow)
			return 0;

		while(lastCol)
		{
			for(i = 0; i < lastRow; i++)
			{
				if(worksheet->cell[i][lastCol - 1].type != BIFF8_CELL_TYPE_BLANK)
					break;
			}

			if(i < lastRow)
				break;

			--lastCol;
		}
	}

	BIFF8_STRING *charDelimiter;
	BIFF8_STRING *charTextDelimiter;
	BIFF8_STRING *charCr;
	BIFF8_STRING *charLf;

	if(specialCharsAlloc(encoding, &charDelimiter, &charTextDelimiter, &charCr, &charLf, delimiter, textDelimiter))
		return -1;

	BIFF8_STRING *string = 0;
	int k = -1, l = -1;

	lastRow += worksheet->dimension.firstRowIndex;
	lastCol += worksheet->dimension.firstColIndex;

	int putDelimiter;
	int newRow = 0;
	int putTextDelimiter;
	int textDelimiterFound = 0;

	unsigned int charIndex;
	unsigned int charSize;

	for(i = firstRow; i < lastRow; i++)
	{
		if(i >= worksheet->dimension.firstRowIndex)
		{
			++k;
			l = -1;
		}

		for(j = firstCol, putDelimiter = 0; j < lastCol; j++)
		{
			if(j >= worksheet->dimension.firstColIndex)
				++l;

			if(k > -1 && l > -1 && (worksheet->cell[k][l].type != BIFF8_CELL_TYPE_BLANK || saveMode != BIFF8_WORKSHEET_SAVE_COMPACT_2))
			{
				if(insertChar(file, charDelimiter, &putDelimiter))
					break;

				if(biff8ConvertCellToString(compDocInfo, globals, &worksheet->cell[k][l], &string))
					return -1;

				if((putTextDelimiter = biff8StringCharMatch(string, textDelimiter, 1, &charIndex, &charSize)) == -1)
					break;

				if(putTextDelimiter)
					textDelimiterFound = 1;
				else
				{
					if((putTextDelimiter = biff8StringCharMatch(string, delimiter, 1, &charIndex, &charSize)) == -1)
						break;

					putTextDelimiter |= enclose;
				}

				if(!putTextDelimiter)
				{
					if((putTextDelimiter = biff8StringCharMatch(string, CHAR_LF, 1, &charIndex, &charSize)) == -1)
						break;
				}

				putTextDelimiter += (putTextDelimiter == 1);

				if(biff8StringConvert(encoding, string, replacement))
					break;

				if(insertChar(file, charTextDelimiter, &putTextDelimiter))
					break;

				if(textDelimiterFound)
				{
					int prevCharIndex;
					int writeSize;

					charIndex = 0;

					while((prevCharIndex = charIndex) < string->numBytes)
					{
						if((textDelimiterFound = biff8StringCharMatch(string, textDelimiter, 0, &charIndex, &charSize)) == -1)
							break;

						if(!textDelimiterFound)
						{
							if(prevCharIndex < string->numBytes)
							{
								writeSize = string->numBytes - prevCharIndex;
								textDelimiterFound = (fwrite(&string->string[prevCharIndex], sizeof(unsigned char), writeSize, file) != writeSize);
							}

							break;
						}

						writeSize = charIndex - prevCharIndex + charSize;

						if(fwrite(&string->string[prevCharIndex], sizeof(unsigned char), writeSize, file) != writeSize)
							break;

						if(insertChar(file, charTextDelimiter, &textDelimiterFound))
							break;

						charIndex += charSize;
					}

					if(textDelimiterFound)
						break;
				}
				else
				{
					if(fwrite(string->string, sizeof(unsigned char), string->numBytes, file) != string->numBytes)
						break;
				}

				if(insertChar(file, charTextDelimiter, &putTextDelimiter))
					break;

				biff8ConvertStringFree(&string);

				putDelimiter = 1;
				newRow = 1;
			}
			else if(saveMode != BIFF8_WORKSHEET_SAVE_COMPACT_2)
			{
				newRow = 1;
				putDelimiter = 1;

				if(insertChar(file, charDelimiter, &putDelimiter))
					break;
			}
		}

		if(j < lastCol)
			break;

		if(crlf && newRow)
		{
			newRow = 2;

			if(insertChar(file, charCr, &newRow))
				break;
		}

		if(insertChar(file, charLf, &newRow))
			break;
	}

	biff8ConvertStringFree(&string);
	specialCharsFree(&charDelimiter, &charTextDelimiter, &charCr, &charLf);

	return ((i < lastRow) ? -1 : 0);
}
