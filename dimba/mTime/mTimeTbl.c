/**
 *  \file       mTimeTble.c
 * 	\bried      mTime timers Table.
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
#include <stdio.h>

#include "mTime.h"
#include "mTimeCfg.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
void
none( void )
{
}

static void(* const actions_2[])( void ) =
{
	none, 
    NULL
};

static void(* const actions_20[])( void ) =
{
	none, NULL
};

static void(* const actions_100[])( void ) =
{
	NULL
};

const timerChain_t timerChain[] =
{
	{ 2000/MTIME_TIME_TICK,		actions_2 		},
	{ 20000/MTIME_TIME_TICK, 	actions_20 		},
	{ 100000/MTIME_TIME_TICK,	actions_100		}
};

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
/* ------------------------------ End of file ------------------------------ */





