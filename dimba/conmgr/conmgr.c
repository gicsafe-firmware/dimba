/**
 *  \file       conmgr.c
 *  \brief      Implementation of connection and protocol manager.
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
/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"
#include "conmgr.h"
#include "modmgr.h"
#include "modcmd.h"
#include "signals.h"
#include "bsp.h"

/* ----------------------------- Local macros ------------------------------ */
/* ......................... Declares active object ........................ */
typedef struct ConMgr ConMgr;

/* ................... Declares states and pseudostates .................... */
RKH_DCLR_BASIC_STATE ConMgr_inactive, ConMgr_sync, ConMgr_error,
                     ConMgr_pin, ConMgr_reg, ConMgr_configure, ConMgr_connect, 
                     ConMgr_unregistered;
RKH_DCLR_COMP_STATE ConMgr_active, ConMgr_initialize, ConMgr_registered;
RKH_DCLR_COND_STATE ConMgr_checkSyncTry;

/* ........................ Declares initial action ........................ */
static void init(ConMgr *const me, RKH_EVT_T *pe);

/* ........................ Declares effect actions ........................ */
static void open(ConMgr *const me, RKH_EVT_T *pe);
static void sendInit(ConMgr *const me, RKH_EVT_T *pe);
static void setPin(ConMgr *const me, RKH_EVT_T *pe);
static void enableUnsolicitedRegStatus(ConMgr *const me, RKH_EVT_T *pe);

/* ......................... Declares entry actions ........................ */
static void sendSync(ConMgr *const me, RKH_EVT_T *pe);
static void checkPin(ConMgr *const me, RKH_EVT_T *pe);
static void checkReg(ConMgr *const me, RKH_EVT_T *pe);

/* ......................... Declares exit actions ......................... */
/* ............................ Declares guards ............................ */
rbool_t checkSyncTry(ConMgr *const me, RKH_EVT_T *pe);

/* ........................ States and pseudostates ........................ */
RKH_CREATE_BASIC_STATE(ConMgr_inactive, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_inactive)
    RKH_TRREG(evOpen, NULL, open, &ConMgr_active),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(ConMgr_active, NULL, NULL, RKH_ROOT, 
                             &ConMgr_initialize, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_active)
    RKH_TRREG(evClose, NULL, NULL, &ConMgr_inactive),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(ConMgr_initialize, NULL, NULL, &ConMgr_active, 
                             &ConMgr_sync, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_initialize)
    RKH_TRREG(evReg, NULL, NULL,   &ConMgr_registered),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_sync, sendSync, NULL, &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_sync)
    RKH_TRREG(evSync, NULL, sendInit, &ConMgr_pin),
    RKH_TRREG(evNoResponse, NULL, NULL, &ConMgr_checkSyncTry),
RKH_END_TRANS_TABLE

RKH_CREATE_COND_STATE(ConMgr_checkSyncTry);
RKH_CREATE_BRANCH_TABLE(ConMgr_checkSyncTry)
    RKH_BRANCH(checkSyncTry,   NULL,   &ConMgr_sync),
    RKH_BRANCH(ELSE,           NULL,   &ConMgr_error),
RKH_END_BRANCH_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_error, NULL, NULL, &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_error)
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_pin, checkPin, NULL, &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_pin)
    RKH_TRREG(evSimReady, NULL, NULL,   &ConMgr_reg),
    RKH_TRREG(evSimPin,   NULL, setPin, &ConMgr_pin),
    RKH_TRREG(evSimError, NULL, NULL,   &ConMgr_error),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_reg, checkReg, NULL, &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_reg)
    RKH_TRREG(evNoReg, NULL, checkReg,   &ConMgr_reg),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(ConMgr_registered, NULL, NULL, &ConMgr_active, 
                             &ConMgr_configure, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_registered)
    RKH_TRREG(evNoReg, NULL, NULL,   &ConMgr_unregistered),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_configure, NULL, NULL, &ConMgr_registered, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_configure)
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_connect, NULL, NULL, &ConMgr_registered, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_connect)
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_unregistered, NULL, NULL, &ConMgr_active, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_unregistered)
    RKH_TRREG(evReg, NULL, NULL,   &ConMgr_registered),
