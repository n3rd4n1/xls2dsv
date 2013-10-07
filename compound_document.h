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
 * compound_document.h
 *
 *  Created on: Jun 15, 2011
 *      Author: Billy
 */

#ifndef COMPOUND_DOCUMENT_H_
#define COMPOUND_DOCUMENT_H_

#include "char_to_uint.h"
#include <stdio.h>

typedef enum {
	COMPOUND_DOCUMENT_SecID_Free = -1,
	COMPOUND_DOCUMENT_SecID_EndOfChain = -2,
	COMPOUND_DOCUMENT_SecID_SAT = -3,
	COMPOUND_DOCUMENT_SecID_MSAT = -4
} COMPOUND_DOCUMENT_SecID;

#define SEC_ID_SIZE sizeof(int)

#define MAGIC		"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1"
#define MAGIC_SIZE	8

#define BYTE_ORDER_ID_LITTLE_ENDIAN		"\xfe\xff"
#define BYTE_ORDER_ID_BIG_ENDIAN		"\xff\xfe"
#define BYTE_ORDER_ID_SIZE				sizeof(short)

#define MASTER_SEC_ALLOC_TBL_PART1_ELEMENTS		109

typedef enum {
	COMPOUND_DOCUMENT_BYTE_ORDER_LITTLE_ENDIAN = CHAR_TO_UINT_BYTE_ORDER_LITTLE_ENDIAN,
	COMPOUND_DOCUMENT_BYTE_ORDER_BIG_ENDIAN = CHAR_TO_UINT_BYTE_ORDER_BIG_ENDIAN
} COMPOUND_DOCUMENT_BYTE_ORDER;

typedef struct {
	unsigned char magic[MAGIC_SIZE]; // compound document file identifier
	unsigned char uid[16]; // unique identifier
	unsigned char rev[2]; // revision number of the file format
	unsigned char ver[2]; // version number of the file format
	unsigned char byteOrderID[BYTE_ORDER_ID_SIZE]; // byte order identifier
	unsigned char ssz[2]; // size of a sector in a compound document file in power-of-two
	unsigned char sssz[2]; // size of a short sector in the short-stream container stream in power-of-two
	unsigned char notUsed0[10];
	unsigned char numSec[4]; // total number of sectors used for the sector allocation table
	unsigned char dirStreamSec1ID[4]; // SecID of first sector of the directory stream
	unsigned char notUsed1[4];
	unsigned char stdStreamMinSize[4]; // minimum size of a standard stream
	unsigned char shortSecAllocTblSec1ID[4]; // SecID of first sector of the short-sector allocation table; or -2 (End of Chain SecID) if not extant
	unsigned char shortSecAllocTblNumSec[4]; // total number of sectors used for the short-sector allocation table
	unsigned char masterSecAllocTblSec1ID[4]; // SecID of first sector of the master sector allocation table; or -2 (End of Chain SecID) if no additional sectors used
	unsigned char masterSecAllocTblNumSec[4]; // total number of sectors used for the master sector allocation table
	unsigned char masterSecAllocTblPart1[MASTER_SEC_ALLOC_TBL_PART1_ELEMENTS * SEC_ID_SIZE]; // first part of the master sector allocation table containing 109 SecIDs
} COMPOUND_DOCUMENT_HEADER;

typedef enum {
	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_TYPE_EMPTY			= 0,
	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_TYPE_USER_STORAGE		= 1,
	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_TYPE_USER_STREAM		= 2,
	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_TYPE_LOCKBYTES		= 3,
	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_TYPE_PROPERTY			= 4,
	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_TYPE_ROOT_STORAGE		= 5
} COMPOUND_DOCUMENT_DIRECTORY_ENTRY_TYPE;

typedef enum {
	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_NODE_COLOR_RED	= 0,
	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_NODE_COLOR_BLACK	= 1
} COMPOUND_DOCUMENT_DIRECTORY_ENTRY_NODE_COLOR;

