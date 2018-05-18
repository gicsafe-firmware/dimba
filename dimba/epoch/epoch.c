/**
 *  \file       epoch.c
 *  \brief      Implementation of epoch time Unix.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.18  LeFr  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "epoch.h"
#include "rtime.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
typedef unsigned long UL;
typedef int MInt;

/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static Epoch currEpoch;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
unsigned long
mktime(Time *stime)
{
    int month;
    short year;

    month = stime->tm_mon;
    year = stime->tm_year;
    if (0 >= (int)(month -= 2))
    /*if ((month -= 2, (MInt)month) <= 0)*/
    {                   /* 1..12 -> 11,12,1..10 */
        month += 12;    /* Puts Feb last since it has leap day */
        year -= 1;
    }

    return (((
                (UL)year/4 - (UL)year/100 + year/400 + 
                367*(UL)month/12 + (UL)stime->tm_mday + (UL)year*365 - 719499
             )*24 + stime->tm_hour  /* now have hours */
            )*60 + stime->tm_min    /* now have minutes */
           )*60 + stime->tm_sec;    /* finally seconds */
}

/* ---------------------------- Global functions --------------------------- */
Epoch 
epoch_init(void)
{
    Time *currTime;

    currTime = rtime_get();
    currEpoch = mktime(currTime);
    return currEpoch;
}

Epoch 
epoch_get(void)
{
    return currEpoch;
}

void 
epoch_mkTime(void)
{
    Time *currTime;

    currTime = rtime_get();
    currEpoch = mktime(currTime);
}

/* ------------------------------ End of file ------------------------------ */
