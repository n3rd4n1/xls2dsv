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
 * biff8_string.c
 *
 *  Created on: Jun 20, 2011
 *      Author: Billy
 */

#include "biff8_string.h"
#include "char_to_uint.h"
#include "biff8_records.h"
#include <stdlib.h>

void biff8StringFree(BIFF8_STRING **string)
{
	if(!string)
		return;

	if(*string)
	{
		if((*string)->string)
			free((*string)->string);

		free(*string);
		*string = 0;
	}
}

int biff8StringAllocate(BIFF8_STRING **string, unsigned int size)
{
	if(!string || !size)
		return -1;

	if(!*string)
	{
		if(!(*string = (BIFF8_STRING *)malloc(sizeof(BIFF8_STRING))))
			return -1;
	}

	if(!((*string)->string = (unsigned char *)malloc(size)))
	{
		biff8StringFree(string);
		return -1;
	}

	return 0;
}

static int utf16IsSurrogatePair(unsigned short *utf16)
{
	return (utf16[1] >= 0xdc00 && utf16[1] <= 0xdfff && *utf16 >= 0xd800 && *utf16 <= 0xdbff);
}

static int utf16ToUnicode(unsigned short *utf16, unsigned int *size)
{
	if(!utf16 || !size)
		return -1;

	int _utf16 = charToUint(CHAR_TO_UINT_SWAP_BYTES, (unsigned char *)utf16, sizeof(unsigned short));

	if(utf16IsSurrogatePair(utf16))
	{
		*size = 4;
		return (((_utf16 - 0xdc00) | ((int)(charToUint(CHAR_TO_UINT_SWAP_BYTES, (unsigned char *)&utf16[1], sizeof(unsigned short)) - 0xd800) << 10)) + 0x10000);
	}

	*size = 2;
	return _utf16;
}

static long long int unicodeToUtf16(unsigned int unicode, unsigned int *size)
{
	if(!size)
		return -1;

	*size = 0;

	if((unicode > 0xd7ff && unicode < 0xe000) || unicode > 0x10ffff)
		return -1LL;

	if(unicode < 0x10000)
	{
		*size = 2;
		return (long long int)charToUint(CHAR_TO_UINT_SWAP_BYTES, (unsigned char *)&unicode, sizeof(unsigned short));;
	}

	*size = 4;
	unicode -= 0x10000;

	unsigned short _unicode[2] = { 0xdc00 + (unicode & 0x3ff), 0xd800 + ((unicode >> 10) & 0x3ff) };
	_unicode[0] = charToUint(CHAR_TO_UINT_SWAP_BYTES, (unsigned char *)_unicode, sizeof(unsigned short));
	_unicode[1] = charToUint(CHAR_TO_UINT_SWAP_BYTES, (unsigned char *)&_unicode[1], sizeof(unsigned short));
	return (long long int)*(long long int *)_unicode;
}

static int utf8ToUnicode(unsigned char *utf8, unsigned int *size)
{
	if(!utf8 || !size)
		return -1;

	*size = 0;

	switch((*utf8 & 0xc0))
	{
	case 0xc0:
		break;
	case 0x80:
		return -1;
	default:
		*size = 1;
		return (int)*utf8;
	}

	int bytes = 1;
	char mask = 0xc0;
	unsigned char m = 0x20;

	while((*utf8 & m))
	{
		m >>= 1;
		mask >>= 1;

		if(++bytes > 5)
			return -1;
	}

	int unicode = (*utf8 & ~*(unsigned char *)&mask) << (bytes * 6);

	for(*size = 1; bytes; ++*size)
	{
		if((utf8[*size] & 0xc0) != 0x80)
		{
			*size = 0;
			return -1;
		}

		--bytes;
		unicode |= ((int)utf8[*size] << (bytes * 6));
	}

	return unicode;
}

static long long int unicodeToUtf8(unsigned int unicode, unsigned int *size)
{
	if(!size)
		return -1LL;

	*size = 0;

	if(unicode > 0x7fffffff)
		return -1LL;

	if(unicode < 0x80)
	{
		*size = 1;
		return unicode;
	}

	unsigned long long utf8 = 0;
	int shift = 0;

	while(1)
	{
		utf8 |= ((unsigned long long)((unicode & 0x3f) | 0x80) << (shift * 8));

		if(!(unicode >>= 6))
		{
			unsigned long long mask = 0x80ULL << (shift * 8);
			unsigned int s = shift + 1;
			unsigned long long m = mask;

			while(s)
			{
				m >>= 1;

				if((utf8 & m))
					break;

				--s;
			}

			if(s)
				mask = 0x80ULL << (++shift * 8);

			++shift;

			while(shift)
			{
				utf8 |= mask;
				mask >>= 1;
				--shift;
				++*size;
			}

			break;
		}

		++shift;
	}

	utf8 = charToUint(CHAR_TO_UINT_SWAP_BYTES, (unsigned char *)&utf8, *size);
	return (utf8 ? utf8 : -1LL);
}

