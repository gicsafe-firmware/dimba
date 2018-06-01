/**
 *  \file       conmgr.h
 *  \brief      Specification of connection and protocol manager.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.15  DaBa  v1.0.01  Initial version
 *  2018.05.07  LeFr  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Darío Baliña      db@vortexmakes.com
 *  LeFr  Leandro Francucci lf@vortexmakes.com
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
#define MOVISTAR_APN_ADDR   "INTERNER.GPRS.UNIFON.COM.AR"  
#define MOVISTAR_APN_USER   "WAP"
#define MOVISTAR_APN_PASS   "WAP"

#define CLARO_APN_ADDR      "INTERNET.CTIMOVIL.COM.AR"
#define CLARO_APN_USER      "CLAROGPRS"
#define CLARO_APN_PASS      "CLAROGPRS999"

#define SYNC_DELAY_TIME     RKH_TIME_MS(1000)
#define MAX_SYNC_RETRY      5
#define SIM_PIN_NUMBER      9474

#define REGISTRATION_TIME   RKH_TIME_MS(20000)

#define MAX_CONFIG_RETRY    5
#define CONFIG_TRY_DELAY    RKH_TIME_MS(3000)

#define CONNECTION_APN      CLARO_APN_ADDR
#define CONNECTION_USER     CLARO_APN_USER
#define CONNECTION_PASSWORD CLARO_APN_PASS

#define CONNECTION_PROT     "TCP"

#define YIPIES_SERVER       "71.6.135.77"
#define YIPIES_PORT         "33499"

#define DOCKLIGHT_SERVER    "181.230.72.86"
#define DOCKLIGHT_PORT      "33498"

#define CONNECTION_DOMAIN   DOCKLIGHT_SERVER 
#define CONNECTION_PORT     DOCKLIGHT_PORT

#define READ_POLLING_TIME   RKH_TIME_MS(2000)

/* ................................ Signals ................................ */
/* ........................ Declares active object ......................... */
RKH_SMA_DCLR(conMgr);

/* ------------------------------- Data types ------------------------------ */
typedef struct SendEvt SendEvt;
struct SendEvt
{
    RKH_EVT_T evt;
    unsigned char *data;
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
