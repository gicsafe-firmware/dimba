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
#include "rkh.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
#define AN_SAMPLING_RATE_SEC        5           
#define MAX_AN1_NUM_SAMPLES         128
#define MAX_AN2_NUM_SAMPLES         128

/* ................................ Signals ................................ */
/* ........................ Declares active object ......................... */
/* ------------------------------- Data types ------------------------------ */
typedef unsigned short SampleValue;

typedef AnSampleBuffer AnSampleBuffer;
struct AnSampleBuffer
{
    int nSamples;
    SampleValue samples[MAX_AN1_NUM_SAMPLES];
};

typedef AnSampler AnSampler;
struct AnSampler
{
    EpochTime timeStamp;    /** Updated every sampling time */
    AnSampleBuffer an1Samples;
    AnSampleBuffer an2Samples;
};

/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
