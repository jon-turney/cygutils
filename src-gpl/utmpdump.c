/*
 * utmpdump	Simple program to dump UTMP and WTMP files in
 *		raw format, so they can be examined.
 *
 * Author:	Miquel van Smoorenburg, <miquels@cistron.nl>
 *              Danek Duvall <duvall@alumni.princeton.edu>
 *
 * Version:	@(#)utmpdump  2.79  12-Sep-2000
 *
 *		This file is part of the sysvinit suite,
 *		Copyright 1991-2000 Miquel van Smoorenburg.
 *
 *		Additional Copyright on this file 1998 Danek Duvall.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */

#include "common.h"
#include <utmp.h>
#include <time.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "oldutmp.h"

char *strptime (const char *, const char *, struct tm *);

struct utmp
oldtonew(struct oldutmp src)
{
        struct utmp dest;

	memset(&dest, 0, sizeof dest);
	dest.ut_type = src.ut_type;
	dest.ut_pid  = src.ut_pid;
	dest.ut_time = src.ut_oldtime;
	dest.ut_addr = src.ut_oldaddr;
	strncpy(dest.ut_id,   src.ut_id,   4);
	strncpy(dest.ut_line, src.ut_line, OLD_LINESIZE);
	strncpy(dest.ut_user, src.ut_user, OLD_NAMESIZE);
	strncpy(dest.ut_host, src.ut_host, OLD_HOSTSIZE);

        return dest;
}

struct oldutmp
newtoold(struct utmp src)
{
        struct oldutmp dest;

	memset(&dest, 0, sizeof dest);
	dest.ut_type    = src.ut_type;
	dest.ut_pid     = src.ut_pid;
	dest.ut_oldtime = src.ut_time;
	dest.ut_oldaddr = src.ut_addr;
	strncpy(dest.ut_id,   src.ut_id,   4);
	strncpy(dest.ut_line, src.ut_line, OLD_LINESIZE);
	strncpy(dest.ut_user, src.ut_user, OLD_NAMESIZE);
	strncpy(dest.ut_host, src.ut_host, OLD_HOSTSIZE);

        return dest;
}

char *
timetostr(const time_t time)
{
	static char s[29];    /* [Sun Sep 01 00:00:00 1998 PST] */

	if (time != 0)
		strftime(s, 29, "%a %b %d %T %Y %Z", localtime(&time));
	else
		s[0] = '\0';

	return s;
}

time_t
strtotime(const char *s_time)
{
	struct tm *tm = malloc(sizeof(*tm));

	if (s_time[0] == ' ' || s_time[0] == '\0')
		return (time_t)0;

	strptime(s_time, "%a %b %d %T %Y", tm);

	/* Cheesy way of checking for DST */
	if (s_time[26] == 'D')
		tm->tm_isdst = 1;

	return mktime(tm);
}

#define cleanse(x) xcleanse(x, sizeof(x))
void
xcleanse(char *s, int len)
{
	for ( ; *s && len-- > 0; s++)
		if (!isprint(*s) || *s == '[' || *s == ']')
			*s = '?';
}

void
unspace(char *s, int len)
{
	while (*s && *s != ' ' && len--)
		++s;

	if (len > 0)
		*s = '\0';
}

void
print_utline(struct utmp ut)
{
	char *addr_string, *time_string;
	struct in_addr in;

	in.s_addr = ut.ut_addr;
	addr_string = inet_ntoa(in);
	time_string = timetostr(ut.ut_time);
	cleanse(ut.ut_id);
	cleanse(ut.ut_user);
	cleanse(ut.ut_line);
	cleanse(ut.ut_host);

        /*            pid    id       user     line     host     addr       time */
	printf("[%d] [%05d] [%-4.4s] [%-*.*s] [%-*.*s] [%-*.*s] [%-15.15s] [%-28.28s]\n",
	       ut.ut_type, ut.ut_pid, ut.ut_id, 8, UT_NAMESIZE, ut.ut_user,
	       12, UT_LINESIZE, ut.ut_line, 20, UT_HOSTSIZE, ut.ut_host,
               addr_string, time_string);
}

