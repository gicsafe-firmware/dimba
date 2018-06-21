/**
 *  \file       adconv.h
 *  \brief      Specification of ADC abstraction.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.17  LeFr  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* --------------------------------- Module -------------------------------- */
#ifndef __ADCONV_H__
#define __ADCONV_H__

/* ----------------------------- Include files ----------------------------- */
#include "anSampler.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
#define ADCONV_RESOLUTION       10  /* In bits */
#define ADCONV_FULL_SCALE       10  /* In volts */
#define ADCONV_DIG_FRACTION     100 /* Number of digits in fractional part */

/* ------------------------------- Data types ------------------------------ */

/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
SampleValue ADConv_getSample(int channel);

/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
