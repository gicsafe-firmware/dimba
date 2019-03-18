/**
 *  \file       ethMgr.h
 *  \brief      Specification of connection and protocol manager.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2019.03.18  DaBa  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Bali#a      db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* --------------------------------- Module -------------------------------- */
#ifndef __ETHMGR_H__
#define __ETHMGR_H__

/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */

/* ................................. Server ................................ */
#define MOSQUITTO_SERVER        "test.mosquitto.org"
#define MOSQUITTO_PORT          "1883"

#define LINSSE_SERVER           "linsse.com.ar"
#define LINSSE_PORT             "2000"

#define AZURE_SERVER            "191.239.243.244"
#define AZURE_PORT              "1883"

#define DOCKLIGHT_SERVER        "181.230.72.86"
#define DOCKLIGHT_PORT          "33498"

#define CONNECTION_PROT         "TCP"
#define CONNECTION_DOMAIN       AZURE_SERVER
#define CONNECTION_PORT         AZURE_PORT

/* ................................ Signals ................................ */
/* ........................ Declares active object ......................... */
RKH_SMA_DCLR(ethMgr);

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
