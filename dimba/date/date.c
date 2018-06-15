/**
 *  \file date.h
 *
 *	This module contains functions for manipulating dates and times,
 *	including functions for determining what the current time is and
 *	conversion between different time representations.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.06.13  LeFr  v1.0.00   Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include <string.h>
#include <stdio.h>
#include "date.h"
#include "epoch.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
#define FEBRUARY                2
#define STARTOFTIME             1970
#define SECDAY                  86400L
#define SECYR                   (SECDAY * 365)
#define leapyear(year)          ((year) % 4 == 0)
#define days_in_year(a)         (leapyear(a) ? 366 : 365)
#define days_in_month(a)        (month_days[(a) - 1])
#define MONTH_QTY               12
#define TYPE_YEARS              2

static const unsigned char month_days[TYPE_YEARS][MONTH_QTY + 1] =
{
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

static const int MonthOffset[] =
{
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};

/* ---------------------------- Local data types --------------------------- */
typedef unsigned long UL;

/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
static void
GregorianDay(Time *tm)
{
    short leapsToDate;
    short lastYear;
    long day;

    lastYear = tm->tm_year - 1;

    /* Number of leap corrections to apply up to end of last year */
    leapsToDate = lastYear / 4 - lastYear / 100 + lastYear / 400;

    /*
     * This year is a leap year if it is divisible by 4 except when it is
     * divisible by 100 unless it is divisible by 400, BUT !!! .........
     * ..... We are in year 2004 so from 1901 till 2099 there is no
     * problem with centuries because 2000 was a leap year
     *
     * e.g. 1904 was a leap year, 1900 was not, 1996 is, and 2000 will be
     */
    day = (tm->tm_year % 4 == 0) && (tm->tm_mon > 2);
    day += (UL)lastYear * 365 + (UL)leapsToDate +
           (UL)(MonthOffset[tm->tm_mon - 1]) +
           (UL)(tm->tm_mday);
    tm->tm_wday = (unsigned char)(day % 7);
}

void
mk_date(long tim, Time *tm)
{
    int i;
    int j;
    long hms, day;
    const unsigned char *p;

    day = tim / SECDAY;
    hms = tim % SECDAY;

    /* Hours, minutes, seconds are easy */
    tm->tm_hour = (unsigned char)(hms / 3600);
    hms %= 3600;
    tm->tm_min = (unsigned char)(hms / 60);
    tm->tm_sec = (unsigned char)(hms % 60);

    /* Number of years in days */
    for (i = STARTOFTIME; day >= days_in_year(i); ++i)
    {
        day -= days_in_year(i);
    }
    tm->tm_year = (short)i;

    /* Number of months in days left */
    for (i = leapyear(tm->tm_year), p = &month_days[i][j = 1];
         day >= *p; 
         ++j, ++p)
    {
        day -= *p;
    }
    tm->tm_mon = (unsigned char)j;

    /* Days are what is left over (+1) from all that. */
    tm->tm_mday = (unsigned char)(day + 1);

    /* Determine the day of week */
    GregorianDay(tm);
}

char *
str_time(char *p, const Time *time)
{
    sprintf(p, "%4d-%02d-%02d %02d:%02d:%02d",
            time->tm_year, time->tm_mon, time->tm_mday, 
            time->tm_hour, time->tm_min, time->tm_sec);
    return p;
}

unsigned long 
mk_time(Time *date)
{
    return epoch_mktime(date);
}

/* ------------------------------ End of file ------------------------------ */
