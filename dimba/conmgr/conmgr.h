/**
 *  \file       conmgr.h
 *  \brief      Specification of connection and protocol manager.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.07  LeFr  v1.0.00  Initial version
 *  2018.05.15  DaBa  v1.0.01  Efects implemented
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 *  DaBa  Darío Baliña      db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* --------------------------------- Module -------------------------------- */
#ifndef __CONMGR_H__
#define __CONMGR_H__

/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
#define SYNC_DELAY_TIME     RKH_TIME_MS(1000)
#define MAX_SYNC_RETRY      5
#define SIM_PIN_NUMBER      9474
#define CONNECTION_APN      "INTERNER.GPRS.UNIFON.COM.AR"
#define CONNECTION_USER     "WAP"
#define CONNECTION_PASSWORD "WAP"
#define CONNECTION_PROT     "TCP"
#define CONNECTION_DOMAIN   "71.6.135.77"
#define CONNECTION_PORT     "33499"

/* ................................ Signals ................................ */
/* ........................ Declares active object ......................... */
RKH_SMA_DCLR(conMgr);

/* ------------------------------- Data types ------------------------------ */
/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
