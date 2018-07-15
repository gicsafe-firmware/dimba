/**
 *  \file       conmgr.c
 *  \brief      Implementation of connection and protocol manager.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.15  DaBa  v1.0.01  Initial version
 *  2018.05.07  LeFr  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Bali�a      db@vortexmakes.com
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"
#include <string.h>
#include "conmgr.h"
#include "modpwr.h"
#include "modmgr.h"
#include "modcmd.h"
#include "mqttProt.h"
#include "dimbaCfg.h"
#include "signals.h"
#include "rtime.h"
#include "bsp.h"

/* ----------------------------- Local macros ------------------------------ */
#define SIZEOF_QDEFER   1

/* ......................... Declares active object ........................ */
typedef struct ConMgr ConMgr;

/* ................... Declares states and pseudostates .................... */
RKH_DCLR_BASIC_STATE ConMgr_inactive, ConMgr_sync,
                ConMgr_init, ConMgr_pin, ConMgr_setPin, ConMgr_enableNetTime,
                ConMgr_getImei, ConMgr_cipShutdown, ConMgr_setManualGet,
                ConMgr_waitReg, ConMgr_unregistered, ConMgr_failure,
                ConMgr_waitNetClockSync, ConMgr_localTime, ConMgr_getOper,
                ConMgr_setAPN, ConMgr_enableGPRS,
                ConMgr_checkIP, ConMgr_waitRetryConfig, ConMgr_waitingServer,
                ConMgr_idle, ConMgr_getStatus, ConMgr_waitPrompt, ConMgr_waitOk,
                ConMgr_receiving, ConMgr_restarting, ConMgr_wReopen,
                ConMgr_waitRetryConnect, ConMgr_disconnecting;

RKH_DCLR_COMP_STATE ConMgr_active, ConMgr_initialize, ConMgr_registered,
                    ConMgr_configure, ConMgr_connecting, ConMgr_connected,
                    ConMgr_sending;
RKH_DCLR_FINAL_STATE ConMgr_activeFinal, ConMgr_initializeFinal, 
                     ConMgr_configureFinal, ConMgr_sendingFinal;
RKH_DCLR_COND_STATE ConMgr_checkSyncTry, ConMgr_checkConfigTry,
                    ConMgr_checkConnectTry;

/* ........................ Declares initial action ........................ */
static void init(ConMgr *const me, RKH_EVT_T *pe);

/* ........................ Declares effect actions ........................ */
static void open(ConMgr *const me, RKH_EVT_T *pe);
static void close(ConMgr *const me, RKH_EVT_T *pe);
static void defer(ConMgr *const me, RKH_EVT_T *pe);
static void setSigLevel(ConMgr *const me, RKH_EVT_T *pe);
static void initializeInit(ConMgr *const me, RKH_EVT_T *pe);
static void storeImei(ConMgr *const me, RKH_EVT_T *pe);
static void storeOper(ConMgr *const me, RKH_EVT_T *pe);
static void checkRegStatus(ConMgr *const me, RKH_EVT_T *pe);
static void startRegStatus(ConMgr *const me, RKH_EVT_T *pe);
static void localTimeGet(ConMgr *const me, RKH_EVT_T *pe);
static void rtimeSync(ConMgr *const me, RKH_EVT_T *pe);
static void configureInit(ConMgr *const me, RKH_EVT_T *pe);
static void configTry(ConMgr *const me, RKH_EVT_T *pe);
static void requestIp(ConMgr *const me, RKH_EVT_T *pe);
static void connectInit(ConMgr *const me, RKH_EVT_T *pe);
static void connectTry(ConMgr *const me, RKH_EVT_T *pe);
static void socketOpen(ConMgr *const me, RKH_EVT_T *pe);
static void socketClose(ConMgr *const me, RKH_EVT_T *pe);
static void readData(ConMgr *const me, RKH_EVT_T *pe);
static void sendRequest(ConMgr *const me, RKH_EVT_T *pe);
static void flushData(ConMgr *const me, RKH_EVT_T *pe);
static void sendOk(ConMgr *const me, RKH_EVT_T *pe);
static void recvOk(ConMgr *const me, RKH_EVT_T *pe);
static void sendFail(ConMgr *const me, RKH_EVT_T *pe);
static void recvFail(ConMgr *const me, RKH_EVT_T *pe);
static void tryGetStatus(ConMgr *const me, RKH_EVT_T *pe);

