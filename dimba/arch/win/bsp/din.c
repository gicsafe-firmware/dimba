/**
 *  \file       dIn.c
 *  \brief      Implementation of Digital Inputs HAL and change detection.
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
#include "rkh.h"
#include "IOChgDet.h"
#include "din.h"
#include "mTimeCfg.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static unsigned char dIns[NUM_DIN_SIGNALS];
static unsigned char dInsKb[NUM_DIN_SIGNALS];

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
void keyb_dIn_parser(char c)
{
	c = c - '0';

	if (c > NUM_DIN_SIGNALS)
		return;

	dInsKb[c] ^= 1;
}

void
dIn_init(void)
{
    memset(dIns, 0, sizeof(dIns));
    memset(dInsKb, 0, sizeof(dIns));
}

void
dIn_scan(void)
{
    unsigned char i;

    for(i=0; i < NUM_DIN_SIGNALS; ++i)
    {
        if(dIns[i] != dInsKb[i])
        {
            IOChgDet_put(i, dInsKb[i]);
            dIns[i] = dInsKb[i];
        }
    }
}

/* ------------------------------ End of file ------------------------------ */
