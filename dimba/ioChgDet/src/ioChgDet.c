/**
 *  \file       ioChgDet.c
 *  \brief      Implementation of IO change detector.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.17  LeFr  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include <string.h>
#include "IoChgDet.h"
#include "CirBuffer.h"
#include "epoch.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
typedef struct IOChgDet IOChgDet;
struct IOChgDet
{
    CirBuffer buffer;
    IOChg changes[MAX_NUM_IO_CHANGES];
};

/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static IOChgDet ioChgDet;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
int
IOChgDet_init(void)
{
    int result;

    /* enter_critical_section() */
    result = cirBuffer_init(&ioChgDet.buffer,
                            (unsigned char *)ioChgDet.changes,
                            sizeof(IOChg), MAX_NUM_IO_CHANGES);
    /* exit_critical_section() */
    return result;
}

int
IOChgDet_put(unsigned char signalId, unsigned char signalValue)
{
    int result;
    IOChg change;

    change.timeStamp = epoch_get();
    change.signalId = signalId;
    change.signalValue = signalValue;

    /* enter_critical_section() */
    result = cirBuffer_put(&ioChgDet.buffer, (unsigned char *)&change);
    /* exit_critical_section() */
    return result;
}

int
IOChgDet_get(IOChg *destBlock, int nChanges)
{
    int nGetChanges;

    /* enter_critical_section() */
    nGetChanges = cirBuffer_getBlock(&ioChgDet.buffer,
                                     (unsigned char *)destBlock, nChanges);
    /* exit_critical_section() */
    return nGetChanges;
}

/* ------------------------------ End of file ------------------------------ */
