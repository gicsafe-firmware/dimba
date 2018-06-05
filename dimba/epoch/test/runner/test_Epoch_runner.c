/**
 *  \file       test_Epoch_runner.c
 *  \brief      Test runner of epoch time of Unix.
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

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
TEST_GROUP_RUNNER(Epoch)
{
	RUN_TEST_CASE(Epoch, ClearAfterInit);
	RUN_TEST_CASE(Epoch, MakeTime);
	RUN_TEST_CASE(Epoch, UpdateStepByStep);
}

/* ------------------------------ End of file ------------------------------ */
