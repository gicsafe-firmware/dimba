/**
 *  \file       emaFilter.c
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

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "emaFilter.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
int16_t emaFilter_LowPass(int16_t new, int16_t last, uint8_t alpha)
{
    int16_t out;

    if(alpha == 0)
        return new;

    out = (new / alpha);
    out += last;
    out -= (last / alpha);
    return out;
}
 
int16_t emaFilter_HighPass(int16_t new, int16_t last, uint8_t alpha)
{
    if(alpha == 0)
        return new;

    last = new - emaFilter_LowPass(new, last, alpha);
    return last;
}

/* ------------------------------ End of file ------------------------------ */
