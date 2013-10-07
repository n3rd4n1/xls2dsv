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
 * biff8_format_string.c
 *
 *  Created on: Jul 1, 2011
 *      Author: Billy
 */

#include "biff8_format_string.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void biff8FormatStringDestroy(BIFF8_FORMAT_STRING *formatString)
{
	if(!formatString)
		return;

	if(formatString->pos)
	{
		free(formatString->pos);
		formatString->pos = 0;

		if(formatString->neg)
		{
			free(formatString->neg);
			formatString->neg = 0;

			if(formatString->zero)
			{
				free(formatString->zero);
				formatString->zero = 0;

				if(formatString->text)
				{
					free(formatString->text);
					formatString->text = 0;
				}
			}
		}
	}
}

int biff8FormatStringBuild(BIFF8_STRING *string, BIFF8_FORMAT_STRING *formatString)
{
	if(!string || !formatString)
		return -1;

	if(!string->string)
		return -1;

	int i = 0, j, len;
	char **format = 0;

	while(i < string->numBytes)
	{
		for(j = i; j < string->numBytes && string->string[j] != ';'; j++);

		if((len = j - i))
		{
			if(format)
			{
				if(format == &formatString->pos)
					format = &formatString->neg;
				else if(format == &formatString->neg)
					format = &formatString->zero;
				else if(format == &formatString->zero)
					format = &formatString->text;
				else
					break;
			}
			else
				format = &formatString->pos;

			if(*format)
				break;

			if(!(*format = (char *)malloc((len + 1) * sizeof(char))))
				break;

			memcpy(*format, &string->string[i], len);
			(*format)[len] = 0;
		}

		i = j + (j < string->numBytes);
	}

	if(i == string->numBytes)
		return 0;

	biff8FormatStringDestroy(formatString);
	return -1;
}

static int isLeapYear(unsigned int year)
{
	if(!(year % 400))
		return 1;

	return ((year % 100) && !(year % 4));
}

typedef struct {
	int valid;

	unsigned long int duration;
	unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char dayOfTheWeek;

	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	int pm;
} DATE_TIME;

static void nextMonth(DATE_TIME *dateTime)
{
	dateTime->year += ((dateTime->month += ((dateTime->month < 12) ? 1 : -11)) == 1);
}

static int isAmPmString(char *format)
{
	int i = 0, len = strlen(format);
	char buffer[6];

	buffer[5] = 0;

	if(len > 4)
	{
		memcpy(buffer, &format[i], 5);

		for(i = 0; i < 5; i++)
			buffer[i] = tolower(buffer[i]);

		return !strcmp(buffer, "am/pm");
	}

	return 0;
}

static int dateTimeConvert(BIFF8_WORKBOOK_GLOBALS *globals, char *format, double number, DATE_TIME *dateTime)
{
	if(!dateTime)
		return -1;

	dateTime->valid = 0;

	if(!globals || !format || number < 0.0)
		return -1;

	if(!globals->dateMode.valid)
		return -1;

	if(number < 0)
		return  -1;

	long int date = (long int)number;
	double time = (number - (double)date) * 24;
	unsigned char rem;

	dateTime->duration = date;
	dateTime->year = globals->dateMode.baseYear;
	dateTime->month = globals->dateMode.baseMonth;
	dateTime->day = globals->dateMode.baseDay;

	while(date)
	{
		while(1)
		{
			switch(dateTime->month)
			{
			case 2:
				rem = isLeapYear(dateTime->year) ? 29 : 28;
				break;
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12:
				rem = 31;
				break;
			case 4:
			case 6:
			case 9:
			case 11:
				rem = 30;
				break;
			default:
				return -1;
			}

			if(dateTime->day > rem)
				return -1;

			if((rem -= dateTime->day))
				break;

			dateTime->day = 0;
			nextMonth(dateTime);
		}

		if(date > rem)
		{
			dateTime->day += rem;
			date -= rem;
		}
		else
		{
			dateTime->day += date;
			date = 0;
		}
	}

	unsigned int year = dateTime->year - (dateTime->month < 3);
	unsigned short y = year % 100;
	unsigned short c = year / 100;

	dateTime->dayOfTheWeek = (unsigned int)(dateTime->day + (2.6 * (dateTime->month + ((dateTime->month < 3) ? 10 : -2)) - 0.2) + y + (y / 4) + (c / 4) - (2 * c)) % 7;

	dateTime->hour = (unsigned char)time;
	dateTime->minute = (unsigned char)(time = (time - (double)(unsigned char)time) * 60);
	dateTime->second = (unsigned char)(time = (time - (double)(unsigned char)time) * 60);

	if(time > 0.5)
	{
		if(++dateTime->second == 60)
		{
			dateTime->second = 0;

			if(++dateTime->minute == 60)
			{
				dateTime->minute = 0;

				if(++dateTime->hour > 24)
					return -1;
			}
		}
	}

	dateTime->pm = -1;

	int i = 0, len = strlen(format) - 4;

	while(i < len)
	{
		if(isAmPmString(&format[i]))
		{
			if((dateTime->pm = (dateTime->hour > 11)) && dateTime->hour > 12)
				dateTime->hour -= 12;

			break;
		}

		++i;
	}

	dateTime->valid = 1;
	return 0;
}

