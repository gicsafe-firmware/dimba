/**
 *  \file       mqttProt.c
 *  \brief      MQTT Client.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.06.05  LeFr  v1.0.00   Initial version
 *  2018.06.05  DaBa  v1.0.00   Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 *  DaBa  Dario Baliña db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"
#include "signals.h"
#include "mqttProt.h"
#include "conmgr.h"

/* ----------------------------- Local macros ------------------------------ */
/* ......................... Declares active object ........................ */
typedef struct MQTTProt MQTTProt;

/* ................... Declares states and pseudostates .................... */
RKH_DCLR_BASIC_STATE Sync_idle, Sync_waitSync, Sync_receiving, 
                     Sync_endCycle, Sync_sending,
                     Client_idle, Client_connRefused, Client_tryConnect,
                     Client_waitToConnect, Client_awaitingAck,
                     Client_waitToPublish, Client_netError;
RKH_DCLR_COMP_STATE Sync_active, Client_connected;
RKH_DCLR_CHOICE_STATE Sync_c10, Sync_c12, Sync_c14, Sync_c25, Sync_c31,
                      Sync_c32, Sync_c38,
                      Client_c7;

/* ........................ Declares initial action ........................ */
static void init(MQTTProt *const me, RKH_EVT_T *pe);

/* ........................ Declares effect actions ........................ */
static void initPublishing(MQTTProt *const me, RKH_EVT_T *pe);
static void publish(MQTTProt *const me, RKH_EVT_T *pe);
static void initRecvAll(MQTTProt *const me, RKH_EVT_T *pe);
static void recvFail(MQTTProt *const me, RKH_EVT_T *pe);
static void parseRecv(MQTTProt *const me, RKH_EVT_T *pe);
static void sendMsgFail(MQTTProt *const me, RKH_EVT_T *pe);
static void setMsgState(MQTTProt *const me, RKH_EVT_T *pe);
static void parseError(MQTTProt *const me, RKH_EVT_T *pe);
static void noConsumed(MQTTProt *const me, RKH_EVT_T *pe);
static void cleanBuf(MQTTProt *const me, RKH_EVT_T *pe);
static void recvMsgError(MQTTProt *const me, RKH_EVT_T *pe);
static void initSendAll(MQTTProt *const me, RKH_EVT_T *pe);
static void initSendOk(MQTTProt *const me, RKH_EVT_T *pe);
static void sendOneMsg(MQTTProt *const me, RKH_EVT_T *pe);
static void endSendAll(MQTTProt *const me, RKH_EVT_T *pe);
static void nextSend(MQTTProt *const me, RKH_EVT_T *pe);

/* ......................... Declares entry actions ........................ */
static void enWaitToConnect(MQTTProt *const me, RKH_EVT_T *pe);
static void BrokerConnect(MQTTProt *const me, RKH_EVT_T *pe);
static void enWaitSync(MQTTProt *const me, RKH_EVT_T *pe);
static void recvAll(MQTTProt *const me, RKH_EVT_T *pe);
static void sendAll(MQTTProt *const me, RKH_EVT_T *pe);

/* ......................... Declares exit actions ......................... */
static void exWaitToConnect(MQTTProt *const me, RKH_EVT_T *pe);
static void exWaitToPublish(MQTTProt *const me, RKH_EVT_T *pe);
static void exWaitSync(MQTTProt *const me, RKH_EVT_T *pe);

/* ............................ Declares guards ............................ */
static rbool_t isConnectOk(const RKH_SM_T *me, RKH_EVT_T *pe);
static rbool_t isUnpackError(const RKH_SM_T *me, RKH_EVT_T *pe);
static rbool_t isConsumed(const RKH_SM_T *me, RKH_EVT_T *pe);
static rbool_t isNotError(const RKH_SM_T *me, RKH_EVT_T *pe);
static rbool_t isRecvBufFull(const RKH_SM_T *me, RKH_EVT_T *pe);
static rbool_t isInitOk(const RKH_SM_T *me, RKH_EVT_T *pe);
static rbool_t isThereMsg(const RKH_SM_T *me, RKH_EVT_T *pe);
static rbool_t isNotResend(const RKH_SM_T *me, RKH_EVT_T *pe);
static rbool_t isSetMsgStateOk(const RKH_SM_T *me, RKH_EVT_T *pe);

/* ........................ States and pseudostates ........................ */
RKH_CREATE_BASIC_STATE(Sync_idle, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(Sync_idle)
    RKH_TRREG(evActivate, NULL, NULL, &Sync_active),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(Sync_active, NULL, NULL, RKH_ROOT, 
                             &Sync_waitSync, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(Sync_active)
    RKH_TRREG(evDeactivate, NULL, NULL, &Sync_idle),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Sync_waitSync, enWaitSync, exWaitSync, &Sync_active, 
                       NULL);
RKH_CREATE_TRANS_TABLE(Sync_waitSync)
    RKH_TRREG(evSyncTout, NULL, initRecvAll, &Sync_receiving),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Sync_receiving, recvAll, NULL, &Sync_active, NULL);
