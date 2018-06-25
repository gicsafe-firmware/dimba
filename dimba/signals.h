/**
 *  \file       signals.h
 *  \brief      Event signal definitions.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.02  DaBa  v1.0.00  Initial version
 *  2018.05.11  LeFr  v1.0.01
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Bali�a db@vortexmakes.com
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* --------------------------------- Module -------------------------------- */
#ifndef __SIGNALS_H__
#define __SIGNALS_H__

/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
/* ................................ Signals ................................ */
typedef enum Signals Signals;
enum Signals
{
	evOpen,
	evClose,
    evCmd,
    evResponse,
    evNoResponse,
    evURC,
	evTimeout,
    evToutWaitResponse,
    evOk,
    evError,
    evToutDelay,
    evSimReady,
    evSimPin,
    evSimError,
    evNoReg,
    evReg,
    evIP,
    evIPInitial,
    evIPStart,
    evIPStatus,
    evIPGprsAct,
    evConnecting,
    evClosed,
    evConnected,
    evSend,
    evSendFail,
    evSent,
    evRecv,
    evRecvFail,
    evReceived,
    evNetConnected,
    evNetDisconnected,
    evDisconnected,
	evTerminate,     /* press the key escape on the keyboard */
	evConnRefused,
	evWaitConnectTout,
	evWaitPublishTout,
	evWaitSyncTout,
    evConnAccepted,
    evActivate,
    evDeactivate,
    evSyncTout,
    evNetClockSync,
    evLocalTime,
    evUnlocked,
    evImei,
	evRestart
};

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
