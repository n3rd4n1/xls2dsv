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
 * biff8_error_codes.h
 *
 *  Created on: Jun 28, 2011
 *      Author: Billy
 */

#ifndef BIFF8_ERROR_CODES_H_
#define BIFF8_ERROR_CODES_H_

typedef enum {
	BIFF8_ERROR_CODE_NULL = 0x00,
	BIFF8_ERROR_CODE_DIV0 = 0x07,
	BIFF8_ERROR_CODE_VALUE = 0x0f,
	BIFF8_ERROR_CODE_REF = 0x17,
	BIFF8_ERROR_CODE_NAME = 0x1d,
	BIFF8_ERROR_CODE_NUM = 0x24,
	BIFF8_ERROR_CODE_NA = 0x2a
} BIFF8_ERROR_CODE;

#define BIFF8_ERROR_CODE_STRING_NULL	"#NULL!"
#define BIFF8_ERROR_CODE_STRING_DIV0	"#DIV/0!"
#define BIFF8_ERROR_CODE_STRING_VALUE	"#VALUE!"
#define BIFF8_ERROR_CODE_STRING_REF		"#REF!"
#define BIFF8_ERROR_CODE_STRING_NAME	"#NAME?"
#define BIFF8_ERROR_CODE_STRING_NUM		"#NUM!"
#define BIFF8_ERROR_CODE_STRING_NA		"#N/A"

#endif /* BIFF8_ERROR_CODES_H_ */