static int utf16ToUtf8(unsigned char *utf16, unsigned char *utf8, unsigned int *sizeUtf16, unsigned int *sizeUtf8)
{
	if(!utf16 || !utf8 || !sizeUtf16 || !sizeUtf8)
		return -1;

	int unicode = utf16ToUnicode((unsigned short *)utf16, sizeUtf16);

	if(unicode == -1)
		return -1;

	unsigned char _utf8[sizeof(long long int)];

	if((*(long long int *)_utf8 = unicodeToUtf8(unicode, sizeUtf8)) == -1LL)
		return -1;

	int i = 0;

	while(i < *sizeUtf8)
	{
		utf8[i] = _utf8[i];
		++i;
	}

	return 0;
}

static int utf8ToUtf16(unsigned char *utf8, unsigned char *utf16, unsigned int *sizeUtf8, unsigned int *sizeUtf16)
{
	if(!utf8 || !utf16 || !sizeUtf8 || !sizeUtf16)
		return -1;

	int unicode = utf8ToUnicode(utf8, sizeUtf8);

	if(unicode == -1)
		return -1;

	unsigned char _utf16[sizeof(long long int)];

	if((*(long long int *)_utf16 = unicodeToUtf16(unicode, sizeUtf16)) == -1LL)
		return -1;

	int i = 0;

	while(i < *sizeUtf16)
	{
		utf16[i] = _utf16[i];
		++i;
	}

	return 0;
}

static int utfToAscii(BIFF8_STRING_ENCODING srcEncoding, unsigned char *utf, unsigned char *ascii, char sub, unsigned int *sizeUtf)
{
	if(!utf || !ascii || sub < 0 || !sizeUtf)
		return -1;

	int unicode;

	switch(srcEncoding)
	{
	case BIFF8_STRING_ENCODING_UTF16:
		unicode = utf16ToUnicode((unsigned short *)utf, sizeUtf);
		break;
	case BIFF8_STRING_ENCODING_UTF8:
		unicode = utf8ToUnicode(utf, sizeUtf);
		break;
	default:
		return -1;
	}

	if(unicode == -1)
		return -1;

	*ascii = (unsigned char)((unicode < 0x80) ? unicode : sub);
	return 0;
}

int biff8StringCharMatch(BIFF8_STRING *string, char c, int resetIndex, unsigned int *index, unsigned int *size)
{
	if(!string || c < 0 || !index || !size)
		return -1;

	int unicode;

	if(resetIndex)
		*index = 0;

	for( ; *index < string->numBytes; *index += *size)
	{
		if((unicode = (string->encoding == BIFF8_STRING_ENCODING_UTF16) ? utf16ToUnicode((unsigned short *)&string->string[*index], size) : utf8ToUnicode((unsigned char *)&string->string[*index], size)) == -1)
			return -1;

		if(unicode == (int)c)
			return 1;
	}

	return 0;
}

int biff8StringConvert(BIFF8_STRING_ENCODING encoding, BIFF8_STRING *string, char sub)
{
	if(!string || sub < 0)
		return -1;

	if(!string->string)
		return -1;

	switch(encoding)
	{
	case BIFF8_STRING_ENCODING_ASCII:
	case BIFF8_STRING_ENCODING_UTF16:
	case BIFF8_STRING_ENCODING_UTF8:
		break;
	default:
		return -1;
	}

	if(!string->numBytes)
	{
		string->encoding = encoding;
		return 0;
	}

	if(string->encoding == encoding)
		return 0;

	if(string->encoding == BIFF8_STRING_ENCODING_UTF16 && (string->numBytes % 2))
		return -1;

	unsigned char *buffer = 0;
	int i, numBytes;

	if(string->encoding == BIFF8_STRING_ENCODING_ASCII)
	{
		if(encoding == BIFF8_STRING_ENCODING_UTF16)
		{
			numBytes = string->numBytes * 2;

			if(!(buffer = (unsigned char *)calloc(numBytes, sizeof(unsigned char))))
				return -1;

			for(i = 0; i < string->numBytes; i++)
				buffer[i * 2 + 1] = string->string[i];
		}
	}
	else
	{
		unsigned int bufferSize;
		unsigned int sizeSrc, sizeDest;
		unsigned char *temp;

		switch(encoding)
		{
		case BIFF8_STRING_ENCODING_ASCII:
			bufferSize = (string->encoding == BIFF8_STRING_ENCODING_UTF16) ? string->numBytes / 2 : string->numBytes;
			break;

		case BIFF8_STRING_ENCODING_UTF16:
			bufferSize = string->numBytes * 4;
			break;

		default: // BIFF8_STRING_ENCODING_UTF8
			bufferSize = string->numBytes / 4 * 6 + 6;
			break;
		}

		if(!(buffer = (unsigned char *)malloc(bufferSize)))
			return -1;

		if(encoding == BIFF8_STRING_ENCODING_ASCII)
		{
			for(i = 0, numBytes = 0; i < string->numBytes; i += sizeSrc, numBytes++)
			{
				if(utfToAscii(string->encoding, &string->string[i], &buffer[numBytes], sub, &sizeSrc))
					break;
			}
		}
		else
		{
			int (*fn)(unsigned char *, unsigned char *, unsigned int *, unsigned int *) = (encoding == BIFF8_STRING_ENCODING_UTF16) ? utf8ToUtf16 : utf16ToUtf8;

			for(i = 0, numBytes = 0; i < string->numBytes; i += sizeSrc, numBytes += sizeDest)
			{
				if(fn(&string->string[i], &buffer[numBytes], &sizeSrc, &sizeDest))
					break;
			}
		}

		if(i != string->numBytes)
		{
			free(buffer);
			return -1;
		}

		if(!(temp = (unsigned char *)realloc(buffer, numBytes + 1)))
		{
			free(buffer);
			return -1;
		}

		buffer = temp;
		buffer[numBytes] = 0;
	}

	if(buffer)
	{
		free(string->string);
		string->string = buffer;
		string->numBytes = numBytes;
	}

	string->encoding = encoding;
	return 0;
}

