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
#define DEBOUNCE_NSAMPLE    5
#define DEBOUNCE_MASK       ((uint8_t)((1 << DEBOUNCE_NSAMPLE) - 1))
#define DEBOUNCE_CHG        ((uint8_t)((1 << (DEBOUNCE_NSAMPLE - 1)) - 1))
#define DEBOUNCE_NOT_MASK   ((uint8_t) ~DEBOUNCE_MASK)
#define DEBOUNCE_NOT_CHG    ((uint8_t) ~DEBOUNCE_CHG)

/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static uint8_t dIns[NUM_DIN_SIGNALS];
static uint8_t dInsSt[NUM_DIN_SIGNALS];

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
void
dIn_init(void)
{
    int i;
    for (i = 0; i < NUM_DIN_SIGNALS; ++i)
    {
        gpioConfig(DI0 + i, GPIO_INPUT);
        dIns[i] = 0;
        dInsSt[i] = 0;
    }
}

void
dIn_scan(void)
{
    int i;
    uint8_t din;

    for (i = 0; i < NUM_DIN_SIGNALS; ++i)
    {
        din = (dIns[i] << 1) | gpioRead(DI0 + i);

        if ((dIns[i] == DEBOUNCE_CHG) && (din == DEBOUNCE_MASK) &&
            (dInsSt[i] == 0))
        {
            dInsSt[i] = 1;
            IOChgDet_put(i, 1);
        }
        else if ((dIns[i] == DEBOUNCE_NOT_CHG) && (din == DEBOUNCE_NOT_MASK) &&
                 (dInsSt[i] == 1))
        {
            dInsSt[i] = 0;
            IOChgDet_put(i, 0);
        }

        dIns[i] = din;
    }
}

/* ------------------------------ End of file ------------------------------ */
