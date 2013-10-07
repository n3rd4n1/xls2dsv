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
 * compound_document.c
 *
 *  Created on: Jun 15, 2011
 *      Author: Billy
 */

#include "compound_document.h"
#include <string.h>
#include <stdlib.h>

static inline int headerCheck(const COMPOUND_DOCUMENT_HEADER *header)
{
	return (memcmp(header->magic, MAGIC, MAGIC_SIZE) ? -1 : 0);
}

int compoundDocumentFileCheck(const char *filename)
{
	if(!filename)
		return -1;

	int ret = -1;
	FILE *file = 0;

	do
	{
		if(!(file = fopen(filename, "r")))
			break;

		char magic[MAGIC_SIZE];

		if(fread(magic, sizeof(char), MAGIC_SIZE, file) != MAGIC_SIZE)
			break;

		if(!headerCheck((COMPOUND_DOCUMENT_HEADER *)magic))
			ret = 0;
	}
	while(0);

	if(file)
		fclose(file);

	return ret;
}

int compoundDocumentFileHeaderGet(FILE *file, COMPOUND_DOCUMENT_HEADER *header)
{
	if(!file || !header)
		return -1;

	if(fseek(file, 0, SEEK_SET))
		return -1;

	if(fread(header, sizeof(char), sizeof(COMPOUND_DOCUMENT_HEADER), file) != sizeof(COMPOUND_DOCUMENT_HEADER))
		return -1;

	if(headerCheck(header))
		return -1;

	return 0;
}

static COMPOUND_DOCUMENT_BYTE_ORDER byteOrderCheck(const COMPOUND_DOCUMENT_HEADER *header)
{
	if(!header)
		return -1;

	if(!memcmp(header->byteOrderID, BYTE_ORDER_ID_LITTLE_ENDIAN, BYTE_ORDER_ID_SIZE))
		return COMPOUND_DOCUMENT_BYTE_ORDER_LITTLE_ENDIAN;

	if(!memcmp(header->byteOrderID, BYTE_ORDER_ID_BIG_ENDIAN, BYTE_ORDER_ID_SIZE))
		return COMPOUND_DOCUMENT_BYTE_ORDER_BIG_ENDIAN;

	return -1;
}

static unsigned int powerOfTwo(int exp)
{
	if(exp)
		return (2 * powerOfTwo(exp - 1));

	return 1;
}

static int sectorFileOffsetGet(COMPOUND_DOCUMENT_INFO *info, unsigned int sec)
{
	if(!info)
		return -1;

	return (sizeof(COMPOUND_DOCUMENT_HEADER) + sec * info->secSize);
}

static void tableDestroy(void **table, unsigned int *size)
{
	if(!table || !size)
		return;

	if(*table)
	{
		free(*table);
		*table = 0;
	}

	*size = 0;
}

static void masterSectorAllocationTableDestroy(COMPOUND_DOCUMENT_INFO *info)
{
	if(!info)
		return;

	tableDestroy((void **)&info->masterSecAllocTbl, &info->masterSecAllocTblSize);
}

