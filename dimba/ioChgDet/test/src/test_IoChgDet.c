/**
 *  \file       test_IoChgDet.c
 *  \brief      Unit test for IO change detector.
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
#include "IoChgDet.h"
#include "Mock_CirBuffer.h"
#include "Mock_epoch.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
int GlobalExpectCount;
int GlobalVerifyOrder;
char *GlobalOrderError;

TEST_GROUP(IoChgDet);

/* ---------------------------- Local variables ---------------------------- */
/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
TEST_SETUP(IoChgDet)
{
    Mock_CirBuffer_Init();
    Mock_epoch_Init();
}

TEST_TEAR_DOWN(IoChgDet)
{
    Mock_CirBuffer_Verify();
    Mock_epoch_Verify();
    Mock_CirBuffer_Destroy();
    Mock_epoch_Destroy();
}

TEST(IoChgDet, ClearAfterInit)
{
    int result;

    cirBuffer_init_ExpectAndReturn(0, 0, sizeof(IOChg), MAX_NUM_IO_CHANGES,
                                   0);
    cirBuffer_init_IgnoreArg_me();
    cirBuffer_init_IgnoreArg_sto();

    result = IOChgDet_init();
    TEST_ASSERT_EQUAL(0, result);
}

TEST(IoChgDet, StoreOneIOChange)
{
    int result;
    unsigned char signalId, signalValue;

    cirBuffer_init_ExpectAndReturn(0, 0, sizeof(IOChg), MAX_NUM_IO_CHANGES, 0);
    cirBuffer_init_IgnoreArg_me();
    cirBuffer_init_IgnoreArg_sto();
    epoch_get_ExpectAndReturn(123455);
    cirBuffer_put_ExpectAndReturn(0, 0, 0);
    cirBuffer_put_IgnoreArg_me();
    cirBuffer_put_IgnoreArg_elem();

    IOChgDet_init();
    signalId = 4;
    signalValue = 1;

    result = IOChgDet_put(signalId, signalValue);
    TEST_ASSERT_EQUAL(0, result);
}

TEST(IoChgDet, GetOneIOChange)
{
    int result;
    unsigned char signalId, signalValue;
    IOChg ioChanges[8];

    cirBuffer_init_ExpectAndReturn(0, 0, sizeof(IOChg), MAX_NUM_IO_CHANGES, 0);
    cirBuffer_init_IgnoreArg_me();
    cirBuffer_init_IgnoreArg_sto();
    epoch_get_ExpectAndReturn(123455);
    cirBuffer_put_ExpectAndReturn(0, 0, 0);
    cirBuffer_put_IgnoreArg_me();
    cirBuffer_put_IgnoreArg_elem();
    cirBuffer_getBlock_ExpectAndReturn(0, (unsigned char *)ioChanges, 1, 1);
    cirBuffer_getBlock_IgnoreArg_me();

    IOChgDet_init();
    signalId = 4;
    signalValue = 1;
    IOChgDet_put(signalId, signalValue);

    result = IOChgDet_get(ioChanges, 1);
    TEST_ASSERT_EQUAL(1, result);
}

/* ------------------------------ End of file ------------------------------ */
