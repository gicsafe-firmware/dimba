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
 *  \file       CirBuffer.h
 *  \brief      Specification of circular buffer.
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
/* --------------------------------- Module -------------------------------- */
#ifndef __CIRBUFFER_H__
#define __CIRBUFFER_H__

/* ----------------------------- Include files ----------------------------- */
/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
/* ------------------------------- Data types ------------------------------ */
typedef struct CirBuffer CirBuffer;
struct CirBuffer
{
    unsigned char *sto;
    unsigned char *end;
    unsigned char *out;
    unsigned char *in;
    int qty;
    int elemSize;
    int numElem;
};

/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
int cirBuffer_init(CirBuffer *const me, unsigned char *sto, int elemSize,
                   int nElem);
int cirBuffer_put(CirBuffer *const me, unsigned char *elem);
int cirBuffer_get(CirBuffer *const me, unsigned char *elem);
int cirBuffer_getBlock(CirBuffer *const me, unsigned char *destBlock,
                       int nElem);
int cirBuffer_getNumElem(CirBuffer *const me);

/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
