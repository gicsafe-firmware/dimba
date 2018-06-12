/**
 *  \file       dIn.c
 *  \brief      Implementation of Digital Inputs HAL and change detection.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.17  DaBa  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Baliña       db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"
#include "sapi.h"
#include "IOChgDet.h"
#include "din.h"
#include "mTimeCfg.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static bool_t dIns[NUM_DIN_SIGNALS];

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
void
dIn_init(void)
{
    int i;
    for(i=0; i < NUM_DIN_SIGNALS; ++i)
    {
        gpioConfig(DI0+i, GPIO_INPUT);
        dIns[i] = 0;
    }
}

void
dIn_scan(void)
{
    int i;
    bool_t din;

    for(i=0; i < NUM_DIN_SIGNALS; ++i)
    {
        din = gpioRead(DI0+i);
        if(dIns[i] != din)
        {
            IOChgDet_put(i, din);
            dIns[i] = din;
        }
    }
}

/* ------------------------------ End of file ------------------------------ */