/* ......................... Declares entry actions ........................ */
static void sendSync(ConMgr *const me);
static void sendInit(ConMgr *const me);
static void checkPin(ConMgr *const me);
static void setPin(ConMgr *const me);
static void netTimeEnable(ConMgr *const me);
static void getImei(ConMgr *const me);
static void cipShutdown(ConMgr *const me);
static void unregEntry(ConMgr *const me);
static void regEntry(ConMgr *const me);
static void failureEntry(ConMgr *const me);
static void setupManualGet(ConMgr *const me);
static void waitNetClockSyncEntry(ConMgr *const me);
static void waitRetryConfigEntry(ConMgr *const me);
static void getOper(ConMgr *const me);
static void setupAPN(ConMgr *const me);
static void startGPRS(ConMgr *const me);
static void wReopenEntry(ConMgr *const me);
static void waitRetryConnEntry(ConMgr *const me);
static void getConnStatus(ConMgr *const me);
static void isConnected(ConMgr *const me);
static void connectingEntry(ConMgr *const me);
static void socketConnected(ConMgr *const me);
static void idleEntry(ConMgr *const me);

/* ......................... Declares exit actions ......................... */
static void unregExit(ConMgr *const me);
static void regExit(ConMgr *const me);
static void waitNetClockSyncExit(ConMgr *const me);
static void wReopenExit(ConMgr *const me);
static void waitRetryConnExit(ConMgr *const me);
static void failureExit(ConMgr *const me);
static void connectingExit(ConMgr *const me);
static void socketDisconnected(ConMgr *const me);
static void idleExit(ConMgr *const me);
static void getStatusExit(ConMgr *const me);

/* ............................ Declares guards ............................ */
rbool_t checkSyncTry(ConMgr *const me, RKH_EVT_T *pe);
rbool_t checkConfigTry(ConMgr *const me, RKH_EVT_T *pe);
rbool_t checkConnectTry(ConMgr *const me, RKH_EVT_T *pe);
rbool_t checkConnectedFailCounter(ConMgr *const me, RKH_EVT_T *pe);


/* ........................ States and pseudostates ........................ */
RKH_CREATE_BASIC_STATE(ConMgr_inactive, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_inactive)
    RKH_TRINT(evSend, NULL, sendFail),
    RKH_TRINT(evRecv, NULL, recvFail),
    RKH_TRREG(evOpen, NULL, open, &ConMgr_active),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(ConMgr_active, NULL, NULL, RKH_ROOT, 
                             &ConMgr_initialize, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_active)
    RKH_TRINT(evSigLevel, NULL, setSigLevel),
    RKH_TRINT(evSend, NULL, sendFail),
    RKH_TRINT(evRecv, NULL, recvFail),
    RKH_TRCOMPLETION(NULL, NULL, &ConMgr_inactive),
    RKH_TRREG(evClose, NULL, close, &ConMgr_inactive),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(ConMgr_initialize, NULL, NULL, &ConMgr_active, 
                             &ConMgr_sync, initializeInit,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_initialize)
    RKH_TRCOMPLETION(NULL, NULL, &ConMgr_unregistered),
    RKH_TRREG(evNoResponse, NULL, NULL, &ConMgr_failure),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_sync, sendSync, NULL, &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_sync)
    RKH_TRREG(evOk,         NULL, NULL, &ConMgr_init),
    RKH_TRREG(evNoResponse, NULL, NULL, &ConMgr_checkSyncTry),
RKH_END_TRANS_TABLE

RKH_CREATE_COND_STATE(ConMgr_checkSyncTry);
RKH_CREATE_BRANCH_TABLE(ConMgr_checkSyncTry)
    RKH_BRANCH(checkSyncTry,   NULL,   &ConMgr_sync),
    RKH_BRANCH(ELSE,           NULL,   &ConMgr_failure),
RKH_END_BRANCH_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_init, sendInit, NULL, &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_init)
    RKH_TRREG(evOk,         NULL, NULL, &ConMgr_pin),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_pin, checkPin, NULL, &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_pin)
    RKH_TRREG(evSimPin,     NULL, NULL, &ConMgr_setPin),
    RKH_TRREG(evSimError,   NULL, NULL, &ConMgr_failure),
    RKH_TRREG(evSimReady,   NULL, NULL, &ConMgr_enableNetTime),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_setPin, setPin, NULL, &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_setPin)
    RKH_TRREG(evOk,         NULL, NULL,   &ConMgr_pin),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_enableNetTime, netTimeEnable, NULL, 
                                                    &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_enableNetTime)
    RKH_TRREG(evOk,         NULL, NULL, &ConMgr_getImei),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_getImei, getImei, NULL, &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_getImei)
    RKH_TRREG(evImei,       NULL, storeImei, &ConMgr_cipShutdown),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_cipShutdown, cipShutdown, NULL, &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_cipShutdown)
    RKH_TRREG(evOk,         NULL, NULL, &ConMgr_setManualGet),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_setManualGet, setupManualGet, NULL, 
                                                    &ConMgr_initialize, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_setManualGet)
    RKH_TRREG(evOk,         NULL, NULL, &ConMgr_initializeFinal),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(ConMgr_registered, regEntry, regExit, &ConMgr_active, 
                             &ConMgr_waitNetClockSync, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_registered)
    RKH_TRREG(evNoReg, NULL, NULL,   &ConMgr_unregistered),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_unregistered, unregEntry, unregExit,
                            &ConMgr_active, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_unregistered)
    RKH_TRINT(evTimeout,    NULL,    checkRegStatus),
    RKH_TRINT(evNoReg,      NULL,    startRegStatus),
    RKH_TRREG(evRegTimeout,  NULL,    NULL, &ConMgr_failure),
    RKH_TRREG(evReg, NULL, NULL,   &ConMgr_registered),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_failure, failureEntry, failureExit, &ConMgr_active, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_failure)
    RKH_TRREG(evTimeout, NULL,  NULL, &ConMgr_active),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_waitNetClockSync, 
                            waitNetClockSyncEntry, waitNetClockSyncExit,
                            &ConMgr_registered, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_waitNetClockSync)
    RKH_TRREG(evTimeout,       NULL, NULL, &ConMgr_getOper),
    RKH_TRREG(evNetClockSync,  NULL, localTimeGet, &ConMgr_localTime),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_localTime, NULL, NULL, &ConMgr_registered, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_localTime)
    RKH_TRREG(evLocalTime,     NULL, rtimeSync,  &ConMgr_getOper),
    RKH_TRREG(evNoResponse,    NULL, NULL,       &ConMgr_getOper),
