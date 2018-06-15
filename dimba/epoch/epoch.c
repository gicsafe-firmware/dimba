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
typedef Epoch (*UpdStep)(void);

/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static Epoch currEpoch;
static Epoch updatingEpoch;
static UpdStep *pUpd;
static Time nextTime;
static int month;
static short year;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
static Epoch
updStep_start(void)
{
    Time *currTime;

    currTime = rtime_get();
    nextTime = *currTime;
    updatingEpoch = 0;
    return updatingEpoch;
}

static Epoch
updStep_calc1(void)
{
    month = nextTime.tm_mon;
    year = nextTime.tm_year;
    if (0 >= (int)(month -= 2))
    {                   /* 1..12 -> 11,12,1..10 */
        month += 12;    /* Puts Feb last since it has leap day */
        year -= 1;
    }
    updatingEpoch += (UL)year/4 - (UL)year/100 + year/400;
    return updatingEpoch;
}

static Epoch
updStep_calc2(void)
{
    updatingEpoch += 367*(UL)month/12 + (UL)nextTime.tm_mday + 
                     (UL)year*365 - 719499;
    return updatingEpoch;
}

static Epoch
updStep_calc3(void)
{
    updatingEpoch = (updatingEpoch * 24) + nextTime.tm_hour;
    return updatingEpoch;
}

static Epoch
updStep_calc4(void)
{
    updatingEpoch = (updatingEpoch * 60) + nextTime.tm_min;
    return updatingEpoch;
}

static Epoch
updStep_calc5(void)
{
    updatingEpoch = (updatingEpoch * 60) + nextTime.tm_sec;
    currEpoch = updatingEpoch;
    return updatingEpoch;
}

static UpdStep updSteps[] = 
{
    updStep_start, 
    updStep_calc1, 
    updStep_calc2, 
    updStep_calc3,
    updStep_calc4,
    updStep_calc5,
    (UpdStep)0
};

unsigned long
epoch_mktime(Time *stime)
{
    int month_;
    short year_;

    month_ = stime->tm_mon;
    year_ = stime->tm_year;
    if (0 >= (int)(month_ -= 2))
    {                   /* 1..12 -> 11,12,1..10 */
        month_ += 12;    /* Puts Feb last since it has leap day */
        year_ -= 1;
    }

    return (((
                (UL)year_/4 - (UL)year_/100 + year_/400 + 
                367*(UL)month_/12 + (UL)stime->tm_mday + (UL)year_*365 - 719499
             )*24 + stime->tm_hour  /* now have hours */
            )*60 + stime->tm_min    /* now have minutes */
           )*60 + stime->tm_sec;    /* finally seconds */
}

/* ---------------------------- Global functions --------------------------- */
Epoch 
epoch_init(void)
{
    pUpd = updSteps;
    epoch_updateNow();
    return currEpoch;
}

Epoch 
epoch_get(void)
{
    return currEpoch;
}

void 
epoch_updateNow(void)
{
    Time *currTime;

    currTime = rtime_get();
    currEpoch = epoch_mktime(currTime);
}

Epoch 
epoch_updateByStep(void)
{
    Epoch tempEpoch;

    tempEpoch = (*pUpd)();
    if (*(++pUpd) == (UpdStep)0)
    {
        pUpd = updSteps;
    }
    return tempEpoch;
}

/* ------------------------------ End of file ------------------------------ */