static int masterSectorAllocationTableBuild(FILE *file, COMPOUND_DOCUMENT_INFO *info, COMPOUND_DOCUMENT_HEADER *header)
{
	if(!file || !info || !header)
		return -1;

	if(info->masterSecAllocTbl || info->masterSecAllocTblSize)
		return -1;

	if(!info->numSec)
		return -1;

	if(!(info->masterSecAllocTbl = (int *)malloc((info->numSec + 2) * sizeof(int))))
		return -1;

	unsigned int *masterSecAllocTbl = (unsigned int *)header->masterSecAllocTblPart1;

	int i = 0;

	while(i < MASTER_SEC_ALLOC_TBL_PART1_ELEMENTS)
	{
		if((info->masterSecAllocTbl[info->masterSecAllocTblSize] = charToUint(info->byteOrder, (unsigned char *)&masterSecAllocTbl[i], SEC_ID_SIZE)) == COMPOUND_DOCUMENT_SecID_EndOfChain)
			break;

		++i;

		if(info->masterSecAllocTbl[info->masterSecAllocTblSize] < 0)
			continue;

		if(++info->masterSecAllocTblSize > info->numSec)
			break;
	}

	masterSecAllocTbl = 0;

	while(info->masterSecAllocTblSize <= info->numSec && info->masterSecAllocTbl[info->masterSecAllocTblSize] != COMPOUND_DOCUMENT_SecID_EndOfChain && (info->masterSecAllocTbl[info->masterSecAllocTblSize] = info->masterSecAllocTblSec1ID) != COMPOUND_DOCUMENT_SecID_EndOfChain && info->masterSecAllocTblNumSec)
	{
		if(!(masterSecAllocTbl = (unsigned int *)malloc(info->secSize)))
			break;

		int sectorFileOffset = sectorFileOffsetGet(info, info->masterSecAllocTblSec1ID);
		int secCnt = 0;

		do
		{
			if(fseek(file, sectorFileOffset, SEEK_SET))
				break;

			if(fread(masterSecAllocTbl, sizeof(char), info->secSize, file) != info->secSize)
				break;

			while(1)
			{
				if((info->masterSecAllocTbl[info->masterSecAllocTblSize] = charToUint(info->byteOrder, (unsigned char *)&masterSecAllocTbl[i], SEC_ID_SIZE)) == COMPOUND_DOCUMENT_SecID_EndOfChain)
					break;

				if(++i == info->secIDItems)
				{
					if(info->masterSecAllocTbl[info->masterSecAllocTblSize] >= 0)
						sectorFileOffset = sectorFileOffsetGet(info, info->masterSecAllocTbl[info->masterSecAllocTblSize]);

					break;
				}

				if(info->masterSecAllocTbl[info->masterSecAllocTblSize] < 0)
					continue;

				if(++info->masterSecAllocTblSize > info->numSec)
					break;
			}
		}
		while(info->masterSecAllocTblSize <= info->numSec && info->masterSecAllocTbl[info->masterSecAllocTblSize] >= 0 && secCnt < info->masterSecAllocTblNumSec);

		free(masterSecAllocTbl);
		break;
	}

	if(info->masterSecAllocTblSize == info->numSec && info->masterSecAllocTbl[info->masterSecAllocTblSize] == COMPOUND_DOCUMENT_SecID_EndOfChain)
		return 0;

	masterSectorAllocationTableDestroy(info);
	return -1;
}

static void sectorAllocationTableDestroy(COMPOUND_DOCUMENT_INFO *info)
{
	if(!info)
		return;

	tableDestroy((void **)&info->secAllocTbl, &info->secAllocTblSize);
}

static int sectorAllocationTableBuild(FILE *file, COMPOUND_DOCUMENT_INFO *info)
{
	if(!file || !info)
		return -1;

	if(!info->masterSecAllocTblSize || !info->masterSecAllocTbl)
		return -1;

	if(info->secAllocTbl || info->secAllocTblSize)
		return -1;

	if(!(info->secAllocTbl = (int *)malloc(info->masterSecAllocTblSize * info->secSize)))
		return -1;

	unsigned int *secAllocTbl = (unsigned int *)malloc(info->secSize);

	if(!secAllocTbl)
	{
		sectorAllocationTableDestroy(info);
		return -1;
	}

	int i = 0, j;

	for( ; i < info->masterSecAllocTblSize; i++)
	{
		if(fseek(file, sectorFileOffsetGet(info, info->masterSecAllocTbl[i]), SEEK_SET))
			break;

		if(fread(secAllocTbl, sizeof(char), info->secSize, file) != info->secSize)
			break;

		for(j = 0; j < info->secIDItems; j++, info->secAllocTblSize++)
			info->secAllocTbl[info->secAllocTblSize] = charToUint(info->byteOrder, (unsigned char *)&secAllocTbl[j], SEC_ID_SIZE);
	}

	free(secAllocTbl);

	if(info->secAllocTblSize == (info->masterSecAllocTblSize * info->secIDItems))
		return 0;

	sectorAllocationTableDestroy(info);
	return -1;
}