RKH_END_TRANS_TABLE

/* ............................. Active object ............................. */
struct ConMgr
{
    RKH_SMA_T ao;       /* base structure */
    RKH_TMR_T timer;    /* which is responsible for toggling the LED */
                        /* posting the TIMEOUT signal event to active object */
                        /* 'conMgr' */
    rui8_t syncRetryCount; 
};

RKH_SMA_CREATE(ConMgr, conMgr, 1, HCAL, &ConMgr_inactive, init, NULL);
RKH_SMA_DEF_PTR(conMgr);

/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
/*
 *  Declare and allocate the 'e_tout' event.
 *  The 'e_tout' event with TIMEOUT signal never changes, so it can be
 *  statically allocated just once by means of RKH_ROM_STATIC_EVENT() macro.
 */
static RKH_ROM_STATIC_EVENT(e_tout, evToutDelay);

static RKH_ROM_STATIC_EVENT(e_Open, evOpen);

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ............................ Initial action ............................. */
static void
init(ConMgr *const me, RKH_EVT_T *pe)
{
	(void)pe;

    RKH_TR_FWK_AO(me);
    RKH_TR_FWK_QUEUE(&RKH_UPCAST(RKH_SMA_T, me)->equeue);
    RKH_TR_FWK_STATE(me, &ConMgr_inactive);
    RKH_TR_FWK_STATE(me, &ConMgr_active);
    RKH_TR_FWK_STATE(me, &ConMgr_initialize);
    RKH_TR_FWK_STATE(me, &ConMgr_sync);
    RKH_TR_FWK_STATE(me, &ConMgr_error);
    RKH_TR_FWK_STATE(me, &ConMgr_pin);
    RKH_TR_FWK_STATE(me, &ConMgr_reg);
    RKH_TR_FWK_STATE(me, &ConMgr_registered);
    RKH_TR_FWK_STATE(me, &ConMgr_unregistered);
    RKH_TR_FWK_STATE(me, &ConMgr_configure);
    RKH_TR_FWK_STATE(me, &ConMgr_connect);
    RKH_TR_FWK_TIMER(&me->timer);
    RKH_TR_FWK_SIG(evSync);
    RKH_TR_FWK_SIG(evOpen);
    RKH_TR_FWK_SIG(evClose);
    RKH_TR_FWK_SIG(evCmd);
    RKH_TR_FWK_SIG(evURC);
    RKH_TR_FWK_SIG(evSimPin);
    RKH_TR_FWK_SIG(evSimError);
    RKH_TR_FWK_SIG(evSimReady);
    RKH_TR_FWK_SIG(evReg);
    RKH_TR_FWK_SIG(evNoReg);
    RKH_TR_FWK_SIG(evTerminate);

    RKH_TMR_INIT(&me->timer, &e_tout, NULL);
    me->syncRetryCount = 0;
}

/* ............................ Effect actions ............................. */
static void
open(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    (void)me;

    RKH_SMA_POST_FIFO(modMgr, &e_Open, conMgr);
}

/* ............................. Entry actions ............................. */
static void
sendSync(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;

    ++me->syncRetryCount;

    ModCmd_sync();
}

static void
sendInit(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;

    ModCmd_initStr();
}

static void
checkPin(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;

    ModCmd_getPinStatus();
}

static void
setPin(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;

    ModCmd_setPin(SIM_PIN_NUMBER);
}

static void
checkReg(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;

    ModCmd_getRegStatus();
}

static void
enableUnsolicitedRegStatus(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;

    ModCmd_UnsolicitedRegStatus();
}

/* ............................. Exit actions .............................. */
/* ................................ Guards ................................. */
rbool_t
checkSyncTry(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    
    return (me->syncRetryCount < MAX_SYNC_RETRY) ? RKH_TRUE : RKH_FALSE;
}

/* ---------------------------- Global functions --------------------------- */
/* ------------------------------ End of file ------------------------------ */
