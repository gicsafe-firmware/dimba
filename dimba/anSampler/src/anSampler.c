/*
 *  --------------------------------------------------------------------------
 *
 *                               GICSAFe-Firmware
 *                               ----------------
 *
 *                      Copyright (C) 2019 CONICET-GICSAFe
 *          All rights reserved. Protected by international copyright laws.
 *
 *  Contact information:
 *  site: https://github.com/gicsafe-firmware
 *  e-mail: <someone>@<somewhere>
 *  ---------------------------------------------------------------------------
 */

/**
 *  \file       AnSampler.c
 *  \brief      Implementation of analog signal sampler.
 */

/* -------------------------- Development history -------------------------- */
/*
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 *  CIM   Carlos Mancón manconci@gmail.com
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
int
Spy_AnSampler_getAnSampler(void)
{
    return 0;
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

    anSampler.timeStamp = epoch_get();
    for (i = 0, pAnSig = anSampler.anSignals;
         (i < NUM_AN_SIGNALS) && (result == 0);
         ++i, ++pAnSig)
    {
        value = ADConv_getSample(i);
        result = cirBuffer_put(&pAnSig->buffer, (unsigned char *)&value);
    }
    return result;
}

int
anSampler_getSet(AnSampleSet *set, int nSamples)
{
    int i, result = 0;
    AnSampleBuffer *pAnSig;

    /* enter_critical_section() */
    pAnSig = anSampler.anSignals;
    set->timeStamp = anSampler.timeStamp;
    set->timeStamp -= (AN_SAMPLING_RATE_SEC  *
                       cirBuffer_getNumElem(&pAnSig->buffer));
    for (i = 0; i < NUM_AN_SIGNALS; ++i, ++pAnSig)
    {
        result = cirBuffer_getBlock(&pAnSig->buffer,
                                    (unsigned char *)set->anSignal[i],
                                    nSamples);
    }
    /* exit_critical_section() */
    return result;
}

int
anSampler_getNumSamples(void)
{
    int nSamples;

    /* enter_critical_section() */
    nSamples = cirBuffer_getNumElem(&anSampler.anSignals[0].buffer);
    /* exit_critical_section() */
    return nSamples;
}

/* ------------------------------ End of file ------------------------------ */
