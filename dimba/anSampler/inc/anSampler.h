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
 *  \file       anSampler.h
 *  \brief      Specification of analog signal sampler.
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
/* --------------------------------- Module -------------------------------- */
#ifndef __ANSAMPLER_H__
#define __ANSAMPLER_H__

/* ----------------------------- Include files ----------------------------- */
#include "CirBuffer.h"
#include "epoch.h"
#include "anin.h"
#include "mTimeCfg.h"
#include "publisher.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
#define AN_SAMPLING_RATE_SEC \
    (MTIME_ANSAMPLE_PUT_PERIOD * MTIME_TIME_TICK) / \
    1000
#define MAX_AN_NUM_SAMPLES   (32 << NUM_PUBTIME_STEPS)
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
int anSampler_getNumSamples(void);

/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