void
dump(FILE *fp, int forever, int oldfmt)
{
	struct utmp ut;
	struct oldutmp uto;

	if (forever)
		fseek(fp, -10 * (oldfmt ? sizeof uto : sizeof ut), SEEK_END);

	do {
		if (oldfmt)
			while (fread(&uto, sizeof uto, 1, fp) == 1)
				print_utline(oldtonew(uto));
		else
			while (fread(&ut, sizeof ut, 1, fp) == 1)
				print_utline(ut);
		if (forever) sleep(1);
	} while (forever);
}

/* This function won't work properly if there's a ']' or a ' ' in the real
 * token.  Thankfully, this should never happen.  */
int
gettok(char *line, char *dest, int size, int eatspace)
{
	int bpos, epos, eaten;
        char *t;

	bpos = strchr(line, '[') - line;
	if (bpos < 0) {
		fprintf(stderr, "Extraneous newline in file.  Exiting.");
                exit(1);
        }
	line += 1 + bpos;

	epos = strchr(line, ']') - line;
	if (epos < 0) {
		fprintf(stderr, "Extraneous newline in file.  Exiting.");
                exit(1);
        }
	line[epos] = '\0';

	eaten = bpos + epos + 1;

	if (eatspace)
                if ((t = strchr(line, ' ')))
                    *t = 0;

        strncpy(dest, line, size);

	return eaten + 1;
}

void
undump(FILE *fp, int forever, int oldfmt)
{
	struct utmp ut;
	struct oldutmp uto;
	char s_addr[16], s_time[29], *linestart, *line;
	int count = 0;

	line = linestart = malloc(1024 * sizeof *linestart);
	s_addr[15] = 0;
	s_time[28] = 0;

	while(fgets(linestart, 1023, fp))
	{
		line = linestart;
                memset(&ut, '\0', sizeof(ut));
                sscanf(line, "[%hd] [%d] [%4c] ", &ut.ut_type, &ut.ut_pid, ut.ut_id);

		line += 19;
                line += gettok(line, ut.ut_user, sizeof(ut.ut_user), 1);
                line += gettok(line, ut.ut_line, sizeof(ut.ut_line), 1);
                line += gettok(line, ut.ut_host, sizeof(ut.ut_host), 1);
		line += gettok(line, s_addr, sizeof(s_addr)-1, 1);
		line += gettok(line, s_time, sizeof(s_time)-1, 0);

                ut.ut_addr = inet_addr(s_addr);
                ut.ut_time = strtotime(s_time);

                if (oldfmt) {
                        uto = newtoold(ut);
                        fwrite(&uto, sizeof(uto), 1, stdout);
                } else
                        fwrite(&ut, sizeof(ut), 1, stdout);

		++count;
	}

	free(linestart);
}

void
usage(int result)
{
	printf("Usage: utmpdump [ -froh ] [ filename ]\n");
	exit(result);
}

int main(int argc, char **argv)
{
	int c;
	FILE *fp;
	int reverse = 0, forever = 0, oldfmt = 0;

	while ((c = getopt(argc, argv, "froh")) != EOF) {
		switch (c) {
		case 'r':
			reverse = 1;
			break;

		case 'f':
			forever = 1;
			break;

		case 'o':
			oldfmt = 1;
			break;

		case 'h':
			usage(0);
			break;

		default:
			usage(1);
		}
	}

	if (optind < argc) {
		fprintf(stderr, "Utmp %sdump of %s\n", reverse ? "un" : "", argv[optind]);
		if ((fp = fopen(argv[optind], "r")) == NULL) {
			perror("Unable to open file");
			exit(1);
		}
	}
	else {
		fprintf(stderr, "Utmp %sdump of stdin\n", reverse ? "un" : "");
		fp = stdin;
	}

	if (reverse)
		undump(fp, forever, oldfmt);
	else
		dump(fp, forever, oldfmt);

	fclose(fp);

	return 0;
}

/*
Everything after this point should be removed when the new cygwin dll is released, as this functionality has been added to it.  The current dll is 1.3.6.
*/

static const char *abb_weekdays[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
    NULL
};

static const char *full_weekdays[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    NULL
};

static const char *abb_month[] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec",
    NULL
};

static const char *full_month[] = {
    "January",
    "February",
    "Mars",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
    NULL,
};

