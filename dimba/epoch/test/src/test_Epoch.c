/**
 *  \file       test_Epoch.c
 *  \brief      Unit test for epoch time of Unix
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.18  LeFr  v1.0.00  ---
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci  lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "unity_fixture.h"
#include "epoch.h"
#include "Mock_rtime.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
int GlobalExpectCount;
int GlobalVerifyOrder;
char *GlobalOrderError;

TEST_GROUP(Epoch);

/* ---------------------------- Local variables ---------------------------- */
static Time time;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
TEST_SETUP(Epoch)
{
    Mock_rtime_Init();
}

TEST_TEAR_DOWN(Epoch)
{
    Mock_rtime_Verify();
    Mock_rtime_Destroy();
}

TEST(Epoch, ClearAfterInit)
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

TEST(Epoch, MakeTime)
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

TEST(Epoch, UpdateStepByStep)
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

/* ------------------------------ End of file ------------------------------ */