RKH_CREATE_TRANS_TABLE(Sync_receiving)
    RKH_TRREG(evRecvFail, NULL, recvFail, &Sync_endCycle),
    RKH_TRREG(evReceived, NULL, parseRecv, &Sync_c10),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Sync_endCycle, NULL, NULL, &Sync_active, NULL);
RKH_CREATE_TRANS_TABLE(Sync_endCycle)
    RKH_TRCOMPLETION(NULL, NULL, &Sync_waitSync),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Sync_sending, sendAll, NULL, &Sync_active, NULL);
RKH_CREATE_TRANS_TABLE(Sync_sending)
    RKH_TRREG(evSendFail, NULL, sendMsgFail, &Sync_endCycle),
    RKH_TRREG(evSent, NULL, setMsgState, &Sync_c38),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_idle, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(Client_idle)
    RKH_TRREG(evNetConnected, NULL, NULL, &Client_connected),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_connRefused, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(Client_connRefused)
    RKH_TRREG(evNetDisconnected, NULL, NULL, &Client_idle),
RKH_END_TRANS_TABLE

RKH_CREATE_HISTORY_STORAGE(Client_connected);

RKH_CREATE_COMP_REGION_STATE(Client_connected, NULL, NULL, RKH_ROOT, 
                             &Client_tryConnect, NULL,
                             RKH_SHISTORY, NULL, NULL, NULL, 
                             RKH_GET_HISTORY_STORAGE(Client_connected));
RKH_CREATE_TRANS_TABLE(Client_connected)
    RKH_TRREG(evConnRefused, NULL, NULL, &Client_connRefused),
    RKH_TRREG(evNetDisconnected, NULL, NULL, &Client_netError),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_tryConnect, BrokerConnect, NULL, 
                       &Client_connected, NULL);
RKH_CREATE_TRANS_TABLE(Client_tryConnect)
    RKH_TRCOMPLETION(NULL, NULL, &Client_c7),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_waitToConnect, enWaitToConnect, exWaitToConnect, 
                       &Client_connected, NULL);
RKH_CREATE_TRANS_TABLE(Client_waitToConnect)
    RKH_TRREG(evWaitConnectTout, NULL, NULL, &Client_tryConnect),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_awaitingAck, NULL, NULL, &Client_connected, NULL);
RKH_CREATE_TRANS_TABLE(Client_awaitingAck)
    RKH_TRREG(evConnAccepted, NULL, initPublishing, &Client_waitToPublish),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_waitToPublish, NULL, exWaitToPublish, 
                       &Client_connected, NULL);
RKH_CREATE_TRANS_TABLE(Client_waitToPublish)
    RKH_TRINT(evConnAccepted, NULL, publish),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_netError, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(Client_netError)
    RKH_TRREG(evNetConnected, NULL, NULL, &Client_connectedHist),
RKH_END_TRANS_TABLE
 
RKH_CREATE_CHOICE_STATE(Client_c7);
RKH_CREATE_BRANCH_TABLE(Client_c7)
	RKH_BRANCH(isConnectOk, NULL, &Client_awaitingAck),
	RKH_BRANCH(ELSE,        NULL, &Client_waitToConnect),
RKH_END_BRANCH_TABLE

RKH_CREATE_CHOICE_STATE(Sync_c10);
RKH_CREATE_BRANCH_TABLE(Sync_c10)
	RKH_BRANCH(isConsumed,      NULL,       &Sync_c12),
	RKH_BRANCH(isUnpackError,   parseError, &Sync_endCycle),
	RKH_BRANCH(ELSE,            noConsumed, &Sync_c14),
RKH_END_BRANCH_TABLE

RKH_CREATE_CHOICE_STATE(Sync_c12);
RKH_CREATE_BRANCH_TABLE(Sync_c12)
	RKH_BRANCH(isNotError,  cleanBuf,       &Sync_receiving),
	RKH_BRANCH(ELSE,        recvMsgError,   &Sync_endCycle),
RKH_END_BRANCH_TABLE

RKH_CREATE_CHOICE_STATE(Sync_c14);
RKH_CREATE_BRANCH_TABLE(Sync_c14)
	RKH_BRANCH(isRecvBufFull,   NULL,           &Sync_endCycle),
	RKH_BRANCH(ELSE,            initSendAll,    &Sync_c25),
RKH_END_BRANCH_TABLE

