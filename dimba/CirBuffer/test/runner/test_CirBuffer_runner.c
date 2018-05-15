/**
 *  \file       test_cirBuffer_runner.c
 *  \brief      Test runner of circular buffer
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.15  LeFr  v1.0.00  ---
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
TEST_GROUP_RUNNER(cirBuffer)
{
	RUN_TEST_CASE(cirBuffer, ClearAfterInit);
	RUN_TEST_CASE(cirBuffer, WrongArgsOnInit);
	RUN_TEST_CASE(cirBuffer, PutOneElem);
}

/* ------------------------------ End of file ------------------------------ */
