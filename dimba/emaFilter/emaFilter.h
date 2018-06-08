/**
 *  \file       emaFilter.h
 *  \brief      Exponential Moving Average Filter routines.
 *
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.01.5  DaBa  v0.0.01  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Baliña       db@vortexmakes.com
 */

/* --------------------------------- Module -------------------------------- */
#ifndef __EMAFILTER_H__
#define __EMAFILTER_H__

/* ----------------------------- Include files ----------------------------- */
#include "stdint.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
/* ------------------------------- Data types ------------------------------ */
/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */

/**
 *  \brief
 *  EMA Low Pass Filter
 *
 *  \param[in] new      New sample.
 *  \param[in] last     Last filtered value.
 *  \param[in] alph     Filtering hardness [1, 2, 4 ...],
 *                      as higher is alpha, filtering is harder.
 */
int16_t emaFilter_LowPass(int16_t new, int16_t last, uint8_t alpha);

/**
 *  \brief
 *  EMA High Pass Filter
 *
 *  \param[in] new      New sample.
 *  \param[in] last     Last filtered value.
 *  \param[in] alph     Filtering hardness [1, 2, 4 ...],
 *                      as higher is alpha, filtering is harder.
 */
int16_t emaFilter_HighPass(int16_t new, int16_t last, uint8_t alpha);

/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ File footer ------------------------------ */
