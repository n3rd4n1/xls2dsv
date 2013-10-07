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
 * xls2dsv.c
 *
 *   Created on: Jun 15, 2011
 *       Author: Billy
 *  Description: xls to dsv file converter
 *  Limitations: XL8[97] - XL12[2007], BIFF8 (compound document file)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include "compound_document.h"
#include "biff8_stream.h"
#include "biff8_workbook.h"
#include "biff8_worksheet.h"
#include "biff8_worksheet_save.h"

#define OPTION_INFO_ENCODING \
				"   a) -utf8, for UTF-8\n" \
				"   b) -utf16, for UTF-16BE, no BOM prepended\n" \
				"   c) -ascii='r', to replace characters beyond U+007F\n" \
				"         with 'r' ASCII character (ex. -ascii=?, -ascii=x3f)\n"

#define OPTION_INFO_DELIMITER \
				"   a) -csv, to output comma-delimited files, using quotation\n" \
				"         marks ('\"', 0x22) as text delimiter\n" \
				"   b) -tsv, to output tab-delimited files\n" \
				"   c) -dsv='D','T','E', to explicitly specify the delimiter 'D',\n" \
				"         the text delimiter 'C', and the first letter of file\n" \
				"         extension to use 'E' (ex. -dsv=?,!,q, -dsv=x3f,x22,q);\n" \
				"         note that ',' or x63 and x09 are reserved delimiters,\n" \
				"         and 'c' and 't' are reserved file extensions\n"

#define OPTION_INFO_COMPACT \
				"   a) -compact=0, to preserve the original dimension\n" \
				"   b) -compact=1, to discard empty rows and columns\n" \
				"         after non-empty ones\n" \
				"   c) -compact=2, to discard empty rows and columns\n" \
				"         before and after non-empty ones\n" \
				"   d) -compact=3, to discard all empty cells\n"

static void help(void)
{
	printf("\n::xls2dsv::\n\n");

	printf("[description]\n"
		   "  xls2dsv parses the contents of excel (.xls) files and saves\n"
		   "  them as delimiter-separated values (DSV). It currently\n"
		   "  supports BIFF8 (97 - 2007) excel files as input, and can\n"
		   "  output pure ASCII (no extended ASCII ), UTF-16BE or UTF-8\n"
		   "  encoded files. It also supports user defined parameters for\n"
		   "  the DSV output. It outputs one file for every worksheet on\n"
		   "  the excel file (workbook), naming them as follows,\n"
		   "  \"[input filename - suffix]_[worksheet name].[suffix]\"\n\n");

	printf("[options]\n\n"
		   "1) -help, displays this message\n\n"
		   "2) encoding, choose one of the following\n"OPTION_INFO_ENCODING"\n"
		   "3) delimiter, choose one of the following\n"OPTION_INFO_DELIMITER"\n"
		   "4) compact, choose one of the following\n"OPTION_INFO_COMPACT"\n"
		   "5) -crlf, inserts CR+LF as newline; otherwise LF only\n\n"
		   "6) -enclose, encloses all values with the\n"
		   "      delimiter-collision-avoidance character\n\n"
		   "7) [source file or directory]\n\n"
		   "8) [destination directory]\n\n");

	printf("[default]\n"
		   "encoding:   -utf8\n"
		   "delimiter:  -csv\n"
		   "compact:    -compact=0\n\n");

	printf("[sample usage]\n\n"
		   "a) \"./xls2dsv\"\n"
		   "    will convert all excel files in the working directory into\n"
		   "    comma-delimited, UTF-8-encoded files, preserving the original\n"
		   "    dimension of the excel files, and saving each individual\n"
		   "    worksheets of every file with .csv file extension\n\n"
		   "b) \".xls2dsv -ascii=$ -dsv=?,x3d,q input.xls output -compact=3\"\n"
		   "    will convert the \"input.xls\" file into question-mark-delimited, pure-\n"
		   "    ascii-encoded (replacing non-ASCII characters with '$' sign) files,\n"
		   "    ignoring all empty cells and enclosing all values containing the\n"
		   "    delimiter character '?', the new line character '\\n', and the text\n"
		   "    delimiter character '\"' with the text delimiter character '\"',\n"
		   "    and saving all worksheets of the file into existing \"output\"\n"
		   "    directory with \".qsv\" file extension\n\n"
		   "c) \"./xls2dsv input\"\n"
		   "    will have the same effect as a) except that input files will\n"
		   "    come from the \"input\" directory and all output will be saved\n"
		   "    into the same \"input\" directory\n");

	printf("\n");
	exit(0);
}