RKH_END_TRANS_TABLE

RKH_CREATE_HISTORY_STORAGE(ConMgr_configure);
RKH_CREATE_COMP_REGION_STATE(ConMgr_configure, NULL, NULL, &ConMgr_registered, 
                             &ConMgr_getOper, configureInit,
                             RKH_SHISTORY, NULL, NULL, NULL,
                             RKH_GET_HISTORY_STORAGE(ConMgr_configure));
RKH_CREATE_TRANS_TABLE(ConMgr_configure)
    RKH_TRCOMPLETION(NULL, connectInit, &ConMgr_connecting),
    RKH_TRREG(evNoResponse, NULL, NULL, &ConMgr_checkConfigTry),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_getOper, getOper, NULL, &ConMgr_configure, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_getOper)
    RKH_TRREG(evOper,          NULL, storeOper,     &ConMgr_setAPN),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_setAPN, setupAPN, NULL, 
                                                    &ConMgr_configure, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_setAPN)
    RKH_TRREG(evOk,         NULL, NULL, &ConMgr_enableGPRS),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_enableGPRS, startGPRS, NULL, 
                                                    &ConMgr_configure, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_enableGPRS)
    RKH_TRREG(evOk,         NULL, NULL, &ConMgr_checkIP),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_checkIP, getConnStatus, NULL, 
                                                    &ConMgr_configure, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_checkIP)
    RKH_TRREG(evIPGprsAct,  requestIp,    NULL, &ConMgr_checkIP),
    RKH_TRREG(evIP,         NULL,         NULL, &ConMgr_checkIP),
    RKH_TRREG(evIPInitial,  NULL,         NULL, &ConMgr_checkIP),
    RKH_TRREG(evIPStart,    NULL,         NULL, &ConMgr_checkIP),
    RKH_TRREG(evIPStatus,   NULL,         NULL, &ConMgr_configureFinal),
RKH_END_TRANS_TABLE

RKH_CREATE_COND_STATE(ConMgr_checkConfigTry);
RKH_CREATE_BRANCH_TABLE(ConMgr_checkConfigTry)
    RKH_BRANCH(checkConfigTry,   NULL, &ConMgr_waitRetryConfig),
    RKH_BRANCH(ELSE,           NULL,   &ConMgr_failure),
