/**
 *  \file       anSampler.h
 *  \brief      Specification of analog signal sampler.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.14  LeFr  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* --------------------------------- Module -------------------------------- */
#ifndef __ANSAMPLER_H__
#define __ANSAMPLER_H__

/* ----------------------------- Include files ----------------------------- */
#include "CirBuffer.h"
#include "epoch.h"
#include "anin.h"
#include "mTimeCfg.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
#define AN_SAMPLING_RATE_SEC (MTIME_ANSAMPLE_PUT_PERIOD * MTIME_TIME_TICK)/1000
#define MAX_AN_NUM_SAMPLES   128
#define NUM_AN_SIGNALS       NUM_ANIN_SIGNALS

/* ................................ Signals ................................ */
/* ........................ Declares active object ......................... */
/* ------------------------------- Data types ------------------------------ */
typedef unsigned short SampleValue;
typedef SampleValue AnSignalValue[MAX_AN_NUM_SAMPLES];
typedef struct AnSampleSet AnSampleSet;
struct AnSampleSet
{
    Epoch timeStamp;    /** Updated every sampling time */
    AnSignalValue anSignal[NUM_AN_SIGNALS];
};

/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
int anSampler_init(void);
int anSampler_put(void);
int anSampler_getSet(AnSampleSet *set, int nSamples);

/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