static int monthStringGet(DATE_TIME *dateTime, char *buffer)
{
	if(!dateTime || !buffer)
		return -1;

	if(!dateTime->valid)
		return -1;

	switch(dateTime->month)
	{
	case 1:
		strcpy(buffer, "January");
		break;
	case 2:
		strcpy(buffer, "February");
		break;
	case 3:
		strcpy(buffer, "March");
		break;
	case 4:
		strcpy(buffer, "April");
		break;
	case 5:
		strcpy(buffer, "May");
		break;
	case 6:
		strcpy(buffer, "June");
		break;
	case 7:
		strcpy(buffer, "July");
		break;
	case 8:
		strcpy(buffer, "August");
		break;
	case 9:
		strcpy(buffer, "September");
		break;
	case 10:
		strcpy(buffer, "October");
		break;
	case 11:
		strcpy(buffer, "November");
		break;
	case 12:
		strcpy(buffer, "December");
		break;
	default:
		return -1;
	}

	return 0;
}

static int dayOfTheWeekStringGet(DATE_TIME *dateTime, char *buffer)
{
	if(!dateTime || !buffer)
		return -1;

	if(!dateTime->valid)
		return -1;

	switch(dateTime->dayOfTheWeek)
	{
	case 0:
		strcpy(buffer, "Sunday");
		break;
	case 1:
		strcpy(buffer, "Monday");
		break;
	case 2:
		strcpy(buffer, "Tuesday");
		break;
	case 3:
		strcpy(buffer, "Wednesday");
		break;
	case 4:
		strcpy(buffer, "Thursday");
		break;
	case 5:
		strcpy(buffer, "Friday");
		break;
	case 6:
		strcpy(buffer, "Saturday");
		break;
	default:
		return -1;
	}

	return 0;
}

BIFF8_FORMAT_STRING_DECODE biff8FormatStringDecode(BIFF8_WORKBOOK_GLOBALS *globals, char *format, double number, char *buffer)
{
	if(!format || !buffer)
		return BIFF8_FORMAT_STRING_DECODE_FAILED;

	DATE_TIME dateTime = { .valid = 0 };

	dateTimeConvert(globals, format, number, &dateTime);

	unsigned char *p;
	char buf[100] = { 0 };
	int f = 0, b = 0;
	int i, j, l;
	int X, Y, Z;
	int y, z;
	int c;
	int len = strlen(format);

	while(f < len)
	{
		buffer[b] = 0;

		switch((c = tolower(format[f])))
		{
		case '[':
			for(i = f; i < len && format[i] != ']'; i++);

			if(format[i] != ']')
				return -1;

			f = i + 1;
			continue;

		case '$':
		case '-':
		case '+':
		case '/':
		case '(':
		case ')':
		case ':':
		case ' ':
			buffer[b] = format[f];
			++b;
			break;

		case '"':
			for(i = f + 1; i < len && format[i] != '"'; i++, b++)
				buffer[b] = format[i];

			if(format[i] != '"')
				return -1;

			f = i + 1;
			continue;

		case '\\':
			if(++f < len)
				buffer[b] = format[f];

			++b;
			++f;
			continue;

		case '@':
			return BIFF8_FORMAT_STRING_DECODE_TEXT_PLACEHOLDER;

		case 'm':
		case 'd':
		case 'y':
		case 'h':
		case 's':
			if(dateTime.valid)
			{
				for(i = f; i < len && tolower(format[i]) == c; i++);

				j = i - f;
				X = 0;
				Y = 0;
				Z = 0;

				switch(c)
				{
				case 'd': // day
					X = j / 4;

					if(dayOfTheWeekStringGet(&dateTime, buf))
						return -1;

					y = strlen(buf) + 1;

					switch((Y = j % 4))
					{
					case 1:
						if(sprintf(&buf[y], "%d", dateTime.day) < 1)
							return -1;

						break;

					case 2:
						if(sprintf(&buf[y], "%02d", dateTime.day) != 2)
							return -1;

						break;

					case 3:
						memcpy(&buf[y], buf, 3);
						buf[y + 3] = 0;
						break;

					default:
						break;
					}

					break;

				case 'y': // year
					X = j / 4;

					if(sprintf(buf, "%04d", dateTime.year) != 4)
						return -1;

					if((Z = j % 4))
					{
						if((Y = Z / 2))
						{
							y = 5;
							strcpy(&buf[y], &buf[2]);
						}

						if((Z -= (Y * 2)))
						{
							z = 8;

							if(sprintf(buf, "%c", format[i -1]) != 1)
								return -1;
						}
					}

					break;

				case 'm': // month or minute
					break;

				case 'h':
					if(c == 'h')
						p = &dateTime.hour;

				default: // second or hour
					if(c == 's')
						p = &dateTime.second;

					X = j / 2;

					if(sprintf(buf, "%02d", *p) != 2)
						return -1;

					if((Y = j % 2))
					{
						y = 3;

						if(sprintf(buf, "%d", *p) < 1)
							return -1;
					}

					break;
				}

				l = strlen(buf);

				while(X)
				{
					strcat(buffer, buf);
					b += l;
					--X;
				}

				if(Y)
				{
					strcat(buffer, &buf[y]);
					b += strlen(&buf[y]);
				}

				if(Z)
				{
					strcat(buffer, &buf[z]);
					b += strlen(&buf[z]);
				}

				f = i;
				continue;
			}

			break;

		case 'a':
			if(dateTime.valid && isAmPmString(&format[f]))
			{
				strcat(buffer, dateTime.pm ? "PM" : "AM");
				b += 2;
				f += 5;
				continue;
			}

		default:
			break;
		}

		++f;
	}

	buffer[b] = 0;
	return BIFF8_FORMAT_STRING_DECODE_DONE;
}
