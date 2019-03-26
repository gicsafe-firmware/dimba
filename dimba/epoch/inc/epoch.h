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
 *  \file       epoch.h
 *  \brief      Specification of epoch time of Unix.
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
/* --------------------------------- Module -------------------------------- */
#ifndef __EPOCH_H__
#define __EPOCH_H__

/* ----------------------------- Include files ----------------------------- */
/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
/* ------------------------------- Data types ------------------------------ */
typedef long Epoch;

typedef struct Time Time;
struct Time
{
    unsigned char tm_sec;       /* Seconds after minute (0,59) */
    unsigned char tm_min;       /* Minutes after hour   (0,59) */
    unsigned char tm_hour;      /* Hours after midnight (0,23) */
    unsigned char tm_mday;      /* Month day            (1,31) */
    unsigned char tm_mon;       /* Months from January  (1,12) */
    short tm_year;              /* From year 1900 */
    unsigned char tm_wday;      /* Days from Sunday     (1,7)  */
    unsigned char tm_isdst;     /* Daylight saving time flag   */
};

/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
Epoch epoch_init(void);
Epoch epoch_get(void);
void epoch_updateNow(void);
Epoch epoch_updateByStep(void);
unsigned long epoch_mktime(Time *stime);

/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