int biff8StringGet(COMPOUND_DOCUMENT_INFO *compDocInfo, BIFF8_STRING_INDEX *index, unsigned int stringLengthSize, BIFF8_STRING **string)
{
	if(!compDocInfo || !index || stringLengthSize < 1 || stringLengthSize > 2 || !string)
		return -1;

	if(*string)
		return -1;

	if(!(*string = (BIFF8_STRING *)malloc(sizeof(BIFF8_STRING))))
		return -1;

	unsigned char *unicodeString[2] = { &((unsigned char *)index->owner)[index->index], (index->split.valid && index->split.hasOpt) ? &((unsigned char *)index->owner)[index->index + index->split.size0 + BIFF8_RECORD_HEADER_SIZE] : 0 };

	int opts = unicodeString[0][stringLengthSize];
	int bytesPerCharSrc = (opts & (0x1 << 0)) ? 2 : 1;
	int bytesPerCharDest = (bytesPerCharSrc == 2 || (unicodeString[1] && *unicodeString[1])) ? 2 : 1;

	(*string)->numBytes = charToUint(compDocInfo->byteOrder, unicodeString[0], stringLengthSize) * bytesPerCharDest;

	if(biff8StringAllocate(string, (*string)->numBytes + 1))
		return -1;

	int c;
	int i = stringLengthSize + 1 + (2 * !!(opts & (0x1 << 3))) + (4 * !!(opts & (0x1 << 2)));
	int j = 0;
	int k = 0;

	for( ; j < (*string)->numBytes; j += bytesPerCharDest, k += bytesPerCharSrc)
	{
		if(unicodeString[1] && (i + k) == index->split.size0)
		{
			bytesPerCharSrc = *unicodeString[1] ? 2 : 1;
			k += (BIFF8_RECORD_HEADER_SIZE + 1);
		}

		c = charToUint(compDocInfo->byteOrder, &unicodeString[0][i + k], bytesPerCharSrc);

		if(bytesPerCharDest == 1)
			(*string)->string[j] = c;
		else
			*((unsigned short *)&((*string)->string[j])) = charToUint(CHAR_TO_UINT_BYTE_ORDER_BIG_ENDIAN, (unsigned char *)&c, sizeof(unsigned short));
	}

	(*string)->string[(*string)->numBytes] = 0;

	if(bytesPerCharDest == 1)
	{
		for(i = 0; i < (*string)->numBytes && (*string)->string[i] < 0x80; i++);

		if(i == (*string)->numBytes)
			(*string)->encoding = BIFF8_STRING_ENCODING_UTF8;
		else
		{
			(*string)->encoding = BIFF8_STRING_ENCODING_ASCII;

			if(biff8StringConvert(BIFF8_STRING_ENCODING_UTF16, *string, 0))
			{
				biff8StringFree(string);
				return -1;
			}
		}
	}
	else
		(*string)->encoding = BIFF8_STRING_ENCODING_UTF16;

	return 0;
}

