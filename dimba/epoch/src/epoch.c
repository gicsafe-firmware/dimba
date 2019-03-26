/*
 *  --------------------------------------------------------------------------
 *
 *                               GICSAFe-Firmware
 *                               ----------------
 *
 *                      Copyright (C) 2019 CONICET-GICSAFe
 *          All rights reserved. Protected by international copyright laws.
 *
 *  Contact information:
 *  site: https://github.com/gicsafe-firmware
 *  e-mail: <someone>@<somewhere>
 *  ---------------------------------------------------------------------------
 */

/**
 *  \file       epoch.c
 *  \brief      Implementation of epoch time Unix.
 */

/* -------------------------- Development history -------------------------- */
/*
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 *  CIM   Carlos Mancón manconci@gmail.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "epoch.h"
#include "rtime.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
static const Time dftTime = {0, 0, 0, 1, 1, 1970, 3, 0};

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
static int
verifyTime(Time *tm)
{
    int isValid = 0;

    if ((tm->tm_sec > 59) ||
        (tm->tm_min > 59) ||
        (tm->tm_hour > 24) ||
        ((tm->tm_mday == 0) || (tm->tm_mday > 31)) ||
        ((tm->tm_mon == 0) || (tm->tm_mon > 12)) ||
        (tm->tm_year < 1970) ||
        ((tm->tm_wday == 0) || (tm->tm_wday > 8)))
    {
        isValid = 1;
    }
    return isValid;
}

static Epoch
updStep_start(void)
{
    int isValid;
    Time *currTime;

    currTime = rtime_get();
    isValid = verifyTime(currTime);
    nextTime = (isValid == 0) ? *currTime : (Time)dftTime;
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
    updatingEpoch += (UL)year / 4 - (UL)year / 100 + year / 400;
    return updatingEpoch;
}

static Epoch
updStep_calc2(void)
{
    updatingEpoch += 367 * (UL)month / 12 + (UL)nextTime.tm_mday +
                     (UL)year * 365 - 719499;
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
                 (UL)year_ / 4 - (UL)year_ / 100 + year_ / 400 +
                 367 * (UL)month_ / 12 + (UL)stime->tm_mday + (UL)year_ * 365 -
                 719499
                 ) * 24 + stime->tm_hour /* now have hours */
             ) * 60 + stime->tm_min /* now have minutes */
            ) * 60 + stime->tm_sec; /* finally seconds */
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
    int isValid;
    Time *currTime;

    currTime = rtime_get();
    isValid = verifyTime(currTime);
    if (isValid == 1)
    {
        currTime = (Time *)&dftTime;
    }
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