static const char *ampm[] = {
    "am",
    "pm",
    NULL
};

/*
 * tm_year is relative this year */
const int tm_year_base = 1900;

/*
 * Return TRUE iff `year' was a leap year.
 */

static int
is_leap_year (int year)
{
    return (year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0);
}

static int
match_string (const char **buf, const char **strs)
{
    int i = 0;

    for (i = 0; strs[i] != NULL; ++i) {
	int len = strlen (strs[i]);

	if (strncasecmp (*buf, strs[i], len) == 0) {
	    *buf += len;
	    return i;
	}
    }
    return -1;
}

static int
first_day (int year)
{
    int ret = 4;

    for (; year > 1970; --year)
	ret = (ret + 365 + is_leap_year (year) ? 1 : 0) % 7;
    return ret;
}

/*
 * Set `timeptr' given `wnum' (week number [0, 53])
 */

static void
set_week_number_sun (struct tm *timeptr, int wnum)
{
    int fday = first_day (timeptr->tm_year + tm_year_base);

    timeptr->tm_yday = wnum * 7 + timeptr->tm_wday - fday;
    if (timeptr->tm_yday < 0) {
	timeptr->tm_wday = fday;
	timeptr->tm_yday = 0;
    }
}

/*
 * Set `timeptr' given `wnum' (week number [0, 53])
 */

static void
set_week_number_mon (struct tm *timeptr, int wnum)
{
    int fday = (first_day (timeptr->tm_year + tm_year_base) + 6) % 7;

    timeptr->tm_yday = wnum * 7 + (timeptr->tm_wday + 6) % 7 - fday;
    if (timeptr->tm_yday < 0) {
	timeptr->tm_wday = (fday + 1) % 7;
	timeptr->tm_yday = 0;
    }
}

/*
 * Set `timeptr' given `wnum' (week number [0, 53])
 */

static void
set_week_number_mon4 (struct tm *timeptr, int wnum)
{
    int fday = (first_day (timeptr->tm_year + tm_year_base) + 6) % 7;
    int offset = 0;

    if (fday < 4)
	offset += 7;

    timeptr->tm_yday = offset + (wnum - 1) * 7 + timeptr->tm_wday - fday;
    if (timeptr->tm_yday < 0) {
	timeptr->tm_wday = fday;
	timeptr->tm_yday = 0;
    }
}

