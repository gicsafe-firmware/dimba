/**
 *  \file       mTime.h
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
/* --------------------------------- Module -------------------------------- */
#ifndef __MTIME_H__
#define __MTIME_H__

/* ----------------------------- Include files ----------------------------- */
/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
/* ------------------------------- Data types ------------------------------ */
typedef struct
{
	unsigned short timer;
	void (* const *ptimeact)(void);
} timerChain_t;

/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
void mTime_init(void);

/*
 * 	This is the main timer interrupt In abstract form, this interrupt
 * 	is called each MTIME_TIME_TICK expressed in terms of msecs.
 * 	that is, if MTIME_TIME_TICK is set to 10, 
 *  then is called in each 10 milliseconds. 
 * From here, is controlled all of the timing chain for the project
 * 	This timing chain is controlled	by table 'timer_chain'
 */
void mTime_tick(void);


/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
