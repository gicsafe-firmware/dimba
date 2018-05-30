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
RKH_DCLR_BASIC_STATE ConMgr_inactive, ConMgr_sync, ConMgr_initError,
                ConMgr_init, ConMgr_pin, ConMgr_setPin, ConMgr_waitReg,
                    ConMgr_unregistered,
                ConMgr_setManualGet, ConMgr_setAPN, ConMgr_enableGPRS,
                ConMgr_checkIP, ConMgr_configureError, ConMgr_disconnected,
                ConMgr_idle, ConMgr_sending, ConMgr_receiving, 
                ConMgr_connectError;

RKH_DCLR_COMP_STATE ConMgr_active, ConMgr_initialize, ConMgr_registered,
                    ConMgr_configure, ConMgr_connect, ConMgr_connected;
RKH_DCLR_FINAL_STATE ConMgr_initializeFinal, ConMgr_configureFinal;
RKH_DCLR_COND_STATE ConMgr_checkSyncTry;

/* ........................ Declares initial action ........................ */
static void init(ConMgr *const me, RKH_EVT_T *pe);

/* ........................ Declares effect actions ........................ */
static void open(ConMgr *const me, RKH_EVT_T *pe);
static void enableUnsolicitedRegStatus(ConMgr *const me, RKH_EVT_T *pe);
static void requestIp(ConMgr *const me, RKH_EVT_T *pe);
static void socketOpen(ConMgr *const me, RKH_EVT_T *pe);
static void socketClose(ConMgr *const me, RKH_EVT_T *pe);
static void sendData(ConMgr *const me, RKH_EVT_T *pe);
static void readData(ConMgr *const me, RKH_EVT_T *pe);

/* ......................... Declares entry actions ........................ */
static void sendSync(ConMgr *const me, RKH_EVT_T *pe);
static void sendInit(ConMgr *const me, RKH_EVT_T *pe);
static void checkPin(ConMgr *const me, RKH_EVT_T *pe);
static void setPin(ConMgr *const me, RKH_EVT_T *pe);
static void checkReg(ConMgr *const me, RKH_EVT_T *pe);
static void setupManualGet(ConMgr *const me, RKH_EVT_T *pe);
static void setupAPN(ConMgr *const me, RKH_EVT_T *pe);
static void startGPRS(ConMgr *const me, RKH_EVT_T *pe);
static void getConnStatus(ConMgr *const me, RKH_EVT_T *pe);
static void startConnection(ConMgr *const me, RKH_EVT_T *pe);
static void startReceivePollingTimer(ConMgr *const me, RKH_EVT_T *pe);

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
    RKH_TRCOMPLETION(NULL, NULL, &ConMgr_registered),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_sync, sendSync, NULL, &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_sync)
    RKH_TRREG(evOk,         NULL, NULL, &ConMgr_init),
    RKH_TRREG(evNoResponse, NULL, NULL, &ConMgr_checkSyncTry),
RKH_END_TRANS_TABLE

RKH_CREATE_COND_STATE(ConMgr_checkSyncTry);
RKH_CREATE_BRANCH_TABLE(ConMgr_checkSyncTry)
    RKH_BRANCH(checkSyncTry,   NULL,   &ConMgr_sync),
    RKH_BRANCH(ELSE,           NULL,   &ConMgr_initError),
