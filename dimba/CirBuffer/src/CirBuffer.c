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
 *  \file       CirBuffer.c
 *  \brief      Implementation of circular buffer.
 */

/* -------------------------- Development history -------------------------- */
/*
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 *  CaMa   Carlos Mancón manconci@gmail.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include <string.h>
#include "CirBuffer.h"
#include "rkh.h"

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
               int nElem)
{
    int result = 1;

    if ((me != (CirBuffer *)0) && (sto != (unsigned char *)0) &&
        (elemSize != 0) && (nElem != 0))
    {
        me->qty = 0;
        me->elemSize = elemSize;
        me->numElem = nElem;
        me->sto = me->in = me->out = sto;
        me->end = sto + (elemSize * nElem);
        result = 0;
    }
    return result;
}

int
cirBuffer_put(CirBuffer *const me, unsigned char *elem)
{
    int result = 1;
    RKH_SR_ALLOC();

    RKH_ENTER_CRITICAL_();
    if ((me != (CirBuffer *)0) && (elem != (unsigned char *)0))
    {
        memcpy(me->in, elem, me->elemSize);
        me->in += me->elemSize;
        ++me->qty;
        if (me->in == me->end)
        {
            me->in = me->sto;
        }

        if (me->qty >= me->numElem)
        {
            me->qty = me->numElem;
            me->out = me->in;
        }
        result = 0;
    }
    RKH_EXIT_CRITICAL_();

    return result;
}

int
cirBuffer_get(CirBuffer *const me, unsigned char *elem)
{
    int result = 1;
    RKH_SR_ALLOC();

    RKH_ENTER_CRITICAL_();
    if ((me != (CirBuffer *)0) && (elem != (unsigned char *)0))
    {
        if (me->qty != 0)
        {
            memcpy(elem, me->out, me->elemSize);
            me->out += me->elemSize;
            --me->qty;
            if (me->out >= me->end)
            {
                me->out = me->sto;
            }
            result = 0;
        }
    }
    RKH_EXIT_CRITICAL_();

    return result;
}

int
cirBuffer_getBlock(CirBuffer *const me, unsigned char *destBlock,
                   int nElem)
{
    int n, nConsumed, result = 0, offset = 0;
    RKH_SR_ALLOC();

    RKH_ENTER_CRITICAL_();
    if ((me != (CirBuffer *)0) && (destBlock != (unsigned char *)0) &&
        (nElem != 0))
    {
        if (me->qty != 0)
        {
            nConsumed = (nElem >= me->qty) ? me->qty : nElem;
            n = (me->end - me->out) / me->elemSize;
            if (n > me->qty)
            {
                n = me->qty;
            }
            if (n > nElem)
            {
                n = nElem;
            }

            do
            {
                memcpy(destBlock + offset, me->out, me->elemSize * n);
                me->out += offset = (n * me->elemSize);
                me->qty -= n;
                result += n;

                if (me->out >= me->end)
                {
                    me->out = me->sto;
                }
                nConsumed -= n;
                n = nConsumed;
            }
            while (nConsumed);
        }
    }
    RKH_EXIT_CRITICAL_();

    return result;
}

int
cirBuffer_getNumElem(CirBuffer *const me)
{
    int n;
    RKH_SR_ALLOC();

    RKH_ENTER_CRITICAL_();
    n = me->qty;
    RKH_EXIT_CRITICAL_();
    return n;
}

/* ------------------------------ End of file ------------------------------ */
