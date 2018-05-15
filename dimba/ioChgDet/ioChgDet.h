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
#include "rkh.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
#define MAX_NUM_IO_CHG      256

/* ------------------------------- Data types ------------------------------ */
typedef struct IOChg IOChg;
struct IOChg
{
    EpochTime timeStamp;
    unsigned char ioSignal; /** IO signal identification */
};

typedef struct IOChgDet IOChgDet;
struct IOChgDet
{
    CirBuffer buffer;
    IOChg values[MAX_NUM_IO_CHG];
};

/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
