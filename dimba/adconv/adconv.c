/**
 *  \file       adconv.c
 *  \brief      Implementation of ADC abstraction.
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
/* ----------------------------- Include files ----------------------------- */
#include "adconv.h"
#include "anSampler.h"
#include "anIn.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
typedef unsigned short ADCSampleUnit;

/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
SampleValue 
convertToSampleValue(ADCSampleUnit sample)
{
    double value;
    int whole, fraction, result;

    value = ((double)(sample * ADCONV_FULL_SCALE) / (1 << ADCONV_RESOLUTION));
    whole = (int)value;
    fraction = (int)(value * ADCONV_DIG_FRACTION);
    fraction = fraction - (whole * ADCONV_DIG_FRACTION);

    result = fraction;
    result |= whole << 8;
    return (SampleValue)result;
}

/* ---------------------------- Global functions --------------------------- */
SampleValue
ADConv_getSample(int channel)
{
    ADCSampleUnit sample;

    sample = anIn_get(channel);
    return convertToSampleValue(sample);
}

/* ------------------------------ End of file ------------------------------ */
