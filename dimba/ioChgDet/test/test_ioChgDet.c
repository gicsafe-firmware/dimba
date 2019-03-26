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
 *  \file       test_IoChgDet.c
 *  \brief      Unit test for IO change detector.
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci  lf@vortexmakes.com
 *  BrPa  Bruno Palacios     BrunoPalacios@Embels.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "unity.h"
#include "ioChgDet.h"
#include "Mock_CirBuffer.h"
#include "Mock_epoch.h"

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
    int result;

    cirBuffer_init_ExpectAndReturn(0, 0, sizeof(IOChg), MAX_NUM_IO_CHANGES,
                                   0);
    cirBuffer_init_IgnoreArg_me();
    cirBuffer_init_IgnoreArg_sto();

    result = IOChgDet_init();
    TEST_ASSERT_EQUAL(0, result);
}

void
test_StoreOneIOChange(void)
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

void
test_GetOneIOChange(void)
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