static void shortSectorAllocationTableDestroy(COMPOUND_DOCUMENT_INFO *info)
{
	if(!info)
		return;

	tableDestroy((void **)&info->shortSecAllocTbl, &info->shortSecAllocTblSize);
}


static int shortSectorAllocationTableBuild(FILE *file, COMPOUND_DOCUMENT_INFO *info)
{
	if(!file || !info)
		return -1;

	if(!info->secAllocTbl || !info->secAllocTblSize)
		return -1;

	if(info->shortSecAllocTbl || info->shortSecAllocTblSize)
		return -1;

	if(info->shortSecAllocTblSec1ID < 0 && !info->shortSecAllocTblNumSec)
		return 0;

	if(!(info->shortSecAllocTbl = (int *)malloc(info->shortSecAllocTblNumSec * info->secSize)))
		return -1;

	unsigned int *shortSecAllocTbl = (unsigned int *)malloc(info->secSize);

	if(!shortSecAllocTbl)
	{
		shortSectorAllocationTableDestroy(info);
		return -1;
	}

	int sector = info->shortSecAllocTblSec1ID;
	int sectorCnt = 0;
	int i;

	while(sector != COMPOUND_DOCUMENT_SecID_EndOfChain && sectorCnt < info->shortSecAllocTblNumSec)
	{
		if(sector >= info->secAllocTblSize)
			break;

		if(fseek(file, sectorFileOffsetGet(info, sector), SEEK_SET))
			break;

		if(fread(shortSecAllocTbl, sizeof(char), info->secSize, file) != info->secSize)
			break;

		for(i = 0; i < info->secIDItems; i++, info->shortSecAllocTblSize++)
			info->shortSecAllocTbl[info->shortSecAllocTblSize] = charToUint(info->byteOrder, (unsigned char *)&shortSecAllocTbl[i], SEC_ID_SIZE);

		sector = info->secAllocTbl[sector];
		++sectorCnt;
	}

	free(shortSecAllocTbl);

	if(sector == COMPOUND_DOCUMENT_SecID_EndOfChain)
		return 0;

	shortSectorAllocationTableDestroy(info);
	return -1;
}

static void dirEntryTableDestroy(COMPOUND_DOCUMENT_INFO *info)
{
	if(!info)
		return;

	tableDestroy((void **)&info->dirEntryTbl, &info->dirEntryTblSize);
}

