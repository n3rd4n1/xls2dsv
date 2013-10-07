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
 * biff8_cell.c
 *
 *  Created on: Jun 21, 2011
 *      Author: Billy
 */

#include "biff8_cell.h"
#include <stdlib.h>

void biff8CellFree(BIFF8_RECORD_DIMENSION_STRUCT *dimension, BIFF8_CELL ***cell)
{
	if(!dimension || !cell)
		return;

	if(!dimension->numRows)
		return;

	if(!*cell)
		return;

	int i = 0;

	while(i < dimension->numRows)
	{
		if(!(*cell)[i])
			break;

		free((*cell)[i]);

		++i;
	}

	free(*cell);
	*cell = 0;
}

int biff8CellAllocate(BIFF8_RECORD_DIMENSION_STRUCT *dimension, BIFF8_CELL ***cell)
{
	if(!dimension || !cell)
		return -1;

	if(!dimension->numRows || !dimension->numColumns)
		return -1;

	if(*cell)
		return -1;

	if(!(*cell = (BIFF8_CELL **)calloc(dimension->numRows, sizeof(BIFF8_CELL *))))
		return -1;

	int i = 0;

	while(i < dimension->numRows)
	{
		if(!((*cell)[i] = (BIFF8_CELL *)calloc(dimension->numColumns, sizeof(BIFF8_CELL))))
			break;

		++i;
	}

	if(i < dimension->numRows)
	{
		biff8CellFree(dimension, cell);
		return -1;
	}

	return 0;
}