RKH_END_BRANCH_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_waitRetryConfig, waitRetryConfigEntry, NULL,
                                                    &ConMgr_registered, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_waitRetryConfig)
    RKH_TRREG(evTimeout,  NULL,    configTry, &ConMgr_configureHist),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(ConMgr_connecting, NULL, NULL, 
                             &ConMgr_registered, &ConMgr_waitingServer,
                             socketOpen,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_connecting)
    RKH_TRREG(evClose,  NULL,  socketClose, &ConMgr_disconnecting),
    RKH_TRREG(evNoResponse, NULL,  NULL, &ConMgr_checkConnectTry),
    RKH_TRREG(evError,      NULL,  NULL, &ConMgr_checkConnectTry),
    RKH_TRREG(evClosed,     NULL,  NULL, &ConMgr_checkConnectTry),
    RKH_TRREG(evIPStatus,   NULL,  NULL, &ConMgr_checkConnectTry),
    RKH_TRREG(evIPInitial,  NULL,  NULL, &ConMgr_configure),
    RKH_TRREG(evIPStart,    NULL,  NULL, &ConMgr_configure),
    RKH_TRREG(evIPGprsAct,  NULL,  NULL, &ConMgr_configure),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_waitingServer, connectingEntry, connectingExit,
                                                    &ConMgr_connecting, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_waitingServer)
    RKH_TRREG(evTimeout,    NULL,  getConnStatus, &ConMgr_waitingServer),
    RKH_TRREG(evConnected,  NULL,  NULL, &ConMgr_connected),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(ConMgr_connected, 
                             socketConnected, socketDisconnected, 
                             &ConMgr_connecting, &ConMgr_idle, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_connected)
    RKH_TRREG(evClosed,       NULL,  NULL,        &ConMgr_connecting),
    RKH_TRREG(evRestart,      NULL,  socketClose, &ConMgr_restarting),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_idle, idleEntry, idleExit,
                                                &ConMgr_connected, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_idle)
    RKH_TRREG(evTimeout, NULL,  getConnStatus,  &ConMgr_getStatus),
    RKH_TRREG(evSend,    NULL,  sendRequest,    &ConMgr_sending),
    RKH_TRREG(evRecv,    NULL,  readData,       &ConMgr_receiving),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_getStatus, NULL, getStatusExit,
                                                &ConMgr_connected, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_getStatus)
    RKH_TRINT(evSend,    NULL,  defer),
    RKH_TRINT(evRecv,    NULL,  defer),
    RKH_TRREG(evNoResponse, checkConnectedFailCounter, tryGetStatus, &ConMgr_idle),
    RKH_TRREG(evConnected, NULL, isConnected,   &ConMgr_idle),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(ConMgr_sending, NULL, NULL, 
                             &ConMgr_connected, &ConMgr_waitPrompt, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_sending)
    RKH_TRCOMPLETION(NULL, NULL, &ConMgr_idle),
	RKH_TRREG(evError, NULL, sendFail, &ConMgr_idle),
    RKH_TRREG(evNoResponse, NULL, sendFail, &ConMgr_idle),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_waitPrompt, NULL, NULL, &ConMgr_sending, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_waitPrompt)
    RKH_TRREG(evOk, NULL,  flushData, &ConMgr_waitOk),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_waitOk, NULL, NULL, &ConMgr_sending, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_waitOk)
    RKH_TRREG(evOk, NULL,  sendOk, &ConMgr_sendingFinal),    
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_receiving, NULL, NULL, &ConMgr_connected, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_receiving)
    RKH_TRREG(evOk, NULL,  recvOk, &ConMgr_idle),
	RKH_TRREG(evError, NULL, recvFail, &ConMgr_idle),
	RKH_TRREG(evNoResponse, NULL, recvFail, &ConMgr_idle),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_restarting, NULL, NULL, &ConMgr_connecting, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_restarting)
    RKH_TRREG(evDisconnected, NULL,  NULL, &ConMgr_wReopen),
	RKH_TRREG(evNoResponse, NULL, NULL, &ConMgr_wReopen),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_wReopen, wReopenEntry, wReopenExit, 
                                                        &ConMgr_connecting, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_wReopen)
    RKH_TRREG(evTimeout, NULL,  NULL, &ConMgr_connecting),
RKH_END_TRANS_TABLE

RKH_CREATE_COND_STATE(ConMgr_checkConnectTry);
RKH_CREATE_BRANCH_TABLE(ConMgr_checkConnectTry)
    RKH_BRANCH(checkConnectTry,  NULL, &ConMgr_waitRetryConnect),
    RKH_BRANCH(ELSE,             NULL, &ConMgr_failure),
RKH_END_BRANCH_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_waitRetryConnect,
                                        waitRetryConnEntry, waitRetryConnExit,
                                        &ConMgr_connecting, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_waitRetryConnect)
    RKH_TRREG(evTimeout, NULL,  connectTry, &ConMgr_connecting),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ConMgr_disconnecting, NULL, NULL,
                                                    &ConMgr_registered, NULL);
RKH_CREATE_TRANS_TABLE(ConMgr_disconnecting)
    RKH_TRREG(evDisconnected,  NULL,    NULL, &ConMgr_activeFinal),
    RKH_TRREG(evNoResponse,    NULL,    NULL, &ConMgr_activeFinal),
RKH_END_TRANS_TABLE

/* ............................. Active object ............................. */
struct ConMgr
{
    RKH_SMA_T ao;       /* base structure */
    RKH_TMR_T timer;    
    RKH_TMR_T timerReg;
    rui8_t retryCount; 
    SendEvt *psend;
    int sigLevel;
    char Imei[IMEI_BUF_SIZE];
    char Oper[OPER_BUF_SIZE];
};

typedef struct Apn
{
    char *addr;
    char *usr;
    char *psw;
}Apn;

typedef struct Operator
{
    char *netCode;
    Apn apn;
}Operator;

RKH_SMA_CREATE(ConMgr, conMgr, 1, HCAL, &ConMgr_inactive, init, NULL);
RKH_SMA_DEF_PTR(conMgr);

/* ------------------------------- Constants ------------------------------- */
#define IMEI_SNR_OFFSET  8