static int dirEntryTableBuild(FILE *file, COMPOUND_DOCUMENT_INFO *info)
{
	if(!file || !info)
		return -1;

	if(!info->secAllocTbl || !info->secAllocTblSize)
		return -1;

	if(info->dirEntryTbl || info->dirEntryTblSize)
		return -1;

	int sector = info->dirStreamSec1ID;
	int sectorNum = 0;

	while(sector != COMPOUND_DOCUMENT_SecID_EndOfChain)
	{
		if(sector >= info->secAllocTblSize)
			return -1;

		sector = info->secAllocTbl[sector];
		++sectorNum;
	}

	if(!sectorNum)
		return 0;

	int entriesPerSector = info->secSize / DIRECTORY_ENTRY_SIZE;

	if(!entriesPerSector)
		return -1;

	int dirEntries = sectorNum * entriesPerSector;

	if(!(info->dirEntryTbl = (COMPOUND_DOCUMENT_DIRECTORY_ENTRY *)malloc(dirEntries * sizeof(COMPOUND_DOCUMENT_DIRECTORY_ENTRY))))
		return -1;

	COMPOUND_DOCUMENT_DIRECTORY_ENTRY_RAW *dirEntryTbl = (COMPOUND_DOCUMENT_DIRECTORY_ENTRY_RAW *)malloc(info->secSize);

	if(!dirEntryTbl)
	{
		dirEntryTableDestroy(info);
		return -1;
	}

	sector = info->dirStreamSec1ID;

	int sectorCnt = 0;
	int i;

	while(sector != COMPOUND_DOCUMENT_SecID_EndOfChain && sectorCnt < sectorNum)
	{
		if(sector >= info->secAllocTblSize)
			break;

		if(fseek(file, sectorFileOffsetGet(info, sector), SEEK_SET))
			break;

		if(fread(dirEntryTbl, sizeof(char), info->secSize, file) != info->secSize)
			break;

		for(i = 0; i < entriesPerSector; i++, info->dirEntryTblSize++)
		{
			memcpy(&info->dirEntryTbl[info->dirEntryTblSize].raw, &dirEntryTbl[i], DIRECTORY_ENTRY_SIZE);

			info->dirEntryTbl[info->dirEntryTblSize].info.nameSize = charToUint(info->byteOrder, info->dirEntryTbl[info->dirEntryTblSize].raw.nameSize , sizeof(info->dirEntryTbl[info->dirEntryTblSize].raw.nameSize));
			info->dirEntryTbl[info->dirEntryTblSize].info.type = *info->dirEntryTbl[info->dirEntryTblSize].raw.type;
			info->dirEntryTbl[info->dirEntryTblSize].info.nodeColor = *info->dirEntryTbl[info->dirEntryTblSize].raw.nodeColor;
			info->dirEntryTbl[info->dirEntryTblSize].info.leftChildNodeDirID = charToUint(info->byteOrder, info->dirEntryTbl[info->dirEntryTblSize].raw.leftChildNodeDirID , sizeof(info->dirEntryTbl[info->dirEntryTblSize].raw.leftChildNodeDirID ));
			info->dirEntryTbl[info->dirEntryTblSize].info.rightChildNodeDirID = charToUint(info->byteOrder, info->dirEntryTbl[info->dirEntryTblSize].raw.rightChildNodeDirID , sizeof(info->dirEntryTbl[info->dirEntryTblSize].raw.rightChildNodeDirID));
			info->dirEntryTbl[info->dirEntryTblSize].info.rootNodeDirID = charToUint(info->byteOrder, info->dirEntryTbl[info->dirEntryTblSize].raw.rootNodeDirID , sizeof(info->dirEntryTbl[info->dirEntryTblSize].raw.rootNodeDirID));
			info->dirEntryTbl[info->dirEntryTblSize].info.userFlags = charToUint(info->byteOrder, info->dirEntryTbl[info->dirEntryTblSize].raw.userFlags , sizeof(info->dirEntryTbl[info->dirEntryTblSize].raw.userFlags));
			info->dirEntryTbl[info->dirEntryTblSize].info.sec1ID = charToUint(info->byteOrder, info->dirEntryTbl[info->dirEntryTblSize].raw.sec1ID , sizeof(info->dirEntryTbl[info->dirEntryTblSize].raw.sec1ID));
			info->dirEntryTbl[info->dirEntryTblSize].info.totalSize = charToUint(info->byteOrder, info->dirEntryTbl[info->dirEntryTblSize].raw.totalSize , sizeof(info->dirEntryTbl[info->dirEntryTblSize].raw.totalSize));
			info->dirEntryTbl[info->dirEntryTblSize].info.allocTbl = (info->dirEntryTbl[info->dirEntryTblSize].info.totalSize < info->stdStreamMinSize && info->dirEntryTblSize) ? COMPOUND_DOCUMENT_DIRECTORY_ENTRY_ALLOCATION_TABLE_SHORT_SECTOR : COMPOUND_DOCUMENT_DIRECTORY_ENTRY_ALLOCATION_TABLE_STANDARD_SECTOR;
		}

		sector = info->secAllocTbl[sector];
		++sectorCnt;
	}

	free(dirEntryTbl);

	if(sector == COMPOUND_DOCUMENT_SecID_EndOfChain)
		return 0;

	dirEntryTableDestroy(info);
	return -1;
}

static void shortStreamDestroy(COMPOUND_DOCUMENT_INFO *info)
{
	if(!info)
		return;

	tableDestroy((void **)&info->shortStream, &info->shortStreamSize);
}

