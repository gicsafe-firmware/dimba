/**
 *  \file       rtime.c
 *  \brief      Implementation of RTC abstraction for Win bsp.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.17  DaBa  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Baliña       db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include <time.h>
#include <stdio.h>

#include "rtime.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static Time t;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
Time *
rtime_get(void)
{
    time_t ltime;
    struct tm *local;

    time(&ltime);
    local = gmtime(&ltime);

    t.tm_sec = (unsigned char)local->tm_sec;
    t.tm_min = (unsigned char)local->tm_min;
    t.tm_hour = (unsigned char)local->tm_hour;
    t.tm_mday = (unsigned char)local->tm_mday;
    t.tm_mon = (unsigned char)local->tm_mon + 1;
    t.tm_year = (short)local->tm_year + 1900;
    t.tm_wday = (unsigned char)local->tm_wday + 1;
    t.tm_isdst = (unsigned char)local->tm_isdst;

    return &t;
}

void
rtime_set(Time *pt)
{
    printf("\r\nLocal time updated\r\n");
    printf("%d/%d/%d,%d:%d:%d\r\n", pt->tm_year, 
                                pt->tm_mon,
                                pt->tm_mday,
                                pt->tm_hour,
                                pt->tm_min,
                                pt->tm_sec );
}
/* ------------------------------ End of file ------------------------------ */
