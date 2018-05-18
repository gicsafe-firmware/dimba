/**
 *  \file       Spy_anSampler.h
 *  \brief      Specification of spy for analog signal sampler.
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
/* --------------------------------- Module -------------------------------- */
#ifndef __SPY_ANSAMPLER_H__
#define __SPY_ANSAMPLER_H__

/* ----------------------------- Include files ----------------------------- */
#include "anSampler.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
/* ................................ Signals ................................ */
/* ........................ Declares active object ......................... */
/* ------------------------------- Data types ------------------------------ */
/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
int Spy_AnSampler_getAnSampler(void);

/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
