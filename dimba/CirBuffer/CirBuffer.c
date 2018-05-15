/**
 *  \file       CirBuffer.c
 *  \brief      Implementation of circular buffer.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.15  LeFr  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  2018.05.07  LeFr  v1.0.00  Initial version
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "CirBuffer.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
int 
cirBuffer_init(CirBuffer *const me, unsigned char *sto, int elemSize,
               int numElem)
{
    int result = 1;

    if ((me != (CirBuffer *)0) && (sto != (unsigned char *)0) &&
        (elemSize != 0) && (numElem != 0))
    {
        me->qty = 0;
        me->elemSize = elemSize;
        me->numElem = numElem;
        me->sto = me->in = me->out = sto;
        me->end = sto + (elemSize * numElem);
        result = 0;
    }
    return result;
}

int 
cirBuffer_put(CirBuffer *const me, unsigned char *elem)
{
    int result = 1;

    if ((me != (CirBuffer *)0) && (elem != (unsigned char *)0))
    {
        memcpy(me->in, elem, me->elemSize);
        ++me->qty;
        result = 0;
    }
    return result;
}

/* ------------------------------ End of file ------------------------------ */
