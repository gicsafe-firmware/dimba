/**
 *  \file       all_tests.c
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
#include <stdio.h>
#include "unity_fixture.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
static void 
runAllTests(void)
{
	RUN_TEST_GROUP(Epoch);
}

/* ---------------------------- Global functions --------------------------- */
int
main(int argc, char *argv[])
{
    static char *args[8];
    int nArgs;

    args[0] = argv[0];
#if 0
    args[1] = "-g";
    args[2] = "trace_args";
    nArgs = 3;
#else
    nArgs = 1;
#endif

	UnityMain(nArgs, args, runAllTests);
    getchar();
}

/* ------------------------------ End of file ------------------------------ */
