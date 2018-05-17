/**
 *  \file       test_AnSampler.c
 *  \brief      Unit test for analog signal sampler.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.17  LeFr  v1.0.00  ---
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci  lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "unity_fixture.h"
#include "AnSampler.h"
#include "Mock_CirBuffer.h"
#include "Mock_epoch.h"
#include "Mock_adconv.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
int GlobalExpectCount;
int GlobalVerifyOrder;
char *GlobalOrderError;

TEST_GROUP(AnSampler);

/* ---------------------------- Local variables ---------------------------- */
/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
TEST_SETUP(AnSampler)
{
    Mock_CirBuffer_Init();
    Mock_epoch_Init();
}

TEST_TEAR_DOWN(AnSampler)
{
    Mock_CirBuffer_Verify();
    Mock_epoch_Verify();
    Mock_CirBuffer_Destroy();
    Mock_epoch_Destroy();
}

TEST(AnSampler, ClearAfterInit)
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

TEST(AnSampler, FailsOneAnSamplerOnInit)
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

TEST(AnSampler, StoreOneAnSample)
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
    ADConv_getSample_ExpectAndReturn(0, 707);
    cirBuffer_put_ExpectAndReturn(0, 0, 0);
    cirBuffer_put_IgnoreArg_me();
    cirBuffer_put_IgnoreArg_elem();
    ADConv_getSample_ExpectAndReturn(1, 707);
    cirBuffer_put_ExpectAndReturn(0, 0, 0);
    cirBuffer_put_IgnoreArg_me();
    cirBuffer_put_IgnoreArg_elem();

    result = anSampler_put();
    TEST_ASSERT_EQUAL(0, result);
}

/* ------------------------------ End of file ------------------------------ */