/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
/*
 *  Declare and allocate the 'e_tout' event.
 *  The 'e_tout' event with TIMEOUT signal never changes, so it can be
 *  statically allocated just once by means of RKH_ROM_STATIC_EVENT() macro.
 */
static RKH_STATIC_EVENT(e_tout, evToutDelay);
static RKH_STATIC_EVENT(e_regTout, evRegTimeout);
static RKH_ROM_STATIC_EVENT(e_Open, evOpen);
static RKH_ROM_STATIC_EVENT(e_Close, evClose);
static RKH_ROM_STATIC_EVENT(e_NetConnected, evNetConnected);
static RKH_ROM_STATIC_EVENT(e_NetDisconnected, evNetDisconnected);
static RKH_ROM_STATIC_EVENT(e_Sent,     evSent);
static RKH_ROM_STATIC_EVENT(e_SendFail, evSendFail);
static RKH_ROM_STATIC_EVENT(e_RecvFail, evRecvFail);
ReceivedEvt e_Received;

static RKH_QUEUE_T qDefer;
static RKH_EVT_T *qDefer_sto[SIZEOF_QDEFER];

static Operator operTable[] =
{
    { MOVISTAR_OPERATOR, 
        { MOVISTAR_APN_ADDR, MOVISTAR_APN_USER, MOVISTAR_APN_PASS } 
    },
    { CLARO_OPERATOR, 
        { CLARO_APN_ADDR, CLARO_APN_USER, CLARO_APN_PASS } 
    },
    { PERSONAL_OPERATOR, 
        { PERSONAL_APN_ADDR, PERSONAL_APN_USER, PERSONAL_APN_PASS } 
    },
    { NULL }
};

static Apn *defaultAPN = &(operTable[0].apn);

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
static Apn *
getAPNbyOper(char *oper)
{
    Operator *pOper;
    
    for( pOper = &operTable[0]; pOper != NULL; ++pOper)
    {
        if(strcmp(oper, pOper->netCode) == 0)
            return &(pOper->apn);
    }
    return defaultAPN;
}