typedef enum {
	ERROR_ENCODING,
	ERROR_DELIMITER,
	ERROR_COMPACT,
	ERROR_SOURCE,
	ERROR_DESTINATION,
	ERROR_UNKNOWN = -1
} ERROR;

static void error(ERROR err, void *arg)
{
	printf("\nERROR: ");
	
	switch(err)
	{
	case ERROR_ENCODING:
		printf("invalid/multiple encoding specified\nspecify only one of the following...\n" OPTION_INFO_ENCODING);
		break;

	case ERROR_DELIMITER:
		printf("invalid/multiple delimiter option specified\nspecify only one of the following...\n" OPTION_INFO_DELIMITER);
		break;

	case ERROR_COMPACT:
		printf("invalid/multiple compact option specified\nspecify only one of the following...\n" OPTION_INFO_COMPACT);
		break;

	case ERROR_SOURCE:
	case ERROR_DESTINATION:
		printf("cannot access %s, \"%s\"\n", (err == ERROR_SOURCE) ? "source" : "destination", (char *)arg);
		break;

	default:
		printf("unable to recognize argument, \"%s\"\n", (char *)arg);
		break;
	}
	
	printf("\n>> hint: \"-help\"\n\n");
	exit(1);
}

#define OPTION_HELP "-help"

#define OPTION_ENCODING_UTF8 "-utf8"
#define OPTION_ENCODING_UTF16 "-utf16"
#define OPTION_ENCODING_ASCII "-ascii="
#define OPTION_ENCODING_ASCII_MIN_SIZE strlen(OPTION_ENCODING_ASCII)

#define OPTION_DELIMITER_CSV "-csv"
#define OPTION_DELIMITER_TSV "-tsv"
#define OPTION_DELIMITER_DSV "-dsv="
#define OPTION_DELIMITER_DSV_MIN_SIZE strlen(OPTION_DELIMITER_DSV)

#define OPTION_CRLF "-crlf"

#define OPTION_ENCLOSE "-enclose"

#define FILE_EXTENSION_CSV ".csv"
#define FILE_EXTENSION_TSV ".tsv"

#define OPTION_COMPACT "-compact="
#define OPTION_COMPACT_MIN_SIZE strlen(OPTION_COMPACT)

static char optionValueGet(char *option, int *index, int cOnly)
{
	if(*index >= strlen(option))
		return -1;

	int i = *index;
	int hex = 0;
	unsigned int value = (unsigned int)-1;

	for( ; option[i] && option[i] != ','; i++)
	{
		if(i == *index && tolower(option[i]) == 'x' && isxdigit(option[i + 1]))
		{
			if(cOnly)
				return -1;

			hex = 1;
			continue;
		}

		if(hex && !isxdigit(option[i]))
			return -1;

		if(!hex && i > *index)
			return -1;
	}

	switch((i - *index))
	{
	case 1:
		value = option[*index];
		break;

	case 0:
		return -1;

	default:
		if(sscanf(&option[*index + 1], "%x", &value) != 1)
			return -1;

		if(value > 0x7f)
			return -1;

		break;
	}

	if(option[i])
		*index = i + 1;
	else
		*index = 0;

	return (char)value;
}

static int isXls(char *name)
{
	if(!name)
		return 0;
	
	int len = strlen(name) - 4;
	
	if(len < 1)
		return 0;
	
	char buffer[5];
	
	strcpy(buffer, &name[len]);
	
	int i = 0;
	
	while(++i < 4)
		buffer[i] = tolower(buffer[i]);
	
	return !strcmp(buffer, ".xls");
}

