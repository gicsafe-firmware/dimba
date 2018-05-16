/**
 *  \file       CirBuffer.h
 *  \brief      Specification of circular buffer.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.15  LeFr  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci lf@vortexmakes.com
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
                   int numElem);
int cirBuffer_put(CirBuffer *const me, unsigned char *elem);
int cirBuffer_get(CirBuffer *const me, unsigned char *elem);

/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