/* ............................ Initial action ............................. */
static void
init(ConMgr *const me, RKH_EVT_T *pe)
{
	(void)pe;

    RKH_TR_FWK_AO(me);

    RKH_TR_FWK_TIMER(&me->timer);
    RKH_TR_FWK_TIMER(&me->timerReg);
    RKH_TR_FWK_STATE(me, &ConMgr_inactive);
    RKH_TR_FWK_STATE(me, &ConMgr_active);
    RKH_TR_FWK_STATE(me, &ConMgr_initialize);
    RKH_TR_FWK_STATE(me, &ConMgr_sync);
    RKH_TR_FWK_STATE(me, &ConMgr_checkSyncTry);
	RKH_TR_FWK_STATE(me, &ConMgr_init);
    RKH_TR_FWK_STATE(me, &ConMgr_pin);
    RKH_TR_FWK_STATE(me, &ConMgr_setPin);
    RKH_TR_FWK_STATE(me, &ConMgr_enableNetTime);
    RKH_TR_FWK_STATE(me, &ConMgr_getImei);
    RKH_TR_FWK_STATE(me, &ConMgr_cipShutdown);
    RKH_TR_FWK_STATE(me, &ConMgr_initializeFinal);
    RKH_TR_FWK_STATE(me, &ConMgr_registered);
    RKH_TR_FWK_STATE(me, &ConMgr_unregistered);
    RKH_TR_FWK_STATE(me, &ConMgr_failure);
    RKH_TR_FWK_STATE(me, &ConMgr_waitNetClockSync);
    RKH_TR_FWK_STATE(me, &ConMgr_localTime);
    RKH_TR_FWK_STATE(me, &ConMgr_getOper);
    RKH_TR_FWK_STATE(me, &ConMgr_configure);
    RKH_TR_FWK_STATE(me, &ConMgr_configureHist);
    RKH_TR_FWK_STATE(me, &ConMgr_setManualGet);
    RKH_TR_FWK_STATE(me, &ConMgr_setAPN);
    RKH_TR_FWK_STATE(me, &ConMgr_enableGPRS);
    RKH_TR_FWK_STATE(me, &ConMgr_checkIP);
    RKH_TR_FWK_STATE(me, &ConMgr_checkConfigTry);
    RKH_TR_FWK_STATE(me, &ConMgr_waitRetryConfig);
    RKH_TR_FWK_STATE(me, &ConMgr_configureFinal);
    RKH_TR_FWK_STATE(me, &ConMgr_connecting);
    RKH_TR_FWK_STATE(me, &ConMgr_waitingServer);
    RKH_TR_FWK_STATE(me, &ConMgr_connected);
    RKH_TR_FWK_STATE(me, &ConMgr_idle);
    RKH_TR_FWK_STATE(me, &ConMgr_getStatus);
    RKH_TR_FWK_STATE(me, &ConMgr_sending);
    RKH_TR_FWK_STATE(me, &ConMgr_waitPrompt);
    RKH_TR_FWK_STATE(me, &ConMgr_waitOk);
    RKH_TR_FWK_STATE(me, &ConMgr_sendingFinal);
    RKH_TR_FWK_STATE(me, &ConMgr_receiving);
    RKH_TR_FWK_STATE(me, &ConMgr_restarting);
    RKH_TR_FWK_STATE(me, &ConMgr_wReopen);
    RKH_TR_FWK_STATE(me, &ConMgr_waitRetryConnect);
    RKH_TR_FWK_STATE(me, &ConMgr_checkConnectTry);
    RKH_TR_FWK_STATE(me, &ConMgr_disconnecting);
    RKH_TR_FWK_TIMER(&me->timer);
    RKH_TR_FWK_TIMER(&me->timerReg);
    RKH_TR_FWK_SIG(evOpen);
    RKH_TR_FWK_SIG(evClose);
    RKH_TR_FWK_SIG(evCmd);
	RKH_TR_FWK_SIG(evOk);
	RKH_TR_FWK_SIG(evURC);
    RKH_TR_FWK_SIG(evSimPin);
    RKH_TR_FWK_SIG(evSimError);
    RKH_TR_FWK_SIG(evSimReady);
    RKH_TR_FWK_SIG(evImei);
    RKH_TR_FWK_SIG(evReg);
    RKH_TR_FWK_SIG(evNoReg);
    RKH_TR_FWK_SIG(evOper);
    RKH_TR_FWK_SIG(evIPInitial);
    RKH_TR_FWK_SIG(evIPStart);
    RKH_TR_FWK_SIG(evIPStatus);
    RKH_TR_FWK_SIG(evIPGprsAct);
    RKH_TR_FWK_SIG(evConnecting);
    RKH_TR_FWK_SIG(evClosed);
    RKH_TR_FWK_SIG(evConnected);
    RKH_TR_FWK_SIG(evSend);
    RKH_TR_FWK_SIG(evRecv);
    RKH_TR_FWK_SIG(evSent);
    RKH_TR_FWK_SIG(evReceived);
    RKH_TR_FWK_SIG(evSendFail);
    RKH_TR_FWK_SIG(evRecvFail);
    RKH_TR_FWK_SIG(evNetConnected);
    RKH_TR_FWK_SIG(evNetDisconnected);
    RKH_TR_FWK_SIG(evDisconnected);
    RKH_TR_FWK_SIG(evTerminate);
    RKH_TR_FWK_SIG(evNetClockSync);
    RKH_TR_FWK_SIG(evLocalTime);
    RKH_TR_FWK_SIG(evRestart);
    RKH_TR_FWK_SIG(evSigLevel);
    RKH_TR_FWK_SIG(evRegTimeout);

    rkh_queue_init(&qDefer, (const void **)qDefer_sto, SIZEOF_QDEFER, 
                CV(0));

    RKH_TMR_INIT(&me->timer, &e_tout, NULL);
    RKH_TMR_INIT(&me->timerReg, &e_regTout, NULL);
    me->retryCount = 0;
}

/* ............................ Effect actions ............................. */
static void
open(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    (void)me;

    RKH_SMA_POST_FIFO(modMgr, &e_Open, conMgr);

    modPwr_on();
}

static void
close(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    (void)me;

    RKH_SMA_POST_FIFO(modMgr, &e_Close, conMgr);

    modPwr_off();
}

static void
defer(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;

    if( rkh_queue_is_full(&qDefer) != RKH_TRUE )
    	rkh_sma_defer(&qDefer, pe);
}

static void
setSigLevel(ConMgr *const me, RKH_EVT_T *pe)
{
    SigLevelEvt *p;
    (void)me;
    
    p = RKH_UPCAST(SigLevelEvt, pe);
    me->sigLevel = p->value;
}

static void
initializeInit(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;

    me->retryCount = 0;
}

static void
storeImei(ConMgr *const me, RKH_EVT_T *pe)
{
    ImeiEvt *p;

	(void)me;

    p = RKH_UPCAST(ImeiEvt, pe);
    strcpy(me->Imei, p->buf);

    dimbaCfg_clientId(me->Imei + IMEI_SNR_OFFSET);
    dimbaCfg_topic(me->Imei + IMEI_SNR_OFFSET);
}

static void
storeOper(ConMgr *const me, RKH_EVT_T *pe)
{
    OperEvt *p;

	(void)me;

    p = RKH_UPCAST(OperEvt, pe);
    strcpy(me->Oper, p->buf);
}

static void
checkRegStatus(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;

    ModCmd_getRegStatus();
}

static void
startRegStatus(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;

    RKH_TMR_ONESHOT(&me->timer, RKH_UPCAST(RKH_SMA_T, me), CHECK_REG_PERIOD);
}