RKH_END_BRANCH_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_initError, NULL, NULL, &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_initError)
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_init, sendInit, NULL, &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_init)
    RKH_TRREG(evOk,         NULL, NULL, &ConMgr_pin),
    RKH_TRREG(evNoResponse, NULL, NULL, &ConMgr_initError),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_pin, checkPin, NULL, &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_pin)
    RKH_TRREG(evSimPin,     NULL, NULL, &ConMgr_setPin),
    RKH_TRREG(evSimError,   NULL, NULL, &ConMgr_initError),
    RKH_TRREG(evSimReady,   NULL, NULL, &ConMgr_waitReg),
    RKH_TRREG(evNoResponse, NULL, NULL, &ConMgr_initError),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_setPin, setPin, NULL, &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_setPin)
    RKH_TRREG(evOk,         NULL, NULL,   &ConMgr_pin),
    RKH_TRREG(evNoResponse, NULL, NULL,   &ConMgr_initError),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_waitReg, checkReg, NULL, 
                                                    &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_waitReg)
    RKH_TRREG(evReg,        NULL, NULL,   &ConMgr_initializeFinal),
    RKH_TRREG(evNoResponse, NULL, NULL,   &ConMgr_initError),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(ConMgr_registered, NULL, NULL, &ConMgr_active, 
                             &ConMgr_configure, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_registered)
    RKH_TRREG(evNoReg, NULL, NULL,   &ConMgr_unregistered),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_unregistered, NULL, NULL, &ConMgr_active, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_unregistered)
    RKH_TRREG(evReg, NULL, NULL,   &ConMgr_registered),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(ConMgr_configure, NULL, NULL, &ConMgr_registered, 
                             &ConMgr_setManualGet, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_configure)
    RKH_TRCOMPLETION(NULL, NULL, &ConMgr_connect),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_setManualGet, setupManualGet, NULL, 
                                                    &ConMgr_configure, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_setManualGet)
    RKH_TRREG(evOk,         NULL, NULL, &ConMgr_setAPN),
    RKH_TRREG(evNoResponse, NULL, NULL, &ConMgr_configureError),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_setAPN, setupAPN, NULL, 
                                                    &ConMgr_configure, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_setAPN)
    RKH_TRREG(evOk,         NULL, NULL, &ConMgr_enableGPRS),
    RKH_TRREG(evNoResponse, NULL, NULL, &ConMgr_configureError),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_enableGPRS, startGPRS, NULL, 
                                                    &ConMgr_configure, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_enableGPRS)
    RKH_TRREG(evOk,         NULL, NULL, &ConMgr_checkIP),
    RKH_TRREG(evNoResponse, NULL, NULL, &ConMgr_configureError),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_checkIP, getConnStatus, NULL, 
                                                    &ConMgr_configure, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_checkIP)
    RKH_TRREG(evIPGprsAct,  requestIp,    NULL, &ConMgr_checkIP),
    RKH_TRREG(evIP,         NULL,         NULL, &ConMgr_checkIP),
    RKH_TRREG(evIPInitial,  NULL,         NULL, &ConMgr_checkIP),
    RKH_TRREG(evIPStart,    NULL,         NULL, &ConMgr_checkIP),
    RKH_TRREG(evIPStatus,   NULL,         NULL, &ConMgr_configureFinal),
    RKH_TRREG(evNoResponse, NULL,         NULL, &ConMgr_configureError),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_configureError, NULL, NULL,
                                                    &ConMgr_configure, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_configureError)
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(ConMgr_connect, startConnection, NULL, 
                             &ConMgr_registered, &ConMgr_disconnected, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_connect)
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_disconnected, NULL, NULL, 
                                                    &ConMgr_connect, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_disconnected)
    RKH_TRINT(evOpen, NULL,  &socketOpen),
    RKH_TRREG(evConnected, NULL,  NULL, &ConMgr_connected),
    RKH_TRREG(evNoResponse, NULL, NULL, &ConMgr_connectError),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(ConMgr_connected, NULL, NULL, 
                             &ConMgr_connect, &ConMgr_idle, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_connected)
    RKH_TRINT(evClose, NULL,  &socketClose),
    RKH_TRREG(evDisconnected, NULL,  NULL, &ConMgr_disconnected),
    RKH_TRREG(evNoResponse, NULL, NULL, &ConMgr_connectError),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_idle, startReceivePollingTimer, 
                                            NULL, &ConMgr_connected, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_idle)
    RKH_TRREG(evSend,               NULL,  NULL, &ConMgr_sending),
    RKH_TRREG(evReceivePollingTout, NULL, readData, &ConMgr_receiving),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_sending, NULL, NULL, &ConMgr_connected, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_sending)
    RKH_TRREG(evOk, NULL,  NULL, &ConMgr_idle),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_receiving, NULL, NULL, &ConMgr_connected, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_receiving)
    RKH_TRREG(evOk, NULL,  NULL, &ConMgr_idle),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_connectError, NULL, NULL, &ConMgr_connect, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_connectError)
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
	RKH_TR_FWK_STATE(me, &ConMgr_init);
    RKH_TR_FWK_STATE(me, &ConMgr_initError);
    RKH_TR_FWK_STATE(me, &ConMgr_pin);
    RKH_TR_FWK_STATE(me, &ConMgr_setPin);
    RKH_TR_FWK_STATE(me, &ConMgr_waitReg);
    RKH_TR_FWK_STATE(me, &ConMgr_initializeFinal);
    RKH_TR_FWK_STATE(me, &ConMgr_registered);
    RKH_TR_FWK_STATE(me, &ConMgr_unregistered);
    RKH_TR_FWK_STATE(me, &ConMgr_configure);
    RKH_TR_FWK_STATE(me, &ConMgr_setManualGet);
    RKH_TR_FWK_STATE(me, &ConMgr_setAPN);
    RKH_TR_FWK_STATE(me, &ConMgr_enableGPRS);
    RKH_TR_FWK_STATE(me, &ConMgr_checkIP);
    RKH_TR_FWK_STATE(me, &ConMgr_configureError);
    RKH_TR_FWK_STATE(me, &ConMgr_configureFinal);
    RKH_TR_FWK_STATE(me, &ConMgr_connect);
    RKH_TR_FWK_STATE(me, &ConMgr_disconnected);
    RKH_TR_FWK_STATE(me, &ConMgr_connected);
    RKH_TR_FWK_STATE(me, &ConMgr_idle);
    RKH_TR_FWK_STATE(me, &ConMgr_sending);
    RKH_TR_FWK_STATE(me, &ConMgr_receiving);
    RKH_TR_FWK_STATE(me, &ConMgr_connectError);
    RKH_TR_FWK_TIMER(&me->timer);
    RKH_TR_FWK_SIG(evOpen);
    RKH_TR_FWK_SIG(evClose);
    RKH_TR_FWK_SIG(evCmd);
	RKH_TR_FWK_SIG(evOk);
	RKH_TR_FWK_SIG(evURC);
    RKH_TR_FWK_SIG(evSimPin);
    RKH_TR_FWK_SIG(evSimError);
    RKH_TR_FWK_SIG(evSimReady);
    RKH_TR_FWK_SIG(evReg);
    RKH_TR_FWK_SIG(evNoReg);
    RKH_TR_FWK_SIG(evIPInitial);
    RKH_TR_FWK_SIG(evIPStart);
    RKH_TR_FWK_SIG(evIPGprsAct);
    RKH_TR_FWK_SIG(evConnected);
    RKH_TR_FWK_SIG(evSend);
    RKH_TR_FWK_SIG(evReceivePollingTout);
    RKH_TR_FWK_SIG(evDisconnected);
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