/* strptime: roken */
char *
strptime (const char *buf, const char *format, struct tm *timeptr)
{
    char c;

    for (; (c = *format) != '\0'; ++format) {
	char *s;
	int ret;

	if (isspace (c)) {
	    while (isspace (*buf))
		++buf;
	} else if (c == '%' && format[1] != '\0') {
	    c = *++format;
	    if (c == 'E' || c == 'O')
		c = *++format;
	    switch (c) {
	    case 'A' :
		ret = match_string (&buf, full_weekdays);
		if (ret < 0)
		    return NULL;
		timeptr->tm_wday = ret;
		break;
	    case 'a' :
		ret = match_string (&buf, abb_weekdays);
		if (ret < 0)
		    return NULL;
		timeptr->tm_wday = ret;
		break;
	    case 'B' :
		ret = match_string (&buf, full_month);
		if (ret < 0)
		    return NULL;
		timeptr->tm_mon = ret;
		break;
	    case 'b' :
	    case 'h' :
		ret = match_string (&buf, abb_month);
		if (ret < 0)
		    return NULL;
		timeptr->tm_mon = ret;
		break;
	    case 'C' :
		ret = strtol (buf, &s, 10);
		if (s == buf)
		    return NULL;
		timeptr->tm_year = (ret * 100) - tm_year_base;
		buf = s;
		break;
	    case 'c' :
		abort ();
	    case 'D' :		/* %m/%d/%y */
		s = strptime (buf, "%m/%d/%y", timeptr);
		if (s == NULL)
		    return NULL;
		buf = s;
		break;
	    case 'd' :
	    case 'e' :
		ret = strtol (buf, &s, 10);
		if (s == buf)
		    return NULL;
		timeptr->tm_mday = ret;
		buf = s;
		break;
	    case 'H' :
	    case 'k' :
		ret = strtol (buf, &s, 10);
		if (s == buf)
		    return NULL;
		timeptr->tm_hour = ret;
		buf = s;
		break;
	    case 'I' :
	    case 'l' :
		ret = strtol (buf, &s, 10);
		if (s == buf)
		    return NULL;
		if (ret == 12)
		    timeptr->tm_hour = 0;
		else
		    timeptr->tm_hour = ret;
		buf = s;
		break;
	    case 'j' :
		ret = strtol (buf, &s, 10);
		if (s == buf)
		    return NULL;
		timeptr->tm_yday = ret - 1;
		buf = s;
		break;
	    case 'm' :
		ret = strtol (buf, &s, 10);
		if (s == buf)
		    return NULL;
		timeptr->tm_mon = ret - 1;
		buf = s;
		break;
	    case 'M' :
		ret = strtol (buf, &s, 10);
		if (s == buf)
		    return NULL;
		timeptr->tm_min = ret;
		buf = s;
		break;
	    case 'n' :
		if (*buf == '\n')
		    ++buf;
		else
		    return NULL;
		break;
	    case 'p' :
		ret = match_string (&buf, ampm);
		if (ret < 0)
		    return NULL;
		if (timeptr->tm_hour == 0) {
		    if (ret == 1)
			timeptr->tm_hour = 12;
		} else
		    timeptr->tm_hour += 12;
		break;
	    case 'r' :		/* %I:%M:%S %p */
		s = strptime (buf, "%I:%M:%S %p", timeptr);
		if (s == NULL)
		    return NULL;
		buf = s;
		break;
	    case 'R' :		/* %H:%M */
		s = strptime (buf, "%H:%M", timeptr);
		if (s == NULL)
		    return NULL;
		buf = s;
		break;
	    case 'S' :
		ret = strtol (buf, &s, 10);
		if (s == buf)
		    return NULL;
		timeptr->tm_sec = ret;
		buf = s;
		break;
	    case 't' :
		if (*buf == '\t')
		    ++buf;
		else
		    return NULL;
		break;
	    case 'T' :		/* %H:%M:%S */
	    case 'X' :
		s = strptime (buf, "%H:%M:%S", timeptr);
		if (s == NULL)
		    return NULL;
		buf = s;
		break;
	    case 'u' :
		ret = strtol (buf, &s, 10);
		if (s == buf)
		    return NULL;
		timeptr->tm_wday = ret - 1;
		buf = s;
		break;
	    case 'w' :
		ret = strtol (buf, &s, 10);
		if (s == buf)
		    return NULL;
		timeptr->tm_wday = ret;
		buf = s;
		break;
	    case 'U' :
		ret = strtol (buf, &s, 10);
		if (s == buf)
		    return NULL;
		set_week_number_sun (timeptr, ret);
		buf = s;
		break;
	    case 'V' :
		ret = strtol (buf, &s, 10);
		if (s == buf)
		    return NULL;
		set_week_number_mon4 (timeptr, ret);
		buf = s;
		break;
	    case 'W' :
		ret = strtol (buf, &s, 10);
		if (s == buf)
		    return NULL;
		set_week_number_mon (timeptr, ret);
		buf = s;
		break;
	    case 'x' :
		s = strptime (buf, "%Y:%m:%d", timeptr);
		if (s == NULL)
		    return NULL;
		buf = s;
		break;
	    case 'y' :
		ret = strtol (buf, &s, 10);
		if (s == buf)
		    return NULL;
		if (ret < 70)
		    timeptr->tm_year = 100 + ret;
		else
		    timeptr->tm_year = ret;
		buf = s;
		break;
	    case 'Y' :
		ret = strtol (buf, &s, 10);
		if (s == buf)
		    return NULL;
		timeptr->tm_year = ret - tm_year_base;
		buf = s;
		break;
	    case 'Z' :
		abort ();
	    case '\0' :
		--format;
		/* FALLTHROUGH */
	    case '%' :
		if (*buf == '%')
		    ++buf;
		else
		    return NULL;
		break;
	    default :
		if (*buf == '%' || *++buf == c)
		    ++buf;
		else
		    return NULL;
		break;
	    }
	} else {
	    if (*buf == c)
		++buf;
	    else
		return NULL;
	}
    }
    return (char *)buf;
}