RKH_CREATE_CHOICE_STATE(Sync_c25);
RKH_CREATE_BRANCH_TABLE(Sync_c25)
	RKH_BRANCH(isInitOk,    initSendOk, &Sync_c31),
	RKH_BRANCH(ELSE,        NULL,       &Sync_endCycle),
RKH_END_BRANCH_TABLE

RKH_CREATE_CHOICE_STATE(Sync_c31);
RKH_CREATE_BRANCH_TABLE(Sync_c31)
	RKH_BRANCH(isThereMsg,  sendOneMsg, &Sync_c32),
	RKH_BRANCH(ELSE,        endSendAll, &Sync_endCycle),
RKH_END_BRANCH_TABLE

RKH_CREATE_CHOICE_STATE(Sync_c32);
RKH_CREATE_BRANCH_TABLE(Sync_c32)
	RKH_BRANCH(isNotResend, nextSend,   &Sync_c31),
	RKH_BRANCH(ELSE,        NULL,       &Sync_sending),
RKH_END_BRANCH_TABLE

RKH_CREATE_CHOICE_STATE(Sync_c38);
RKH_CREATE_BRANCH_TABLE(Sync_c38)
	RKH_BRANCH(isSetMsgStateOk, nextSend,   &Sync_c31),
	RKH_BRANCH(ELSE,            NULL,       &Sync_endCycle),
RKH_END_BRANCH_TABLE

/* ............................. Active object ............................. */
struct MQTTProt
{
    RKH_SMA_T ao;           /* Base structure */
    RKHSmaVtbl vtbl;        /* Virtual table */
    RKH_SM_T sync;          /* Sync orthogonal region */
};

RKH_SMA_CREATE(MQTTProt, mqttProt, 2, HCAL, &Client_idle, init, 
               NULL);
RKH_SMA_DEF_PTR(mqttProt);
RKH_SM_CONST_CREATE(syncRegion, 0, HCAL, &Sync_idle, NULL, NULL);

/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
void 
dispatch(RKH_SMA_T *me, void *arg)
{
    rkh_sm_dispatch((RKH_SM_T *)me, (RKH_EVT_T *)arg);
    rkh_sm_dispatch(&RKH_DOWNCAST(MQTTProt, me)->sync, (RKH_EVT_T *)arg);
}

/* ............................ Initial action ............................. */
static void
init(MQTTProt *const me, RKH_EVT_T *pe)
{
	(void)pe;

    RKH_TR_FWK_AO(me);
    RKH_TR_FWK_QUEUE(&RKH_UPCAST(RKH_SMA_T, me)->equeue);
    RKH_TR_FWK_STATE(me, &Client_idle);
}

/* ............................ Effect actions ............................. */
static void
initPublishing(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
publish(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
initRecvAll(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
recvFail(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
parseRecv(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
sendMsgFail(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
setMsgState(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
parseError(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
noConsumed(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
cleanBuf(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
recvMsgError(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
initSendAll(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
initSendOk(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
sendOneMsg(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
endSendAll(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
nextSend(MQTTProt *const me, RKH_EVT_T *pe)
{
}

/* ............................. Entry actions ............................. */
static void 
enWaitToConnect(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
BrokerConnect(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
enWaitSync(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
recvAll(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
sendAll(MQTTProt *const me, RKH_EVT_T *pe)
{
}

/* ............................. Exit actions .............................. */
static void 
exWaitToConnect(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
exWaitToPublish(MQTTProt *const me, RKH_EVT_T *pe)
{
}

static void 
exWaitSync(MQTTProt *const me, RKH_EVT_T *pe)
{
}

/* ................................ Guards ................................. */
static rbool_t 
isConnectOk(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return RKH_FALSE;
}

static rbool_t 
isConsumed(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return RKH_FALSE;
}

static rbool_t 
isUnpackError(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return RKH_FALSE;
}

static rbool_t 
isNotError(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return RKH_FALSE;
}

static rbool_t 
isRecvBufFull(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return RKH_FALSE;
}

static rbool_t 
isInitOk(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return RKH_FALSE;
}

static rbool_t 
isThereMsg(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return RKH_FALSE;
}

static rbool_t 
isNotResend(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return RKH_FALSE;
}

static rbool_t 
isSetMsgStateOk(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return RKH_FALSE;
}

/* ---------------------------- Global functions --------------------------- */
void
MQTTProt_ctor(void)
{
    MQTTProt *me;

    me = RKH_DOWNCAST(MQTTProt, mqttProt);
    me->vtbl = rkhSmaVtbl;
    me->vtbl.task = dispatch;
    rkh_sma_ctor(RKH_UPCAST(RKH_SMA_T, me), &me->vtbl);
    RKH_SM_INIT(&me->sync, syncRegion, 0, HCAL, Sync_idle, NULL, NULL);
}

/* ------------------------------ End of file ------------------------------ */