void 
requestIp(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    (void)me;

    ModCmd_requestIP();
}

void 
sendData(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    (void)me;

    ModCmd_sendData(CONMGR_TEST_TX_PACKET);
}

void 
readData(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    (void)me;

    ModCmd_readData();
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
setupManualGet(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;

    ModCmd_setManualGet();
}
   
static void
setupAPN(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;

    ModCmd_setupAPN(CONNECTION_APN, CONNECTION_USER, CONNECTION_PASSWORD);
}
   
static void
startGPRS(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;

    ModCmd_startGPRS();
}

static void
getConnStatus(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;

    ModCmd_getConnStatus();
}

static void
startConnection(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;

    socketOpen(me, pe);
}

static void
socketOpen(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;

    ModCmd_connect(CONNECTION_PROT, CONNECTION_DOMAIN, CONNECTION_PORT);
}

static void
socketClose(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;

    ModCmd_disconnect();
}


static void
startReceivePollingTimer(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;

    ModCmd_disconnect();

    RKH_SET_STATIC_EVENT(&e_tout, evReceivePollingTout);
    RKH_TMR_ONESHOT(&me->timer, RKH_UPCAST(RKH_SMA_T, me),
                                                CONMGR_TEST_RX_POLLING);
}


/* ................................ Guards ................................. */
rbool_t
checkSyncTry(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    
    return (me->syncRetryCount < MAX_SYNC_RETRY) ? RKH_TRUE : RKH_FALSE;
}

/* ---------------------------- Global functions --------------------------- */
/* ------------------------------ End of file ------------------------------ */
