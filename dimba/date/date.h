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
/* --------------------------------- Module -------------------------------- */
#ifndef __DATE_H__
#define __DATE_H__

/* ----------------------------- Include files ----------------------------- */
#include "epoch.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
#define TM_YEAR_BASE    1900    /* tm_year is relative this year */

/* ------------------------------- Data types ------------------------------ */
/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
/**
 *  \brief
 *	Convert time format to date format.
 *
 *	The function converts the time to the fields in the output structure,
 *	taking into account leap years. The time is since 0:00:00.00, January 1,
 *	1970.
 *
 *	\param time		time in seconds to be converted.
 *	\param date		pointer to the corresponding date structure.
 */
void mk_date(long time, Time *date);

/**
 *  \brief
 *	Convert second time format from date format.
 *
 *	The function converts the field in the input structure to time, taking
 *	into account leap years. The time is since 0:00:00.00, January 1, 1970.
 *
 *	\param date		pointer to a date structure.
 *
 *	\return
 *	Time in seconds.
 */
unsigned long mk_time(Time *date);

/**
 *  \brief
 *  Sets the local time according to given date structure.
 *
 *	\param date		pointer to a date to be set.
 *
 *  \return
 *  True (1) on success, false (0) otherwise.
 */
int set_time(Time *date);

/**
 *  \brief
 *  Format time as string.
 */
char *str_time(char *p, const Time *time);

/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
