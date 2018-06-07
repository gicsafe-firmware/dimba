/**
 *  \file       anin.c
 *  \brief      Implementation of Analog Inputs adquisition and filtering.
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
#include "anin.h"
#include "mTimeCfg.h"
#include "emaFilter.h"
#include "anSampler.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
#define ANINS_EMA_ALPHA     2
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static adc_t anIns[NUM_ANIN_SIGNALS];

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
adc_t
anIn_adcRead(int channel)
{   
    (void)channel;

    return 0x5A5A;
}

/* ---------------------------- Global functions --------------------------- */
void
anIn_init(void)
{
    memset(anIns, 0, sizeof(anIns));
}

void
anIn_captureAndFilter(void)
{
    unsigned char i;
    int16_t value;

    for(i=0; i < NUM_ANIN_SIGNALS; ++i)
    {
        value = anIn_adcRead(i);
        anIns[i] = emaFilter_LowPass(value, anIns[i], ANINS_EMA_ALPHA);
    }
}

adc_t
anIn_get(int channel)
{
    if(channel > NUM_ANIN_SIGNALS)
        return 0;

    return anIns[channel];
}

void
anIn_update(void)
{
   anSampler_put();
}

/* ------------------------------ End of file ------------------------------ */
