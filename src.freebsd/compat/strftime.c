/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 *
 * Portions of this software were developed by David Chisnall
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley. The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
#ifndef NOID
static const char	elsieid[] = "@(#)strftime.3	8.3";
/*
 * Based on the UCB version with the ID appearing below.
 * This is ANSIish only when "multibyte character == plain character".
 */
#endif /* !defined NOID */
#endif /* !defined lint */

#if defined(LIBC_SCCS) && !defined(lint)
static const char	sccsid[] = "@(#)strftime.c	5.4 (Berkeley) 3/14/89";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <langinfo.h>
 
#define DAYSPERWEEK 7
#define MONSPERYEAR 12
#define HOURSPERDAY 24
#define SECSPERMIN 60
#define MINSPERHOUR 60
#define DAYSPERNYEAR 365
#define DAYSPERLYEAR 366
#define TM_YEAR_BASE 1900
#define TM_GMTOFF tm_gmtoff
#define TM_ZONE tm_zone

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#define isleap_sum(a, b) isleap((a) % 400 + (b) % 400)

static char *	_add(const char *, char *, const char *);
static char *	_conv(int, const char *, char *, const char *);
static char *	_fmt(const char *, const struct tm *, char *, const char *);
static char *	_yconv(int, int, int, int, char *, const char *);

extern char *	tzname[];

#ifndef YEAR_2000_NAME
#define YEAR_2000_NAME	"CHECK_STRFTIME_FORMATS_FOR_TWO_DIGIT_YEARS"
#endif /* !defined YEAR_2000_NAME */

#define	IN_NONE	0
#define	IN_SOME	1
#define	IN_THIS	2
#define	IN_ALL	3

#define	PAD_DEFAULT	0
#define	PAD_LESS	1
#define	PAD_SPACE	2
#define	PAD_ZERO	3

static const char fmt_padding[][4][5] = {
	/* DEFAULT,	LESS,	SPACE,	ZERO */
#define	PAD_FMT_MONTHDAY	0
#define	PAD_FMT_HMS		0
#define	PAD_FMT_CENTURY		0
#define	PAD_FMT_SHORTYEAR	0
#define	PAD_FMT_MONTH		0
#define	PAD_FMT_WEEKOFYEAR	0
#define	PAD_FMT_DAYOFMONTH	0
	{ "%02d",	"%d",	"%2d",	"%02d" },
#define	PAD_FMT_SDAYOFMONTH	1
#define	PAD_FMT_SHMS		1
	{ "%2d",	"%d",	"%2d",	"%02d" },
#define	PAD_FMT_DAYOFYEAR	2
	{ "%03d",	"%d",	"%3d",	"%03d" },
#define	PAD_FMT_YEAR		3
	{ "%04d",	"%d",	"%4d",	"%04d" }
};

size_t
strftime_bsd(char * __restrict s, size_t maxsize, const char * __restrict format,
    const struct tm * __restrict t)
{
	char *	p;
	tzset();
	p = _fmt(((format == NULL) ? "%c" : format), t, s, s + maxsize);
	if (p == s + maxsize)
		return (0);
	*p = '\0';
	return p - s;
}