static int unicodeStringSplit(COMPOUND_DOCUMENT_INFO *compDocInfo, unsigned char *unicodeString, unsigned int stringLengthSize, unsigned int size, BIFF8_STRING_SPLIT *split)
{
	if(!compDocInfo || !unicodeString || stringLengthSize < 1 || stringLengthSize > 2 || !split)
		return -1;

	int opts = unicodeString[stringLengthSize];

	int bytesPerChar = (opts & (0x1 << 0)) ? 2 : 1;
	int stringLength = charToUint(compDocInfo->byteOrder, unicodeString, stringLengthSize) - 1;
	int rt = 0, sz = 0;

	split->size0 = stringLengthSize + 1;

	if(opts & (0x1 << 3))
	{
		rt = charToUint(compDocInfo->byteOrder, &unicodeString[split->size0], 2);
		split->size0 += 2;
	}

	if(opts & (0x1 << 2))
	{
		sz = charToUint(compDocInfo->byteOrder, &unicodeString[split->size0], 4);
		split->size0 += 4;
	}

	split->size0 += bytesPerChar;

	if(size < split->size0)
		return -1;

	split->hasOpt = 0;

	while(size > split->size0)
	{
		if(stringLength)
		{
			if(size < (split->size0 + bytesPerChar))
				break;

			split->size0 += bytesPerChar;
			--stringLength;
		}
		else if(rt)
		{
			if(size < (split->size0 + 4))
				break;

			split->size0 += 4;
			--rt;
		}
		else
		{
			if(size > (split->size0 + sz))
				break;

			sz -= (size - split->size0);
			split->size0 = size;
		}
	}

	if(size != split->size0)
		return -1;

	split->size1 = (4 * rt) + sz;

	if(stringLength)
	{
		split->hasOpt = 1;

		struct split {
			BIFF8_RECORD_HEADER_RAW header;
			unsigned char opt;
		} *rec = (struct split *)&unicodeString[size];

		if(charToUint(compDocInfo->byteOrder, rec->header.id, sizeof(rec->header.id)) != BIFF8_RECORD_CONTINUE)
			return -1;

		split->size1 += (1 + stringLength * (rec->opt ? 2 : 1));
	}

	return 0;
}

static int unicodeStringSizeGet(COMPOUND_DOCUMENT_INFO *compDocInfo, unsigned char *unicodeString, unsigned int stringLengthSize)
{
	if(!compDocInfo || !unicodeString || stringLengthSize < 1 || stringLengthSize > 2)
		return -1;

	int opts = unicodeString[stringLengthSize];
	int stringLength = charToUint(compDocInfo->byteOrder, unicodeString, stringLengthSize) * ((opts & (0x1 << 0)) ? 2 : 1);

	int unicodeStringSize = stringLengthSize + 1;
	int rt = 0, sz = 0;

	if(opts & (0x1 << 3))
	{
		rt = charToUint(compDocInfo->byteOrder, &unicodeString[unicodeStringSize], 2);
		unicodeStringSize += 2;
	}

	if(opts & (0x1 << 2))
	{
		sz = charToUint(compDocInfo->byteOrder, &unicodeString[unicodeStringSize], 4);
		unicodeStringSize += 4;
	}

	return (unicodeStringSize + stringLength + (4 * rt) + sz);
}

int biff8StringIndexArrayNextGet(COMPOUND_DOCUMENT_INFO *compDocInfo, unsigned int stringLengthSize, BIFF8_STRING_INDEX *currentIndex, BIFF8_STRING_INDEX *nextIndex)
{
	if(!compDocInfo || stringLengthSize < 1 || stringLengthSize > 2 || !currentIndex || !nextIndex)
		return -1;

	if(!currentIndex->owner)
		return -1;

	unsigned short recordSize = BIFF8_RECORD_HEADER_SIZE + charToUint(compDocInfo->byteOrder, currentIndex->owner->dataSize, sizeof(currentIndex->owner->dataSize));
	int unicodeStringSize = unicodeStringSizeGet(compDocInfo, &((unsigned char *)currentIndex->owner)[currentIndex->index], stringLengthSize);

	nextIndex->index = currentIndex->index + unicodeStringSize;
	currentIndex->split.valid = 0;

	if(recordSize > nextIndex->index)
		nextIndex->owner = currentIndex->owner;
	else
	{
		nextIndex->owner = (BIFF8_RECORD_HEADER_RAW *)(&((unsigned char *)currentIndex->owner)[recordSize]);

		int diff = nextIndex->index - recordSize;
		nextIndex->index = BIFF8_RECORD_HEADER_SIZE;

		if(diff)
		{
			if(unicodeStringSplit(compDocInfo, &((unsigned char *)currentIndex->owner)[currentIndex->index], stringLengthSize, unicodeStringSize - diff,  &currentIndex->split))
				return -1;

			currentIndex->split.valid = 1;
			nextIndex->index += currentIndex->split.size1;
		}
	}

	return 0;
}