static int shortStreamBuild(FILE *file, COMPOUND_DOCUMENT_INFO *info)
{
	if(!file || !info)
		return -1;

	if(!info->secAllocTbl || !info->secAllocTblSize)
		return -1;

	if(!info->dirEntryTbl || !info->dirEntryTblSize)
		return -1;

	if(info->shortStream || info->shortStreamSize)
		return -1;

	if(info->dirEntryTbl[0].info.type != COMPOUND_DOCUMENT_DIRECTORY_ENTRY_TYPE_ROOT_STORAGE)
		return -1;

	if(info->dirEntryTbl[0].info.allocTbl != COMPOUND_DOCUMENT_DIRECTORY_ENTRY_ALLOCATION_TABLE_STANDARD_SECTOR)
		return -1;

	if(info->dirEntryTbl[0].info.sec1ID < 0 && !info->dirEntryTbl[0].info.totalSize && !info->shortSecAllocTbl && !info->shortSecAllocTblSize && !info->shortSecAllocTblNumSec && info->shortSecAllocTblSec1ID < 0)
		return 0;

	if(info->dirEntryTbl[0].info.sec1ID < 0 || !info->dirEntryTbl[0].info.totalSize)
		return -1;

	if(!(info->shortStream = (char *)malloc(info->dirEntryTbl[0].info.totalSize)))
		return -1;

	unsigned int *shortStream = (unsigned int *)malloc(info->secSize);

	if(!shortStream)
	{
		shortStreamDestroy(info);
		return -1;
	}

	int sector = info->dirEntryTbl[0].info.sec1ID;
	int sectorCnt = 0;
	unsigned int bytesToWrite;

	while(sector != COMPOUND_DOCUMENT_SecID_EndOfChain && sectorCnt < info->secAllocTblSize && info->shortStreamSize < info->dirEntryTbl[0].info.totalSize)
	{
		if(sector >= info->secAllocTblSize)
			break;

		if(fseek(file, sectorFileOffsetGet(info, sector), SEEK_SET))
			break;

		if(fread(shortStream, sizeof(char), info->secSize, file) != info->secSize)
			break;

		memcpy(&info->shortStream[info->shortStreamSize], shortStream, bytesToWrite = ((bytesToWrite = info->dirEntryTbl[0].info.totalSize - info->shortStreamSize) > info->secSize) ? info->secSize : bytesToWrite);
		info->shortStreamSize += bytesToWrite;

		sector = info->secAllocTbl[sector];
		++sectorCnt;
	}

	free(shortStream);

	if(sector == COMPOUND_DOCUMENT_SecID_EndOfChain && info->shortStreamSize == info->dirEntryTbl[0].info.totalSize)
		return 0;

	shortStreamDestroy(info);
	return -1;
}

void compoundDocumentFileTableDestroy(COMPOUND_DOCUMENT_INFO *info)
{
	shortStreamDestroy(info);
	dirEntryTableDestroy(info);
	shortSectorAllocationTableDestroy(info);
	sectorAllocationTableDestroy(info);
	masterSectorAllocationTableDestroy(info);
}