typedef struct {
	unsigned char name[64]; // null-terminated 16-bit unicode character array of the name of the entry
	unsigned char nameSize[2]; // size of the used area of the character buffer of the name, including the null character
	unsigned char type[1]; // entry type
	unsigned char nodeColor[1]; // node color of the entry
	unsigned char leftChildNodeDirID[4]; // DirID of the left child node inside the red-black tree of all direct members of the parent storage, if a user storage or stream; -1 otherwise
	unsigned char rightChildNodeDirID[4]; // DirID of the right child node inside the red-black tree of all direct members of the parent storage, if a user storage or stream; -1 otherwise
	unsigned char rootNodeDirID[4]; // DirID of the root node entry of the red-black tree of all storage members, if a storage; -1 otherwise
	unsigned char storageUID[16]; // unique identifier, if a storage
	unsigned char userFlags[4]; // user flags
	unsigned char creationTimeStamp[8]; // time stamp of creation of this entry
	unsigned char lastModificationTimeStamp[8]; // time stamp of last modification of this entry
	unsigned char sec1ID[4]; // SecID of first sector or short-sector, if this entry refers to a stream; SecID of first sector of the short-stream container stream, if this is the root storage entry; 0 otherwise
	unsigned char totalSize[4]; // total stream size in bytes, if this entry refers to a stream; total size of the short-stream container stream, if this is the root storage entry; 0 otherwise
	unsigned char notUsed[4];
} COMPOUND_DOCUMENT_DIRECTORY_ENTRY_RAW;

#define DIRECTORY_ENTRY_SIZE sizeof(COMPOUND_DOCUMENT_DIRECTORY_ENTRY_RAW)

typedef enum {
	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_ALLOCATION_TABLE_STANDARD_SECTOR,
	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_ALLOCATION_TABLE_SHORT_SECTOR
} COMPOUND_DOCUMENT_DIRECTORY_ENTRY_ALLOCATION_TABLE;

typedef struct {
	unsigned short nameSize;
	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_TYPE type;
	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_NODE_COLOR nodeColor;
	int leftChildNodeDirID;
	int rightChildNodeDirID;
	int rootNodeDirID;
	int userFlags;
	int sec1ID;
	unsigned int totalSize;
	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_ALLOCATION_TABLE allocTbl;
} COMPOUND_DOCUMENT_DIRECTORY_ENTRY_INFO;

typedef struct {
	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_RAW raw;
	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_INFO info;
} COMPOUND_DOCUMENT_DIRECTORY_ENTRY;

typedef struct {
	COMPOUND_DOCUMENT_BYTE_ORDER byteOrder;
	unsigned short rev;
	unsigned short ver;
	unsigned short secSize;
	unsigned short shortSecSize;
	unsigned int numSec;
	int dirStreamSec1ID;
	unsigned int stdStreamMinSize;
	int shortSecAllocTblSec1ID;
	unsigned int shortSecAllocTblNumSec;
	int masterSecAllocTblSec1ID;
	unsigned int masterSecAllocTblNumSec;

	unsigned short secIDItems;

	int *masterSecAllocTbl;
	unsigned int masterSecAllocTblSize;

	int *secAllocTbl;
	unsigned int secAllocTblSize;

	int *shortSecAllocTbl;
	unsigned int shortSecAllocTblSize;

	COMPOUND_DOCUMENT_DIRECTORY_ENTRY *dirEntryTbl;
	unsigned int dirEntryTblSize;

	char *shortStream;
	unsigned int shortStreamSize;
} COMPOUND_DOCUMENT_INFO;

#define COMPOUND_DOCUMENT_INFO_INITIALIZER { .masterSecAllocTbl = 0, .masterSecAllocTblSize = 0, .secAllocTbl = 0, .secAllocTblSize = 0, .shortSecAllocTbl = 0, .shortSecAllocTblSize = 0, .dirEntryTbl = 0, .dirEntryTblSize = 0, .shortStream = 0, .shortStreamSize = 0 }

int compoundDocumentFileCheck(const char *filename);
int compoundDocumentFileHeaderGet(FILE *file, COMPOUND_DOCUMENT_HEADER *header);
int compoundDocumentFileInfoGet(FILE *file, COMPOUND_DOCUMENT_INFO *info);
void compoundDocumentFileTableDestroy(COMPOUND_DOCUMENT_INFO *info);
int compoundDocumentFileStreamRead(FILE *file, COMPOUND_DOCUMENT_INFO *info, int dirID, unsigned char *buffer, unsigned int offset, unsigned int size);
int compoundDocumentFileInfoPrint(const char *filename);

#endif /* COMPOUND_DOCUMENT_H_ */
