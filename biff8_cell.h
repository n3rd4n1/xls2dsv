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
 * biff8_cell.h
 *
 *  Created on: Jun 21, 2011
 *      Author: Billy
 */

#ifndef BIFF8_CELL_H_
#define BIFF8_CELL_H_

#include "biff8_record_details.h"
#include "biff8_string.h"

typedef enum {
	BIFF8_CELL_TYPE_BLANK = 0,
	BIFF8_CELL_TYPE_SST_STRING,
	BIFF8_CELL_TYPE_RK,
	BIFF8_CELL_TYPE_NUMBER,
	BIFF8_CELL_TYPE_STRING,
	BIFF8_CELL_TYPE_BOOLEAN,
	BIFF8_CELL_TYPE_ERROR
} BIFF8_CELL_TYPE;

typedef struct {
	BIFF8_CELL_TYPE type;
	int xfIndex;
	unsigned long long arg;
} BIFF8_CELL;

int biff8CellAllocate(BIFF8_RECORD_DIMENSION_STRUCT *dimension, BIFF8_CELL ***cell);
void biff8CellFree(BIFF8_RECORD_DIMENSION_STRUCT *dimension, BIFF8_CELL ***cell);

#endif /* BIFF8_CELL_H_ */