int compoundDocumentFileInfoGet(FILE *file, COMPOUND_DOCUMENT_INFO *info)
{
	if(!file || !info)
		return -1;

	COMPOUND_DOCUMENT_HEADER header;

	if(compoundDocumentFileHeaderGet(file, &header))
		return -1;

	if((info->byteOrder = byteOrderCheck(&header)) != COMPOUND_DOCUMENT_BYTE_ORDER_LITTLE_ENDIAN && info->byteOrder != COMPOUND_DOCUMENT_BYTE_ORDER_BIG_ENDIAN)
		return -1;

	info->rev = charToUint(info->byteOrder, header.rev, sizeof(header.rev));
	info->ver = charToUint(info->byteOrder, header.ver, sizeof(header.ver));
	info->secSize = powerOfTwo(charToUint(info->byteOrder, header.ssz, sizeof(header.ssz)));
	info->shortSecSize = powerOfTwo(charToUint(info->byteOrder, header.sssz, sizeof(header.sssz)));
	info->numSec = charToUint(info->byteOrder, header.numSec, sizeof(header.numSec));
	info->dirStreamSec1ID = charToUint(info->byteOrder, header.dirStreamSec1ID, sizeof(header.dirStreamSec1ID));
	info->stdStreamMinSize = charToUint(info->byteOrder, header.stdStreamMinSize, sizeof(header.stdStreamMinSize));
	info->shortSecAllocTblSec1ID = charToUint(info->byteOrder, header.shortSecAllocTblSec1ID, sizeof(header.shortSecAllocTblSec1ID));
	info->shortSecAllocTblNumSec = charToUint(info->byteOrder, header.shortSecAllocTblNumSec, sizeof(header.shortSecAllocTblNumSec));
	info->masterSecAllocTblSec1ID = charToUint(info->byteOrder, header.masterSecAllocTblSec1ID, sizeof(header.masterSecAllocTblSec1ID));
	info->masterSecAllocTblNumSec = charToUint(info->byteOrder, header.masterSecAllocTblNumSec, sizeof(header.masterSecAllocTblNumSec));

	info->secIDItems = info->secSize / SEC_ID_SIZE;

	do
	{
		if(masterSectorAllocationTableBuild(file, info, &header))
			break;

		if(sectorAllocationTableBuild(file, info))
			break;

		if(shortSectorAllocationTableBuild(file, info))
			break;

		if(dirEntryTableBuild(file, info))
			break;

		if(shortStreamBuild(file, info))
			break;

		return 0;
	}
	while(0);

	compoundDocumentFileTableDestroy(info);
	return -1;
}

static int shortSectorOffsetGet(COMPOUND_DOCUMENT_INFO *info, unsigned int sec)
{
	if(!info)
		return -1;

	return (sec * info->shortSecSize);
}

int compoundDocumentFileStreamRead(FILE *file, COMPOUND_DOCUMENT_INFO *info, int dirID, unsigned char *buffer, unsigned int offset, unsigned int size)
{
	if(!info || !buffer || !size)
		return -1;

	if(dirID < 1 || dirID >= info->dirEntryTblSize)
		return -1;

	if(offset > info->dirEntryTbl[dirID].info.totalSize)
		return -1;

	if((offset + size) > info->dirEntryTbl[dirID].info.totalSize)
		size = info->dirEntryTbl[dirID].info.totalSize - offset;

	if(!size)
		return -1;

	int secID = info->dirEntryTbl[dirID].info.sec1ID;

	if(secID < 0)
		return -1;

	int secChainPos;
	int secPos;

	unsigned int bytesRead = 0;
	unsigned int bytesToWrite;
	int i;

	if(info->dirEntryTbl[dirID].info.allocTbl == COMPOUND_DOCUMENT_DIRECTORY_ENTRY_ALLOCATION_TABLE_SHORT_SECTOR)
	{
		if(!info->shortSecSize)
			return -1;

		if(!info->shortStream || !info->shortStreamSize)
			return -1;

		secChainPos = offset / info->shortSecSize;
		secPos = offset % info->shortSecSize;

		for(i = 0; i < info->shortSecAllocTblSize && i < secChainPos && secID >= 0; i++)
			secID = info->shortSecAllocTbl[secID];

		if(i < secChainPos || secID < 0)
			return -1;

		for(i = 0; i < info->shortSecAllocTblSize && secID >= 0; bytesRead += bytesToWrite, secPos = 0, secID = info->shortSecAllocTbl[secID], i++)
			memcpy(&buffer[bytesRead], &info->shortStream[shortSectorOffsetGet(info, secID) + secPos], bytesToWrite = ((bytesToWrite = size - bytesRead) > (info->shortSecSize - secPos)) ? (info->shortSecSize - secPos) : bytesToWrite);
	}
	else if(info->dirEntryTbl[dirID].info.allocTbl == COMPOUND_DOCUMENT_DIRECTORY_ENTRY_ALLOCATION_TABLE_STANDARD_SECTOR)
	{
		if(!file)
			return -1;

		if(!info->secAllocTbl || !info->secAllocTblSize)
			return -1;

		secChainPos = offset / info->secSize;
		secPos = offset % info->secSize;

		for(i = 0; i < info->secAllocTblSize && i < secChainPos && secID >= 0; i++)
			secID = info->secAllocTbl[secID];

		if(i < secChainPos || secID < 0)
			return -1;

		for(i = 0; i < info->secAllocTblSize && secID >= 0; bytesRead += bytesToWrite, secPos = 0, secID = info->secAllocTbl[secID], i++)
		{
			bytesToWrite = ((bytesToWrite = size - bytesRead) > (info->secSize - secPos)) ? (info->secSize - secPos) : bytesToWrite;

			if(fseek(file, sectorFileOffsetGet(info, secID) + secPos, SEEK_SET))
				break;

			if(fread(&buffer[bytesRead], sizeof(char), bytesToWrite, file) != bytesToWrite)
				break;
		}
	}

	if(bytesRead == size)
		return bytesRead;

	return -1;
}

