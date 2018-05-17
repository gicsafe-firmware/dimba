/**
 *  \file       test_IoChgDet_runner.c
 *  \brief      Test runner of IO change detector.
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

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
TEST_GROUP_RUNNER(IoChgDet)
{
	RUN_TEST_CASE(IoChgDet, ClearAfterInit);
	RUN_TEST_CASE(IoChgDet, StoreOneIOChange);
	RUN_TEST_CASE(IoChgDet, GetOneIOChange);
}

/* ------------------------------ End of file ------------------------------ */
