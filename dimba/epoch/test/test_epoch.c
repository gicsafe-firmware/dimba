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
 *  \file       test_Epoch.c
 *  \brief      Unit test for epoch time of Unix
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
#include "epoch.h"
#include "Mock_rtime.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static Time time;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
void
setUp(void)
{
    Mock_rtime_Init();
}

void
tearDown(void)
{
    Mock_rtime_Verify();
    Mock_rtime_Destroy();
}

void
test_ClearAfterInit(void)
{
    Epoch epoch;

    time.tm_sec = 0;
    time.tm_min = 0;
    time.tm_hour = 0;
    time.tm_mday = 1;
    time.tm_mon = 1;
    time.tm_year = 1970;
    time.tm_wday = 1;
    time.tm_isdst = 0;
    rtime_get_ExpectAndReturn(&time);

    epoch = epoch_init();
    TEST_ASSERT_EQUAL(0, epoch);
}

void
test_MakeTime(void)
{
    Epoch epoch;

    time.tm_sec = 0;
    time.tm_min = 0;
    time.tm_hour = 0;
    time.tm_mday = 18;
    time.tm_mon = 5;
    time.tm_year = 2018;
    time.tm_wday = 5;
    time.tm_isdst = 0;
    rtime_get_ExpectAndReturn(&time);

    epoch_updateNow();

    epoch = epoch_get();
    TEST_ASSERT_EQUAL(1526601600, epoch);
}

void
test_UpdateStepByStep(void)
{
    Epoch epoch, updatingEpoch;

    time.tm_sec = 0;
    time.tm_min = 0;
    time.tm_hour = 0;
    time.tm_mday = 1;
    time.tm_mon = 1;
    time.tm_year = 1970;
    time.tm_wday = 1;
    time.tm_isdst = 0;
    rtime_get_ExpectAndReturn(&time);
    epoch_updateNow();
    epoch = epoch_get();
    TEST_ASSERT_EQUAL(0, epoch);

    time.tm_sec = 0;
    time.tm_min = 0;
    time.tm_hour = 0;
    time.tm_mday = 18;
    time.tm_mon = 5;
    time.tm_year = 2018;
    time.tm_wday = 5;
    time.tm_isdst = 0;
    rtime_get_ExpectAndReturn(&time);
    updatingEpoch = epoch_updateByStep();
    TEST_ASSERT_EQUAL(0, updatingEpoch);

    updatingEpoch = epoch_updateByStep();
    TEST_ASSERT_EQUAL(489, updatingEpoch);

    updatingEpoch = epoch_updateByStep();
    TEST_ASSERT_EQUAL(17669, updatingEpoch);

    updatingEpoch = epoch_updateByStep();
    TEST_ASSERT_EQUAL(424056, updatingEpoch);

    updatingEpoch = epoch_updateByStep();
    TEST_ASSERT_EQUAL(25443360, updatingEpoch);

    updatingEpoch = epoch_updateByStep();
    TEST_ASSERT_EQUAL(1526601600, updatingEpoch);

    epoch = epoch_get();
    TEST_ASSERT_EQUAL(updatingEpoch, epoch);
}

void
test_ReceiveTimeZero(void)
{
    Epoch epoch;

    time.tm_sec = 0;
    time.tm_min = 0;
    time.tm_hour = 0;
    time.tm_mday = 0;
    time.tm_mon = 0;
    time.tm_year = 0;
    time.tm_wday = 0;
    time.tm_isdst = 0;
    rtime_get_ExpectAndReturn(&time);

    epoch = epoch_init();
    TEST_ASSERT_EQUAL(0, epoch);
}

/* ------------------------------ End of file ------------------------------ */