static char *
_fmt(const char *format, const struct tm * const t, char *pt,
    const char * const ptlim)
{
	int Ealternative, Oalternative, PadIndex;

	for ( ; *format; ++format) {
		if (*format == '%') {
			Ealternative = 0;
			Oalternative = 0;
			PadIndex	 = PAD_DEFAULT;
label:
			switch (*++format) {
			case '\0':
				--format;
				break;
			case 'A':
				pt = _add((t->tm_wday < 0 ||
					t->tm_wday >= DAYSPERWEEK) ?
					"?" : nl_langinfo(DAY_1 + t->tm_wday),
					pt, ptlim);
				continue;
			case 'a':
				pt = _add((t->tm_wday < 0 ||
					t->tm_wday >= DAYSPERWEEK) ?
					"?" : nl_langinfo(ABDAY_1 + t->tm_wday),
					pt, ptlim);
				continue;
			case 'B':
				pt = _add((t->tm_mon < 0 ||
					t->tm_mon >= MONSPERYEAR) ?
					"?" : nl_langinfo(MON_1 + t->tm_mon),
					pt, ptlim);
				continue;
			case 'b':
			case 'h':
				pt = _add((t->tm_mon < 0 ||
					t->tm_mon >= MONSPERYEAR) ?
					"?" : nl_langinfo(ABMON_1 + t->tm_mon),
					pt, ptlim);
				continue;
			case 'C':
				/*
				 * %C used to do a...
				 *	_fmt("%a %b %e %X %Y", t);
				 * ...whereas now POSIX 1003.2 calls for
				 * something completely different.
				 * (ado, 1993-05-24)
				 */
				pt = _yconv(t->tm_year, TM_YEAR_BASE, 1, 0,
					pt, ptlim);
				continue;
			case 'c':
				pt = _fmt(nl_langinfo(D_T_FMT), t, pt, ptlim);
				continue;
			case 'D':
				pt = _fmt("%m/%d/%y", t, pt, ptlim);
				continue;
			case 'd':
				pt = _conv(t->tm_mday,
					fmt_padding[PAD_FMT_DAYOFMONTH][PadIndex],
					pt, ptlim);
				continue;
			case 'E':
				if (Ealternative || Oalternative)
					break;
				Ealternative++;
				goto label;
			case 'O':
				/*
				 * C99 locale modifiers.
				 * The sequences
				 *	%Ec %EC %Ex %EX %Ey %EY
				 *	%Od %oe %OH %OI %Om %OM
				 *	%OS %Ou %OU %OV %Ow %OW %Oy
				 * are supposed to provide alternate
				 * representations.
				 *
				 * FreeBSD extension
				 *      %OB
				 */
				if (Ealternative || Oalternative)
					break;
				Oalternative++;
				goto label;
			case 'e':
				pt = _conv(t->tm_mday,
					fmt_padding[PAD_FMT_SDAYOFMONTH][PadIndex],
					pt, ptlim);
				continue;
			case 'F':
				pt = _fmt("%Y-%m-%d", t, pt, ptlim);
				continue;
			case 'H':
				pt = _conv(t->tm_hour, fmt_padding[PAD_FMT_HMS][PadIndex],
					pt, ptlim);
				continue;
			case 'I':
				pt = _conv((t->tm_hour % 12) ?
					(t->tm_hour % 12) : 12,
					fmt_padding[PAD_FMT_HMS][PadIndex],
					pt, ptlim);
				continue;
			case 'j':
				pt = _conv(t->tm_yday + 1,
					fmt_padding[PAD_FMT_DAYOFYEAR][PadIndex],
					pt, ptlim);
				continue;
			case 'k':
				/*
				 * This used to be...
				 *	_conv(t->tm_hour % 12 ?
				 *		t->tm_hour % 12 : 12, 2, ' ');
				 * ...and has been changed to the below to
				 * match SunOS 4.1.1 and Arnold Robbins'
				 * strftime version 3.0. That is, "%k" and
				 * "%l" have been swapped.
				 * (ado, 1993-05-24)
				 */
				pt = _conv(t->tm_hour, fmt_padding[PAD_FMT_SHMS][PadIndex],
					pt, ptlim);
				continue;
#ifdef KITCHEN_SINK
			case 'K':
				/*
				** After all this time, still unclaimed!
				*/
				pt = _add("kitchen sink", pt, ptlim);
				continue;
#endif /* defined KITCHEN_SINK */
			case 'l':
				/*
				 * This used to be...
				 *	_conv(t->tm_hour, 2, ' ');
				 * ...and has been changed to the below to
				 * match SunOS 4.1.1 and Arnold Robbin's
				 * strftime version 3.0. That is, "%k" and
				 * "%l" have been swapped.
				 * (ado, 1993-05-24)
				 */
				pt = _conv((t->tm_hour % 12) ?
					(t->tm_hour % 12) : 12,
					fmt_padding[PAD_FMT_SHMS][PadIndex],
					pt, ptlim);
				continue;
			case 'M':
				pt = _conv(t->tm_min, fmt_padding[PAD_FMT_HMS][PadIndex],
					pt, ptlim);
				continue;
			case 'm':
				pt = _conv(t->tm_mon + 1,
					fmt_padding[PAD_FMT_MONTH][PadIndex],
					pt, ptlim);
				continue;
			case 'n':
				pt = _add("\n", pt, ptlim);
				continue;
			case 'p':
				pt = _add(nl_langinfo((t->tm_hour >= (HOURSPERDAY / 2)) ?
					PM_STR : AM_STR),
					pt, ptlim);
				continue;
			case 'R':
				pt = _fmt("%H:%M", t, pt, ptlim);
				continue;
			case 'r':
				pt = _fmt(nl_langinfo(T_FMT_AMPM), t, pt, ptlim);
				continue;
			case 'S':
				pt = _conv(t->tm_sec, fmt_padding[PAD_FMT_HMS][PadIndex],
					pt, ptlim);
				continue;
			case 's':
				{
					struct tm	tm;
					char		buf[32];
					time_t		mkt;

					tm = *t;
					mkt = mktime(&tm);
					(void) sprintf(buf, "%lld", (long long) mkt);
					pt = _add(buf, pt, ptlim);
				}
				continue;
			case 'T':
				pt = _fmt("%H:%M:%S", t, pt, ptlim);
				continue;
			case 't':
				pt = _add("\t", pt, ptlim);
				continue;
			case 'U':
				pt = _conv((t->tm_yday + DAYSPERWEEK -
					t->tm_wday) / DAYSPERWEEK,
					fmt_padding[PAD_FMT_WEEKOFYEAR][PadIndex],
					pt, ptlim);
				continue;
			case 'u':
				/*
				 * From Arnold Robbins' strftime version 3.0:
				 * "ISO 8601: Weekday as a decimal number
				 * [1 (Monday) - 7]"
				 * (ado, 1993-05-24)
				 */
				pt = _conv((t->tm_wday == 0) ?
					DAYSPERWEEK : t->tm_wday,
					"%d", pt, ptlim);
				continue;
			case 'V':	/* ISO 8601 week number */
			case 'G':	/* ISO 8601 year (four digits) */
			case 'g':	/* ISO 8601 year (two digits) */
/*
 * From Arnold Robbins' strftime version 3.0: "the week number of the
 * year (the first Monday as the first day of week 1) as a decimal number
 * (01-53)."
 * (ado, 1993-05-24)
 *
 * From "http://www.ft.uni-erlangen.de/~mskuhn/iso-time.html" by Markus Kuhn:
 * "Week 01 of a year is per definition the first week which has the
 * Thursday in this year, which is equivalent to the week which contains
 * the fourth day of January. In other words, the first week of a new year
 * is the week which has the majority of its days in the new year. Week 01
 * might also contain days from the previous year and the week before week
 * 01 of a year is the last week (52 or 53) of the previous year even if
 * it contains days from the new year. A week starts with Monday (day 1)
 * and ends with Sunday (day 7). For example, the first week of the year
 * 1997 lasts from 1996-12-30 to 1997-01-05..."
 * (ado, 1996-01-02)
 */
				{
					int	year;
					int	base;
					int	yday;
					int	wday;
					int	w;

					year = t->tm_year;
					base = TM_YEAR_BASE;
					yday = t->tm_yday;
					wday = t->tm_wday;
					for ( ; ; ) {
						int	len;
						int	bot;
						int	top;

						len = isleap_sum(year, base) ?
							DAYSPERLYEAR :
							DAYSPERNYEAR;
						/*
						 * What yday (-3 ... 3) does
						 * the ISO year begin on?
						 */
						bot = ((yday + 11 - wday) %
							DAYSPERWEEK) - 3;
						/*
						 * What yday does the NEXT
						 * ISO year begin on?
						 */
						top = bot -
							(len % DAYSPERWEEK);
						if (top < -3)
							top += DAYSPERWEEK;
						top += len;
						if (yday >= top) {
							++base;
							w = 1;
							break;
						}
						if (yday >= bot) {
							w = 1 + ((yday - bot) /
								DAYSPERWEEK);
							break;
						}
						--base;
						yday += isleap_sum(year, base) ?
							DAYSPERLYEAR :
							DAYSPERNYEAR;
					}
#ifdef XPG4_1994_04_09
					if ((w == 52 &&
						t->tm_mon == TM_JANUARY) ||
						(w == 1 &&
						t->tm_mon == TM_DECEMBER))
							w = 53;
#endif /* defined XPG4_1994_04_09 */
					if (*format == 'V')
						pt = _conv(w, fmt_padding[PAD_FMT_WEEKOFYEAR][PadIndex],
							pt, ptlim);
					else if (*format == 'g') {
						pt = _yconv(year, base, 0, 1,
							pt, ptlim);
					} else	pt = _yconv(year, base, 1, 1,
							pt, ptlim);
				}
				continue;
			case 'v':
				/*
				 * From Arnold Robbins' strftime version 3.0:
				 * "date as dd-bbb-YYYY"
				 * (ado, 1993-05-24)
				 */
				pt = _fmt("%e-%b-%Y", t, pt, ptlim);
				continue;
			case 'W':
				pt = _conv((t->tm_yday + DAYSPERWEEK -
					(t->tm_wday ?
					(t->tm_wday - 1) :
					(DAYSPERWEEK - 1))) / DAYSPERWEEK,
					fmt_padding[PAD_FMT_WEEKOFYEAR][PadIndex],
					pt, ptlim);
				continue;
			case 'w':
				pt = _conv(t->tm_wday, "%d", pt, ptlim);
				continue;
			case 'X':
				pt = _fmt(nl_langinfo(T_FMT), t, pt, ptlim);
				continue;
			case 'x':
				pt = _fmt(nl_langinfo(D_FMT), t, pt, ptlim);
				continue;
			case 'y':
				pt = _yconv(t->tm_year, TM_YEAR_BASE, 0, 1,
					pt, ptlim);
				continue;
			case 'Y':
				pt = _yconv(t->tm_year, TM_YEAR_BASE, 1, 1,
					pt, ptlim);
				continue;
			case 'Z':
#ifdef TM_ZONE
				if (t->TM_ZONE != NULL)
					pt = _add(t->TM_ZONE, pt, ptlim);
				else
#endif /* defined TM_ZONE */
				if (t->tm_isdst >= 0)
					pt = _add(tzname[t->tm_isdst != 0],
						pt, ptlim);
				/*
				 * C99 says that %Z must be replaced by the
				 * empty string if the time zone is not
				 * determinable.
				 */
				continue;
			case 'z':
				{
				int		diff;
				char const *	sign;

				if (t->tm_isdst < 0)
					continue;
#ifdef TM_GMTOFF
				diff = t->TM_GMTOFF;
#else /* !defined TM_GMTOFF */
				/*
				 * C99 says that the UTC offset must
				 * be computed by looking only at
				 * tm_isdst. This requirement is
				 * incorrect, since it means the code
				 * must rely on magic (in this case
				 * altzone and timezone), and the
				 * magic might not have the correct
				 * offset. Doing things correctly is
				 * tricky and requires disobeying C99;
				 * see GNU C strftime for details.
				 * For now, punt and conform to the
				 * standard, even though it's incorrect.
				 *
				 * C99 says that %z must be replaced by the
				 * empty string if the time zone is not
				 * determinable, so output nothing if the
				 * appropriate variables are not available.
				 */
				if (t->tm_isdst == 0)
#ifdef USG_COMPAT
					diff = -timezone;
#else /* !defined USG_COMPAT */
					continue;
#endif /* !defined USG_COMPAT */
				else
#ifdef ALTZONE
					diff = -altzone;
#else /* !defined ALTZONE */
					continue;
#endif /* !defined ALTZONE */
#endif /* !defined TM_GMTOFF */
				if (diff < 0) {
					sign = "-";
					diff = -diff;
				} else
					sign = "+";
				pt = _add(sign, pt, ptlim);
				diff /= SECSPERMIN;
				diff = (diff / MINSPERHOUR) * 100 +
					(diff % MINSPERHOUR);
				pt = _conv(diff,
					fmt_padding[PAD_FMT_YEAR][PadIndex],
					pt, ptlim);
				}
				continue;
			case '+':
#ifdef _DATE_FMT
				pt = _fmt(nl_langinfo(_DATE_FMT), t, pt, ptlim);
#else
				pt = _fmt("%a %b %e %H:%M:%S %Z %Y", t, pt, ptlim);
#endif
				continue;
			case '-':
				if (PadIndex != PAD_DEFAULT)
					break;
				PadIndex = PAD_LESS;
				goto label;
			case '_':
				if (PadIndex != PAD_DEFAULT)
					break;
				PadIndex = PAD_SPACE;
				goto label;
			case '0':
				if (PadIndex != PAD_DEFAULT)
					break;
				PadIndex = PAD_ZERO;
				goto label;
			case '%':
			/*
			 * X311J/88-090 (4.12.3.5): if conversion char is
			 * undefined, behavior is undefined. Print out the
			 * character itself as printf(3) also does.
			 */
			default:
				break;
			}
		}
		if (pt == ptlim)
			break;
		*pt++ = *format;
	}
	return (pt);
}

