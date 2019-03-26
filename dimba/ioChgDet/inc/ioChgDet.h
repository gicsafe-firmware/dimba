/**
 *  \file       ioChgDet.h
 *  \brief      Specification of I/O change detector
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.14  LeFr  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* --------------------------------- Module -------------------------------- */
#ifndef __IOCHGDET_H__
#define __IOCHGDET_H__

/* ----------------------------- Include files ----------------------------- */
#include "epoch.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
#define MAX_NUM_IO_CHANGES      256

/* ------------------------------- Data types ------------------------------ */
typedef struct IOChg IOChg;
struct IOChg
{
    Epoch timeStamp;
    unsigned char signalId;
    unsigned char signalValue;
};

/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
int IOChgDet_init(void);
int IOChgDet_put(unsigned char signalId, unsigned char siganlValue);
int IOChgDet_get(IOChg *destBlock, int nChanges);

/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