int main(int argc, char *argv[])
{
	int i;
	BIFF8_STRING_ENCODING encoding = BIFF8_STRING_ENCODING_UTF8;
	char delimiter = ',';
	char textDelimiter = '"';
	char replacement = '?';
	BIFF8_WORKSHEET_SAVE_MODE saveMode = BIFF8_WORKSHEET_SAVE_PRESERVE;
	int crlf = 0;
	int enclose = 0;

	char fileExt[] = FILE_EXTENSION_CSV;

	int srcIsDir = 1;
	char src[FILENAME_MAX + 1] = "./";
	char dest[FILENAME_MAX + 1] = "./";
	char fileName[FILENAME_MAX + 1];
	
	char srcFile[FILENAME_MAX * 2];
	char destFile[FILENAME_MAX * 2];
	char destFileTemp[FILENAME_MAX * 3];

	struct stat statStruct;

	if(argc > 1)
	{
		int len, index;
		int encodingSet = 0;
		int delimiterSet = 0;
		int saveModeSet = 0;
		int srcSet = 0;
		int destSet = 0;
		
		for(i = 1; i < argc; i++)
		{
			len = strlen(argv[i]);
			
			if(!strcmp(argv[i], OPTION_HELP))
				help();

			if(!strcmp(argv[i], OPTION_ENCODING_UTF8))
			{
				if(encodingSet)
					error(ERROR_ENCODING, 0);
				
				encodingSet = 1;
				continue;
			}
			
			if(!strcmp(argv[i], OPTION_ENCODING_UTF16))
			{
				if(encodingSet)
					error(ERROR_ENCODING, 0);
				
				encoding = BIFF8_STRING_ENCODING_UTF16;
				encodingSet = 1;
				continue;
			}
			
			if(!memcmp(argv[i], OPTION_ENCODING_ASCII, OPTION_ENCODING_ASCII_MIN_SIZE))
			{
				if(encodingSet)
					error(ERROR_ENCODING, 0);

				index = OPTION_ENCODING_ASCII_MIN_SIZE;
				
				if((replacement = optionValueGet(argv[i], &index, 0)) == -1 || index)
					error (ERROR_ENCODING, 0);
				
				encoding = BIFF8_STRING_ENCODING_ASCII;
				encodingSet = 1;
				continue;
			}
			
			if(!strcmp(argv[i], OPTION_DELIMITER_CSV))
			{
				if(delimiterSet)
					error(ERROR_DELIMITER, 0);

				delimiterSet = 1;
				continue;
			}
			
			if(!strcmp(argv[i], OPTION_DELIMITER_TSV))
			{
				if(delimiterSet)
					error(ERROR_DELIMITER, 0);

				delimiter = '\t';
				fileExt[1] = 't';
				delimiterSet = 1;
				continue;
			}

			if(!memcmp(argv[i], OPTION_DELIMITER_DSV, OPTION_DELIMITER_DSV_MIN_SIZE))
			{
				if(delimiterSet)
					error(ERROR_DELIMITER, 0);

				index = OPTION_DELIMITER_DSV_MIN_SIZE;

				if((delimiter = optionValueGet(argv[i], &index, 0)) == -1 || !index)
					error(ERROR_DELIMITER, 0);

				if(delimiter == ',' || delimiter == '\t')
					error(ERROR_DELIMITER, 0);

				if((textDelimiter = optionValueGet(argv[i], &index, 0)) == -1 || !index)
					error(ERROR_DELIMITER, 0);

				if((fileExt[1] = optionValueGet(argv[i], &index, 1)) == -1 || index)
					error(ERROR_DELIMITER, 0);

				if(fileExt[1] == 'c' || fileExt[1] == 't')
					error(ERROR_DELIMITER, 0);

				delimiterSet = 1;
				continue;
			}

			if(!memcmp(argv[i], OPTION_COMPACT, OPTION_COMPACT_MIN_SIZE))
			{
				if(saveModeSet)
					error(ERROR_COMPACT, 0);

				if(len != (OPTION_COMPACT_MIN_SIZE + 1))
					error(ERROR_COMPACT, 0);

				switch((saveMode = argv[i][OPTION_COMPACT_MIN_SIZE] - '0'))
				{
				case 0:
				case 1:
				case 2:
				case 3:
					break;
				default:
					error(ERROR_COMPACT, 0);
				}

				saveModeSet = 1;
				continue;
			}
			
			if(!strcmp(argv[i], OPTION_CRLF))
			{
				crlf = 1;
				continue;
			}

			if(!strcmp(argv[i], OPTION_ENCLOSE))
			{
				enclose = 1;
				continue;
			}

			if(!srcSet)
			{
				if(stat(argv[i], &statStruct))
					error(ERROR_SOURCE, (void *)argv[i]);

				if(!S_ISDIR(statStruct.st_mode))
				{
					if(!S_ISREG(statStruct.st_mode))
						error(ERROR_SOURCE, (void *)argv[i]);

					if(!isXls(argv[i]))
					   error(ERROR_SOURCE, (void *)argv[i]);
					
					srcIsDir = 0;
				}

				strcpy(src, argv[i]);

				if(src[len - 1] != '/' && srcIsDir)
					strcat(src, "/");

				strcpy(dest, src);
				
				if(!srcIsDir)
				{
					len = strlen(dest);
					
					while(len && dest[--len] != '/');
					
					if(dest[len] == '/')
						dest[len + 1] = 0;
					else
						strcpy(dest, "./");
				}
				
				srcSet = 1;
				continue;
			}

			if(!destSet)
			{
				if(stat(argv[i], &statStruct))
					error(ERROR_DESTINATION, (void *)argv[i]);
				
				if(!S_ISDIR(statStruct.st_mode))
					error(ERROR_DESTINATION, (void *)argv[i]);

				strcpy(dest, argv[i]);

				if(dest[len - 1] != '/')
					strcat(dest, "/");

				destSet = 1;
				continue;
			}

			error(ERROR_UNKNOWN, (void *)argv[i]);
		}
	}
	
	printf("\n[xls2dsv]\n");

	printf(">> encoding: ");

	switch(encoding)
	{
	case BIFF8_STRING_ENCODING_ASCII:
		printf("ASCII; non-ASCII characters will be replaced by ");
		iscntrl(replacement) ? printf("0x%.2x", replacement) : printf("'%c' (0x%.2x)", replacement, replacement);
		printf("\n");
		break;

	case BIFF8_STRING_ENCODING_UTF16:
		printf("UTF-16BE (no BOM)\n");
		break;

	default:
		printf("UTF-8\n");
		break;
	}
	
	printf(">> delimiter: ");
	iscntrl(delimiter) ? printf("0x%.2x", delimiter) : printf("0x%.2x=%c", delimiter, delimiter);
	printf("\n");

	printf(">> text delimiter: ");

	if(delimiter == '\t')
		printf("none");
	else
		iscntrl(textDelimiter) ? printf("0x%.2x", textDelimiter) : printf("0x%.2x=%c", textDelimiter, textDelimiter);

	printf("\n");

	printf(">> file extension: \"%s\"\n", fileExt);
	
	printf(">> save mode: ");
	
	switch(saveMode)
	{
	case BIFF8_WORKSHEET_SAVE_PRESERVE:
		printf("preserve original dimension\n");
		break;

	case BIFF8_WORKSHEET_SAVE_COMPACT_0:
		printf("discard empty rows and columns after non-empty ones\n");
		break;

	case BIFF8_WORKSHEET_SAVE_COMPACT_1:
		printf("discard empty rows and columns before and after non-empty ones\n");
		break;

	default:
		printf("discard all empty cells\n");
		break;
	}

	printf(">> source %s: %s\n", srcIsDir ? "directory" : "file", src);

	printf(">> destination directory: %s\n", dest);

	printf("\n");
	
	DIR *dir = opendir(src);
	struct dirent *dirent = (struct dirent *)-1;
	FILE *ifile;
	FILE *ofile;

	COMPOUND_DOCUMENT_INFO compDocInfo = COMPOUND_DOCUMENT_INFO_INITIALIZER;
	BIFF8_WORKBOOK_GLOBALS globals = BIFF8_WORKBOOK_GLOBALS_INITIALIZER;
	BIFF8_WORKSHEET worksheet = BIFF8_WORKSHEET_INITIALIZER;
	unsigned char *workbook = 0;
	int workbookDirID;

	while(1)
	{
		strcpy(srcFile, src);
		strcpy(destFile, dest);

		if(dir)
		{
			errno = 0;

			if(!(dirent = readdir(dir)))
			{
				if(errno)
				{
					printf("%s", src);
					perror(": readdir: ");
				}

				break;
			}
			
			strcpy(fileName, dirent->d_name);
			
			if(!isXls(fileName))
				continue;
			
			strcat(srcFile, fileName);
		}
		else
		{
			for(i = strlen(srcFile); i && srcFile[--i] != '/'; );
			
			strcpy(fileName, &srcFile[i + (srcFile[i] == '/')]);
			
		
		}
		
		if(stat(srcFile, &statStruct))
		{
			printf("%s", srcFile);
			perror(": stat: ");
			continue;
		}
		
		if(!S_ISREG(statStruct.st_mode))
			continue;
		
		strcat(destFile, fileName);
		
		printf("[%s]\n", srcFile);

		if(!(ifile = fopen(srcFile, "r")))
		{
			printf("ERROR: unable to open input file\n");
			continue;
		}

		do
		{
			if(compoundDocumentFileInfoGet(ifile, &compDocInfo))
				break;

			if((workbookDirID = biff8WorkbookLocate(&compDocInfo)) < 0)
				break;

			if(!(workbook = (unsigned char *)malloc(compDocInfo.dirEntryTbl[workbookDirID].info.totalSize)))
				break;

			if(compoundDocumentFileStreamRead(ifile, &compDocInfo, workbookDirID, workbook, 0, compDocInfo.dirEntryTbl[workbookDirID].info.totalSize) != compDocInfo.dirEntryTbl[workbookDirID].info.totalSize)
				break;

			if(biff8WorkbookGlobalsInfoBuild(&compDocInfo, workbook, compDocInfo.dirEntryTbl[workbookDirID].info.totalSize, &globals))
				break;

			for(i = strlen(destFile) - 1; i && destFile[i] != '.'; i--);

			if(destFile[i] == '.')
				destFile[i] = 0;
			else
				i = strlen(destFile);

			strcat(destFile, "_");

			for(i = 0; i < globals.numSheets; i++, biff8WorksheetInfoDestroy(&worksheet))
			{
				if(biff8WorksheetInfoBuild(&compDocInfo, &globals, workbook, compDocInfo.dirEntryTbl[workbookDirID].info.totalSize, i, &worksheet))
				{
					printf("--> warning: unable to parse worksheet %d\n", i + 1);
					continue;
				}

				strcpy(destFileTemp, destFile);
				strcat(destFileTemp, (char *)globals.sheet[i].name->string);
				strcat(destFileTemp, fileExt);

				printf("--> %s ", destFileTemp);

				if(!(ofile = fopen(destFileTemp, "w")))
				{
					printf("[ERROR: unable to open output file]\n");
					continue;
				}

				if(biff8WorksheetSaveToFile(ofile, &compDocInfo, &globals, &worksheet, delimiter, textDelimiter, encoding, replacement, crlf, enclose, saveMode))
					printf("[ERROR: cannot save worksheet]\n");
				else
					printf("[OK]\n");

				fclose(ofile);
			}
		}
		while(0);

		printf("\n");

		if(workbook)
		{
			free(workbook);
			workbook = 0;
		}

		biff8WorksheetInfoDestroy(&worksheet);
		biff8WorkbookGlobalsInfoDestroy(&globals);
		compoundDocumentFileTableDestroy(&compDocInfo);
		fclose(ifile);

		if(!dir)
			break;
	}

	if(dir)
		closedir(dir);

	return 0;
}
