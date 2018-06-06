/**
 *  \file       din.c
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

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static unsigned char dins[NUM_DIN_SIGNALS];
static unsigned char dinsKb[NUM_DIN_SIGNALS];
static int tick;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
void keyb_din_parser(char c)
{
	c = c - '0';

	if (c > NUM_DIN_SIGNALS)
		return;

	dinsKb[c] ^= 1;
}

void
din_init(void)
{
    memset(dins, 0, sizeof(dins));
    memset(dinsKb, 0, sizeof(dins));
    
    tick = DIN_SCAN_PERIOD;
}

void
din_scan(void)
{
    unsigned char i;

    if(tick && (--tick != 0))
        return;
        
    tick = DIN_SCAN_PERIOD;

    for(i=0; i < NUM_DIN_SIGNALS; ++i)
    {
        if(dins[i] != dinsKb[i])
        {
            IOChgDet_put(i, dinsKb[i]);
            dins[i] = dinsKb[i];
        }
    }
}

/* ------------------------------ End of file ------------------------------ */