static void
localTimeGet(ConMgr *const me, RKH_EVT_T *pe)
{
	(void)me;
	(void)pe;

    ModCmd_getLocalTime();
}

static void 
rtimeSync(ConMgr *const me, RKH_EVT_T *pe)
{
	(void)me;

    rtime_set(&(RKH_UPCAST(LocalTimeEvt, pe)->time));
}

static void
configureInit(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;

    me->retryCount = 0;
}

static void
configTry(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;

    ++me->retryCount;
	ModCmd_init();
}

static void 
requestIp(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    (void)me;

    ModCmd_requestIP();
}

static void
connectInit(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;

    me->retryCount = 0;
}

static void
connectTry(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;

    ++me->retryCount;
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
readData(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    (void)me;

    RKH_SET_STATIC_EVENT(RKH_UPCAST(RKH_EVT_T, &e_Received), evReceived);
    ModCmd_readData();
}

static void
sendRequest(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)me;

    me->psend = RKH_UPCAST(SendEvt, pe);

    ModCmd_sendDataRequest((rui16_t)(me->psend->size));
}

static void
flushData(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;

    ModCmd_sendData(me->psend->buf, me->psend->size);
}

static void
sendOk(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    (void)me;

    me->retryCount = 0;
    RKH_SMA_POST_FIFO(mqttProt, &e_Sent, conMgr);
}

static void
recvOk(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    (void)me;

    me->retryCount = 0;
    RKH_SMA_POST_FIFO(mqttProt, RKH_UPCAST(RKH_EVT_T, &e_Received), conMgr);
}

static void
sendFail(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    (void)me;

    RKH_SMA_POST_FIFO(mqttProt, &e_SendFail, conMgr);
	ModCmd_init();
}

static void
recvFail(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    (void)me;

    RKH_SMA_POST_FIFO(mqttProt, &e_RecvFail, conMgr);
	ModCmd_init();
}

static void
tryGetStatus(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;

    ++me->retryCount;
	ModCmd_init();
}

/* ............................. Entry actions ............................. */
static void
sendSync(ConMgr *const me)
{
    ++me->retryCount;

    ModCmd_sync();
}

static void
sendInit(ConMgr *const me)
{
    (void)me;

    ModCmd_initStr();
}

static void
checkPin(ConMgr *const me)
{
    (void)me;

    ModCmd_getPinStatus();
}

static void
setPin(ConMgr *const me)
{
    (void)me;

    ModCmd_setPin(SIM_PIN_NUMBER);
}

static void
netTimeEnable(ConMgr *const me)
{
    (void)me;

    ModCmd_enableNetTime();
}

static void
getImei(ConMgr *const me)
{
    (void)me;

    ModCmd_getImei();
}

static void
cipShutdown(ConMgr *const me)
{
    (void)me;

    ModCmd_cipShutdown();
}

static void 
unregEntry(ConMgr *const me)
{
    ModCmd_getRegStatus();

    RKH_SET_STATIC_EVENT(&e_tout, evTimeout);
    RKH_TMR_ONESHOT(&me->timer, RKH_UPCAST(RKH_SMA_T, me), CHECK_REG_PERIOD);

    RKH_SET_STATIC_EVENT(&e_regTout, evRegTimeout);
    RKH_TMR_ONESHOT(&me->timerReg, RKH_UPCAST(RKH_SMA_T, me), REGISTRATION_TIME);
}

static void 
regEntry(ConMgr *const me)
{
    bsp_regStatus(RegisteredSt);
}

static void 
failureEntry(ConMgr *const me)
{
    RKH_SET_STATIC_EVENT(&e_tout, evTimeout);
    RKH_TMR_ONESHOT(&me->timer, RKH_UPCAST(RKH_SMA_T, me), FAILURE_TRY_DELAY);
    modPwr_off();
}

static void
waitRetryConfigEntry(ConMgr *const me)
{
    RKH_SET_STATIC_EVENT(&e_tout, evTimeout);
    RKH_TMR_ONESHOT(&me->timer, RKH_UPCAST(RKH_SMA_T, me), CONFIG_TRY_DELAY);
}

static void
setupManualGet(ConMgr *const me)
{
    (void)me;

    ModCmd_setManualGet();
}
   
static void
waitNetClockSyncEntry(ConMgr *const me)
{
    RKH_SET_STATIC_EVENT(&e_tout, evTimeout);
    RKH_TMR_ONESHOT(&me->timer, RKH_UPCAST(RKH_SMA_T, me), WAIT_NETCLOCK_TIME);
}

static void
getOper(ConMgr *const me)
{
    (void)me;

    ModCmd_getOper();
}
   
static void
setupAPN(ConMgr *const me)
{
    Apn *apn;
    (void)me;

    apn = getAPNbyOper(me->Oper);
    ModCmd_setupAPN(apn->addr, apn->usr, apn->psw);
}
   