int compoundDocumentFileInfoPrint(const char *filename)
{
	int ret = -1;
	FILE *file = 0;

	do
	{
		if(!(file = fopen(filename, "r")))
			break;

		COMPOUND_DOCUMENT_HEADER header;
		COMPOUND_DOCUMENT_INFO info = COMPOUND_DOCUMENT_INFO_INITIALIZER;

		int i;
		char c = 'a' - 1;

		if(compoundDocumentFileHeaderGet(file, &header))
			break;

		if(compoundDocumentFileInfoGet(file, &info))
			break;

		printf("\n[compound document file information]");
		printf("\n%c) file name: %s", ++c, filename);

		for(printf("\n%c) file identifier:", ++c), i = 0; i < MAGIC_SIZE; i++)
			printf(" %.2x", header.magic[i]);

		for(printf("\n%c) unique identifier:", ++c), i = 0; i < sizeof(header.uid); i++)
			printf(" %.2x", header.uid[i]);

		printf("\n%c) byte order: %s-endian", ++c, (info.byteOrder == COMPOUND_DOCUMENT_BYTE_ORDER_LITTLE_ENDIAN) ? "little" : "big");

		printf("\n%c) revision number: %.4x", ++c, info.rev);

		printf("\n%c) version number: %.4x", ++c, info.ver);

		printf("\n%c) sector size: %u", ++c, info.secSize);

		printf("\n%c) short-sector size: %u", ++c, info.shortSecSize);

		printf("\n%c) total number of sectors: %u", ++c, info.numSec);

		printf("\n%c) directory stream first sector SecID: %d", ++c, info.dirStreamSec1ID);

		printf("\n%c) minimum size of a standard stream: %u", ++c, info.stdStreamMinSize);

		printf("\n%c) short-sector allocation table first sector SecID: %d", ++c, info.shortSecAllocTblSec1ID);

		printf("\n%c) short-sector allocation table total number of sectors used: %d", ++c, info.shortSecAllocTblNumSec);

		printf("\n%c) master sector allocation table first sector SecID: %d", ++c, info.masterSecAllocTblSec1ID);

		printf("\n%c) master sector allocation table total number of sectors used: %d", ++c, info.masterSecAllocTblNumSec);

		for(printf("\n\n%c) MSAT (%d):\n", ++c, info.masterSecAllocTblSize), i = 0; i <= info.masterSecAllocTblSize; i++)
			printf(" %d", info.masterSecAllocTbl[i]);

		for(printf("\n\n%c) SAT (%d):\n", ++c, info.secAllocTblSize), i = 0; i < info.secAllocTblSize; i++)
			printf(" [%d]%d", i, info.secAllocTbl[i]);

		for(printf("\n\n%c) SSAT (%d):\n", ++c, info.shortSecAllocTblSize), i = 0; i < info.shortSecAllocTblSize; i++)
			printf(" [%d]%d", i, info.shortSecAllocTbl[i]);

		printf("\n\n%c) directory entries:\n", ++c);

		int entryCnt = 0;
		int x;

		while(entryCnt < info.dirEntryTblSize)
		{
			printf("[%d]\n", entryCnt);

			printf("name: ");

			for(i = 0; i < info.dirEntryTbl[entryCnt].info.nameSize; i += 2)
			{
				x = charToUint(info.byteOrder, &info.dirEntryTbl[entryCnt].raw.name[i], sizeof(short));

				if(x < 256)
				{
					switch(x)
					{
					case 1:
						printf("<01h>");
						break;
					case 5:
						printf("<05h>");
						break;
					default:
						printf("%c", x);
						break;
					}
				}
				else
					printf("?");
			}

			printf("\n");

			printf("type: ");

			switch(info.dirEntryTbl[entryCnt].info.type)
			{
			case COMPOUND_DOCUMENT_DIRECTORY_ENTRY_TYPE_EMPTY:
				printf("empty");
				break;
			case COMPOUND_DOCUMENT_DIRECTORY_ENTRY_TYPE_USER_STORAGE:
				printf("user storage");
				break;
			case COMPOUND_DOCUMENT_DIRECTORY_ENTRY_TYPE_USER_STREAM:
				printf("user stream");
				break;
			case COMPOUND_DOCUMENT_DIRECTORY_ENTRY_TYPE_LOCKBYTES:
				printf("lockbytes");
				break;
			case COMPOUND_DOCUMENT_DIRECTORY_ENTRY_TYPE_PROPERTY:
				printf("property");
				break;
			case COMPOUND_DOCUMENT_DIRECTORY_ENTRY_TYPE_ROOT_STORAGE:
				printf("root storage");
				break;
			default:
				printf("unknown");
				break;
			}

			printf("\n");

			printf("node color: ");

			switch(info.dirEntryTbl[entryCnt].info.nodeColor)
			{
			case COMPOUND_DOCUMENT_DIRECTORY_ENTRY_NODE_COLOR_RED:
				printf("red");
				break;
			case COMPOUND_DOCUMENT_DIRECTORY_ENTRY_NODE_COLOR_BLACK:
				printf("black");
				break;
			default:
				printf("unknown");
				break;
			}

			printf("\n");

			printf("left child node DirID: %d\n", info.dirEntryTbl[entryCnt].info.leftChildNodeDirID);
			printf("right child node DirID: %d\n", info.dirEntryTbl[entryCnt].info.rightChildNodeDirID);
			printf("root node DirID: %d\n", info.dirEntryTbl[entryCnt].info.rootNodeDirID);

			for(printf("unique identifier:"), i = 0; i < 16; i++)
				printf(" %.2x", info.dirEntryTbl[entryCnt].raw.storageUID[i]);

			printf("\n");

			printf("user flags: %.8x\n", info.dirEntryTbl[entryCnt].info.userFlags);
			printf("creation time stamp: skipped\n");
			printf("last modification time stamp: skipped\n");
			printf("first sector SecID: %d\n", info.dirEntryTbl[entryCnt].info.sec1ID);
			printf("total stream size: %u\n", info.dirEntryTbl[entryCnt].info.totalSize);

			printf("type of allocation table: ");

			switch(info.dirEntryTbl[entryCnt].info.allocTbl)
			{
			case COMPOUND_DOCUMENT_DIRECTORY_ENTRY_ALLOCATION_TABLE_STANDARD_SECTOR:
				printf("sector allocation table (SAT)");
				break;
			case COMPOUND_DOCUMENT_DIRECTORY_ENTRY_ALLOCATION_TABLE_SHORT_SECTOR:
				printf("short-sector allocation table (SSAT)");
				break;
			default:
				printf("unknown");
				break;
			}

			printf("\n\n");

			++entryCnt;
		}

		printf("%c) short stream size: %u\n", ++c, info.shortStreamSize);

		printf("\n\n");

		compoundDocumentFileTableDestroy(&info);
	}
	while(0);

	return ret;
}

