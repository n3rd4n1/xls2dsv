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
 * biff_stream.c
 *
 *  Created on: Jun 17, 2011
 *      Author: Billy
 */

#include "biff8_stream.h"
#include <string.h>

BIFF8_STREAM_TYPE biff8StreamTypeGet(COMPOUND_DOCUMENT_INFO *compDocInfo, int dirID)
{
	if(!compDocInfo)
		return BIFF8_STREAM_TYPE_UNKNOWN;

	if(!compDocInfo->dirEntryTbl)
		return BIFF8_STREAM_TYPE_UNKNOWN;

	if(dirID < 0 || dirID >= compDocInfo->dirEntryTblSize)
		return BIFF8_STREAM_TYPE_UNKNOWN;
	
	if(!memcmp(BIFF8_STREAM_NAME_ROOT_ENTRY, compDocInfo->dirEntryTbl[dirID].raw.name, 22) && compDocInfo->dirEntryTbl[dirID].info.nameSize == BIFF8_STREAM_NAME_SIZE_ROOT_ENTRY)
		return (dirID ? BIFF8_STREAM_TYPE_UNKNOWN : BIFF8_STREAM_TYPE_ROOT_ENTRY);

	if(!memcmp(BIFF8_STREAM_NAME_WORKBOOK, compDocInfo->dirEntryTbl[dirID].raw.name, BIFF8_STREAM_NAME_SIZE_WORKBOOK) && compDocInfo->dirEntryTbl[dirID].info.nameSize == BIFF8_STREAM_NAME_SIZE_WORKBOOK)
		return BIFF8_STREAM_TYPE_WORKBOOK;

	if(!memcmp(BIFF8_STREAM_NAME_SUMMARY_INFORMATION, compDocInfo->dirEntryTbl[dirID].raw.name, BIFF8_STREAM_NAME_SIZE_SUMMARY_INFORMATION) && compDocInfo->dirEntryTbl[dirID].info.nameSize == BIFF8_STREAM_NAME_SIZE_SUMMARY_INFORMATION)
		return BIFF8_STREAM_TYPE_SUMMARY_INFORMATION;

	if(!memcmp(BIFF8_STREAM_NAME_DOCUMENT_SUMMARY_INFORMATION, compDocInfo->dirEntryTbl[dirID].raw.name, BIFF8_STREAM_NAME_SIZE_DOCUMENT_SUMMARY_INFORMATION) && compDocInfo->dirEntryTbl[dirID].info.nameSize == BIFF8_STREAM_NAME_SIZE_DOCUMENT_SUMMARY_INFORMATION)
		return BIFF8_STREAM_TYPE_DOCUMENT_SUMMARY_INFORMATION;

	if(!memcmp(BIFF8_STREAM_NAME_CTLS, compDocInfo->dirEntryTbl[dirID].raw.name, BIFF8_STREAM_NAME_SIZE_CTLS) && compDocInfo->dirEntryTbl[dirID].info.nameSize == BIFF8_STREAM_NAME_SIZE_CTLS)
		return BIFF8_STREAM_TYPE_CTLS;

	if(!memcmp(BIFF8_STREAM_NAME_USER_NAMES, compDocInfo->dirEntryTbl[dirID].raw.name, BIFF8_STREAM_NAME_SIZE_USER_NAMES) && compDocInfo->dirEntryTbl[dirID].info.nameSize == BIFF8_STREAM_NAME_SIZE_USER_NAMES)
		return BIFF8_STREAM_TYPE_USER_NAMES;

	if(!memcmp(BIFF8_STREAM_NAME_REVISION_LOG, compDocInfo->dirEntryTbl[dirID].raw.name, BIFF8_STREAM_NAME_SIZE_REVISION_LOG) && compDocInfo->dirEntryTbl[dirID].info.nameSize == BIFF8_STREAM_NAME_SIZE_REVISION_LOG)
		return BIFF8_STREAM_TYPE_REVISION_LOG;

	return BIFF8_STREAM_TYPE_UNKNOWN;
}

int biff8StreamIsWorkbook(COMPOUND_DOCUMENT_INFO *compDocInfo, int dirID)
{
	return (biff8StreamTypeGet(compDocInfo, dirID) == BIFF8_STREAM_TYPE_WORKBOOK);
}