static void
startGPRS(ConMgr *const me)
{
    (void)me;

    ModCmd_startGPRS();
}

static void
getConnStatus(ConMgr *const me)
{
    (void)me;

    ModCmd_getConnStatus();
}

static void
isConnected(ConMgr *const me)
{
    me->retryCount = 0;
}

static void
connectingEntry(ConMgr *const me)
{
    (void)me;

    RKH_SET_STATIC_EVENT(&e_tout, evTimeout);
    RKH_TMR_ONESHOT(&me->timer, RKH_UPCAST(RKH_SMA_T, me), CONNSTATUS_PERIOD);
}

static void
socketConnected(ConMgr *const me)
{
    (void)me;

    me->retryCount = 0;
    RKH_SMA_POST_FIFO(mqttProt, &e_NetConnected, conMgr);
    bsp_netStatus(ConnectedSt);
}

static void
wReopenEntry(ConMgr *const me)
{
    (void)me;

    RKH_SET_STATIC_EVENT(&e_tout, evTimeout);
    RKH_TMR_ONESHOT(&me->timer, RKH_UPCAST(RKH_SMA_T, me), REOPEN_DELAY);
}

static void
waitRetryConnEntry(ConMgr *const me)
{
    (void)me;

    RKH_SET_STATIC_EVENT(&e_tout, evTimeout);
    RKH_TMR_ONESHOT(&me->timer, RKH_UPCAST(RKH_SMA_T, me), CONNECT_TRY_DELAY);
}

static void
idleEntry(ConMgr *const me)
{
    (void)me;
    
    RKH_SET_STATIC_EVENT(&e_tout, evTimeout);
    RKH_TMR_ONESHOT(&me->timer, RKH_UPCAST(RKH_SMA_T, me), CONNSTATUS_PERIOD);
}


/* ............................. Exit actions ............................. */
static void 
unregExit(ConMgr *const me)
{
    rkh_tmr_stop(&me->timer);
    rkh_tmr_stop(&me->timerReg);
}

static void 
regExit(ConMgr *const me)
{
    bsp_regStatus(UnregisteredSt);
}

static void
waitNetClockSyncExit(ConMgr *const me)
{
    rkh_tmr_stop(&me->timer);
}

static void
failureExit(ConMgr *const me)
{
    (void)me;

    modPwr_on();
    ModCmd_init();
    rkh_tmr_stop(&me->timer);
}

static void
wReopenExit(ConMgr *const me)
{
    (void)me;

    rkh_tmr_stop(&me->timer);
}

static void
waitRetryConnExit(ConMgr *const me)
{
    (void)me;

    ModCmd_init();
    rkh_tmr_stop(&me->timer);
}

static void
connectingExit(ConMgr *const me)
{
    (void)me;

    rkh_tmr_stop(&me->timer);
}

static void
socketDisconnected(ConMgr *const me)
{
    (void)me;

    RKH_SMA_POST_FIFO(mqttProt, &e_NetDisconnected, conMgr);
    bsp_netStatus(DisconnectedSt);
}

static void
idleExit(ConMgr *const me)
{
    (void)me;

    rkh_tmr_stop(&me->timer);
}

static void
getStatusExit(ConMgr *const me)
{
    (void)me;

    rkh_sma_recall((RKH_SMA_T *)me, &qDefer);
}

/* ................................ Guards ................................. */
rbool_t
checkSyncTry(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    
    return (me->retryCount < MAX_SYNC_RETRY) ? RKH_TRUE : RKH_FALSE;
}

rbool_t
checkConfigTry(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    
    return (me->retryCount < MAX_CONFIG_RETRY) ? RKH_TRUE : RKH_FALSE;
}

rbool_t
checkConnectTry(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    
    return (me->retryCount < MAX_CONNECT_RETRY) ? RKH_TRUE : RKH_FALSE;
}

rbool_t
checkConnectedFailCounter(ConMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    
    return (me->retryCount < MAX_CONSTATUS_NORESP) ? RKH_TRUE : RKH_FALSE;
}

/* ---------------------------- Global functions --------------------------- */
ReceivedEvt *
ConMgr_ReceiveDataGetRef(void)
{
    return &e_Received;
}

char *
ConMgr_imei(void)
{
    ConMgr *me;

    me = RKH_UPCAST(ConMgr, conMgr); 
    return me->Imei;
}

char *
ConMgr_imeiSNR(void)
{
    ConMgr *me;

    me = RKH_UPCAST(ConMgr, conMgr); 
    return (me->Imei + IMEI_SNR_OFFSET);
}

int
ConMgr_sigLevel(void)
{
    ConMgr *me;

    me = RKH_UPCAST(ConMgr, conMgr); 
    return me->sigLevel;
}

/* ------------------------------ End of file ------------------------------ */