static char *
_conv(const int n, const char * const format, char * const pt,
    const char * const ptlim)
{
	char	buf[16];

	(void) sprintf(buf, format, n);
	return _add(buf, pt, ptlim);
}

static char *
_add(const char *str, char *pt, const char * const ptlim)
{
	while (pt < ptlim && (*pt = *str++) != '\0')
		++pt;
	return (pt);
}

/*
 * POSIX and the C Standard are unclear or inconsistent about
 * what %C and %y do if the year is negative or exceeds 9999.
 * Use the convention that %C concatenated with %y yields the
 * same output as %Y, and that %Y contains at least 4 bytes,
 * with more only if necessary.
 */

static char *
_yconv(const int a, const int b, const int convert_top, const int convert_yy,
    char *pt, const char * const ptlim)
{
	register int	lead;
	register int	trail;

#define	DIVISOR	100
	trail = a % DIVISOR + b % DIVISOR;
	lead = a / DIVISOR + b / DIVISOR + trail / DIVISOR;
	trail %= DIVISOR;
	if (trail < 0 && lead > 0) {
		trail += DIVISOR;
		--lead;
	} else if (lead < 0 && trail > 0) {
		trail -= DIVISOR;
		++lead;
	}
	if (convert_top) {
		if (lead == 0 && trail < 0)
			pt = _add("-0", pt, ptlim);
		else	pt = _conv(lead, "%02d", pt, ptlim);
	}
	if (convert_yy)
		pt = _conv(((trail < 0) ? -trail : trail), "%02d", pt,
		     ptlim);
	return (pt);
}
