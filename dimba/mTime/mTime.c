/**
 *  \file       mTime.c
 * 	\bried      Module to manage timer interrupt.
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
#include  <stddef.h>

#include "mTime.h"
#include "mTimeCfg.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
static unsigned short counter;
static int enabled = 0;

/* ---------------------------- Global variables --------------------------- */
extern const timerChain_t timerChain[NUM_TIMER_DIVISORS];

/* ---------------------------- Local variables ---------------------------- */
static
void
execute_list(void (* const *p)(void))
{
	for(; *p != NULL; ++p)
		(**p)();
}

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
void
mTime_init(void)
{
	counter = 0;
    enabled = 1;
}

/*
 * 	This is the main timer interrupt In abstract form, this interrupt
 * 	is called each MTIME_TIME_TICK expressed in terms of msecs.
 * 	that is, if MTIME_TIME_TICK is set to 10, 
 *  then is called in each 10 milliseconds. 
 * From here, is controlled all of the timing chain for the project
 * 	This timing chain is controlled	by table 'timer_chain'
 */
void
mTime_tick(void)
{
	const timerChain_t *p;
	unsigned char num;
    
    if(!enabled)
        return;
    
	for(p = timerChain, num = NUM_TIMER_DIVISORS; num--; ++p)
		if((counter % p->timer) == 0)
			execute_list(p->ptimeact);
	if(++counter >= (p-1)->timer)
		counter = 0;
}

/* ------------------------------ End of file ------------------------------ */



