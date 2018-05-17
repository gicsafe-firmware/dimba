/**
 *  \file       AnSampler.c
 *  \brief      Implementation of analog signal sampler.
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
#include "anSampler.h"
#include "CirBuffer.h"
#include "epoch.h"
#include "adconv.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
typedef struct AnSampleBuffer AnSampleBuffer;
struct AnSampleBuffer
{
    CirBuffer buffer;
    SampleValue samples[MAX_AN_NUM_SAMPLES];
};

typedef struct AnSampler AnSampler;
struct AnSampler
{
    Epoch timeStamp;    /** Updated every sampling time */
    AnSampleBuffer anSignals[NUM_AN_SIGNALS];
};

/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static AnSampler anSampler;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
SampleValue 
convertToSampleValue(ADCSampleUnit sample)
{
    double value;
    int whole, fraction, result;

    value = ((double)(sample * 15) / (1<<10));
    whole = (int)value;
    fraction = (int)(value * 100);
    fraction = fraction - (whole * 100);

    result = fraction;
    result |= whole << 8;
    return (SampleValue)result;
}

/* ---------------------------- Global functions --------------------------- */
int 
anSampler_init(void)
{
    int i, result = 0;
    AnSampleBuffer *pAnSig;

    for (i = 0, pAnSig = anSampler.anSignals; 
         (i < NUM_AN_SIGNALS) && (result == 0); 
         ++i, ++pAnSig)
    {
        result = cirBuffer_init(&pAnSig->buffer, 
                                (unsigned char *)pAnSig->samples, 
                                sizeof(SampleValue), 
                                MAX_AN_NUM_SAMPLES);
    }
    return result;
}

int
anSampler_put(void)
{
    int i, result = 0;
    AnSampleBuffer *pAnSig;
    SampleValue value;
    ADCSampleUnit adValue;

    anSampler.timeStamp = epoch_get();
    for (i = 0, pAnSig = anSampler.anSignals; 
         (i < NUM_AN_SIGNALS) && (result == 0); 
         ++i, ++pAnSig)
    {
        adValue = ADConv_getSample(i);
        value = convertToSampleValue(adValue);
        result = cirBuffer_put(&pAnSig->buffer, (unsigned char *)&value);
    }
    return result;
}

/* ------------------------------ End of file ------------------------------ */
