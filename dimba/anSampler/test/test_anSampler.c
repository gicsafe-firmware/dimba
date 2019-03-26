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
 *  \file       test_AnSampler.c
 *  \brief      Unit test for analog signal sampler.
 */

/* -------------------------- Development history -------------------------- */
/*
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci  lf@vortexmakes.com
 *  CIM   Carlos Mancón manconci@gmail.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "unity.h"
#include "anSampler.h"
#include "Spy_anSampler.h"
#include "Mock_CirBuffer.h"
#include "Mock_epoch.h"
#include "Mock_adconv.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
void
setUp(void)
{
    Mock_CirBuffer_Init();
    Mock_epoch_Init();
}

void
tearDown(void)
{
    Mock_CirBuffer_Verify();
    Mock_epoch_Verify();
    Mock_CirBuffer_Destroy();
    Mock_epoch_Destroy();
}

void
test_ClearAfterInit(void)
{
    int i, result;

    for (i = 0; i < NUM_AN_SIGNALS; ++i)
    {
        cirBuffer_init_ExpectAndReturn(0, 0, 0, MAX_AN_NUM_SAMPLES, 0);
        cirBuffer_init_IgnoreArg_me();
        cirBuffer_init_IgnoreArg_sto();
        cirBuffer_init_IgnoreArg_elemSize();
    }

    result = anSampler_init();
    TEST_ASSERT_EQUAL(0, result);
}

void
test_FailsOneAnSamplerOnInit(void)
{
    int result;

    cirBuffer_init_ExpectAndReturn(0, 0, 0, MAX_AN_NUM_SAMPLES, 1);
    cirBuffer_init_IgnoreArg_me();
    cirBuffer_init_IgnoreArg_sto();
    cirBuffer_init_IgnoreArg_elemSize();

    result = anSampler_init();
    TEST_ASSERT_EQUAL(1, result);

    cirBuffer_init_ExpectAndReturn(0, 0, 0, MAX_AN_NUM_SAMPLES, 0);
    cirBuffer_init_IgnoreArg_me();
    cirBuffer_init_IgnoreArg_sto();
    cirBuffer_init_IgnoreArg_elemSize();
    cirBuffer_init_ExpectAndReturn(0, 0, 0, MAX_AN_NUM_SAMPLES, 1);
    cirBuffer_init_IgnoreArg_me();
    cirBuffer_init_IgnoreArg_sto();
    cirBuffer_init_IgnoreArg_elemSize();

    result = anSampler_init();
    TEST_ASSERT_EQUAL(1, result);
}

void
test_StoreOneAnSample(void)
{
    int i, result;

    for (i = 0; i < NUM_AN_SIGNALS; ++i)
    {
        cirBuffer_init_ExpectAndReturn(0, 0, 0, MAX_AN_NUM_SAMPLES, 0);
        cirBuffer_init_IgnoreArg_me();
        cirBuffer_init_IgnoreArg_sto();
        cirBuffer_init_IgnoreArg_elemSize();
    }

    result = anSampler_init();
    TEST_ASSERT_EQUAL(0, result);

    epoch_get_ExpectAndReturn(123456);
    for (i = 0; i < NUM_AN_SIGNALS; ++i)
    {
        ADConv_getSample_ExpectAndReturn(i, 707);
        cirBuffer_put_ExpectAndReturn(0, 0, 0);
        cirBuffer_put_IgnoreArg_me();
        cirBuffer_put_IgnoreArg_elem();
    }

    result = anSampler_put();
    TEST_ASSERT_EQUAL(0, result);
}

void
test_GetAnSampleSet(void)
{
    AnSampleSet set;
    int i, result, nReqSamples, nBufSamples;

    epoch_get_ExpectAndReturn(123456);
    for (i = 0; i < NUM_AN_SIGNALS; ++i)
    {
        ADConv_getSample_ExpectAndReturn(i, 707);
        cirBuffer_put_ExpectAndReturn(0, 0, 0);
        cirBuffer_put_IgnoreArg_me();
        cirBuffer_put_IgnoreArg_elem();
    }

    anSampler_put();

    nBufSamples = 16;
    nReqSamples = 8;
    cirBuffer_getNumElem_ExpectAndReturn(0, nBufSamples);
    cirBuffer_getNumElem_IgnoreArg_me();
    for (i = 0; i < NUM_AN_SIGNALS; ++i)
    {
        cirBuffer_getBlock_ExpectAndReturn(0, 0, nReqSamples, nReqSamples);
        cirBuffer_getBlock_IgnoreArg_me();
        cirBuffer_getBlock_IgnoreArg_destBlock();
    }

    result = anSampler_getSet(&set, nReqSamples);
    TEST_ASSERT_EQUAL(nReqSamples, result);
    TEST_ASSERT_EQUAL(123456 - (nBufSamples * AN_SAMPLING_RATE_SEC),
                      set.timeStamp);
}

void
test_GetTotalNumOfStoredSamples(void)
{
    AnSampleSet set;
    int nSamples, i, j, result, nReqSamples, nBufSamples;

    for (j = 0; j < 4; ++j)
    {
        epoch_get_ExpectAndReturn(123456);
        for (i = 0; i < NUM_AN_SIGNALS; ++i)
        {
            ADConv_getSample_ExpectAndReturn(i, 707);
            cirBuffer_put_ExpectAndReturn(0, 0, 0);
            cirBuffer_put_IgnoreArg_me();
            cirBuffer_put_IgnoreArg_elem();
        }
    }

    for (j = 0; j < 4; ++j)
    {
        anSampler_put();
    }

    cirBuffer_getNumElem_ExpectAndReturn(0, 4);
    cirBuffer_getNumElem_IgnoreArg_me();

    nSamples = anSampler_getNumSamples();
    TEST_ASSERT_EQUAL(4, nSamples);
}

/* ------------------------------ End of file ------------------------------ */
