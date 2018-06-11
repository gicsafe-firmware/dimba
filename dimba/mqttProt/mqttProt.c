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
#include "rkhtmr.h"
#include "signals.h"
#include "mqttProt.h"
#include "conmgr.h"
#include "mqtt.h"

/* ----------------------------- Local macros ------------------------------ */
/* ......................... Declares active object ........................ */
typedef struct MQTTProt MQTTProt;
typedef struct SyncRegion SyncRegion;

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
static void handleRecvMsg(MQTTProt *const me, RKH_EVT_T *pe);
static void activateSync(MQTTProt *const me, RKH_EVT_T *pe);

/* ......................... Declares entry actions ........................ */
static void enWaitToConnect(MQTTProt *const me, RKH_EVT_T *pe);
static void brokerConnect(MQTTProt *const me, RKH_EVT_T *pe);
static void enWaitSync(MQTTProt *const me, RKH_EVT_T *pe);
static void recvAll(MQTTProt *const me, RKH_EVT_T *pe);
static void sendAll(MQTTProt *const me, RKH_EVT_T *pe);
static void enWaitToPublish(MQTTProt *const me, RKH_EVT_T *pe);

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
    RKH_TRREG(evNetConnected, NULL, activateSync, &Client_connected),
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

RKH_CREATE_BASIC_STATE(Client_tryConnect, brokerConnect, NULL, 
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
    RKH_TRREG(evConnAccepted, NULL, NULL, &Client_waitToPublish),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_waitToPublish, enWaitToPublish, exWaitToPublish, 
                       &Client_connected, NULL);
RKH_CREATE_TRANS_TABLE(Client_waitToPublish)
    RKH_TRINT(evWaitPublishTout, NULL, publish),
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
	RKH_BRANCH(isConsumed,      handleRecvMsg,  &Sync_c12),
	RKH_BRANCH(isUnpackError,   parseError,     &Sync_endCycle),
	RKH_BRANCH(ELSE,            noConsumed,     &Sync_c14),
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
struct SyncRegion
{
    RKH_SM_T sm;            /* Orthogonal region */
    MQTTProt *itsMQTTProt;
    RKH_TMR_T syncTmr;
};

struct MQTTProt
{
    RKH_SMA_T ao;               /* Base structure */
    RKHSmaVtbl vtbl;            /* Virtual table */
    SyncRegion itsSyncRegion;   /* Sync orthogonal region */
    RKH_TMR_T publishTmr;
    RKH_TMR_T tryConnTmr;
    struct mqtt_client client;
    uint8_t sendbuf[2048];  /* sendbuf should be large enough to hold */
                            /* multiple whole mqtt messages */
    uint8_t recvbuf[1024];  /* recvbuf should be large enough any whole */
                            /* mqtt message expected to be received */
    enum MQTTErrors operRes;
};

RKH_SMA_CREATE(MQTTProt, mqttProt, 2, HCAL, &Client_idle, init, NULL);
RKH_SMA_DEF_PTR(mqttProt);
RKH_SM_CONST_CREATE(syncRegion, 0, HCAL, &Sync_idle, NULL, NULL);

/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static RKH_ROM_STATIC_EVENT(evWaitConnectToutObj, evWaitConnectTout);
static RKH_ROM_STATIC_EVENT(evWaitPublishToutObj, evWaitPublishTout);
static RKH_ROM_STATIC_EVENT(evWaitSyncToutObj, evWaitSyncTout);
static RKH_ROM_STATIC_EVENT(evRecvObj, evRecv);
static RKH_ROM_STATIC_EVENT(evActivateObj, evActivate);
static SendEvt evSendObj;
static LocalSendAll localSend;
static LocalRecvAll localRecv;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
void 
dispatch(RKH_SMA_T *me, void *arg)
{
    SyncRegion *region;
    
    region = &(RKH_DOWNCAST(MQTTProt, me)->itsSyncRegion);
    rkh_sm_dispatch((RKH_SM_T *)me, (RKH_EVT_T *)arg);
    rkh_sm_dispatch(RKH_UPCAST(RKH_SM_T, region), (RKH_EVT_T *)arg);
}

/* ............................ Initial action ............................. */
static void
init(MQTTProt *const me, RKH_EVT_T *pe)
{
	(void)pe;

    RKH_TR_FWK_AO(me);
    RKH_TR_FWK_QUEUE(&RKH_UPCAST(RKH_SMA_T, me)->equeue);
    RKH_TR_FWK_STATE(me, &Client_idle);

    mqtt_init(&me->client, 0, me->sendbuf, sizeof(me->sendbuf), 
              me->recvbuf, sizeof(me->recvbuf), 0);
}

/* ............................ Effect actions ............................. */
static void 
publish(MQTTProt *const me, RKH_EVT_T *pe)
{
    const char *topic;
    char application_message[128];

    /* Get digital input changes and analog input samples */
    /* Format a payload to send */
    /* mqtt_publish(...); */

    topic = "datetime";
    strcpy(application_message, "The time is 2018-06-04 08:12:12");
    me->operRes = mqtt_publish(&me->client, 
                               topic, 
                               application_message, 
                               strlen(application_message) + 1, 
                               MQTT_PUBLISH_QOS_0);
}

static void 
initRecvAll(MQTTProt *const me, RKH_EVT_T *pe)
{
    mqtt_initRecvAll();
}

static void 
recvFail(MQTTProt *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = ((SyncRegion *)me)->itsMQTTProt;
    mqtt_recvFail(&realMe->client, &localRecv); /* an error occurred */
}

static void 
parseRecv(MQTTProt *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;
    ReceivedEvt *evt;

    realMe = ((SyncRegion *)me)->itsMQTTProt;
    evt = RKH_DOWNCAST(ReceivedEvt, pe);

    memcpy(realMe->client.recv_buffer.curr, evt->buf, evt->size);
    localRecv.rv = evt->size;
    mqtt_parseRecv(&realMe->client, &localRecv);
}

static void 
sendMsgFail(MQTTProt *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = ((SyncRegion *)me)->itsMQTTProt;
    mqtt_sendMsgFail(&realMe->client, &localSend);
}

static void 
setMsgState(MQTTProt *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = ((SyncRegion *)me)->itsMQTTProt;
    mqtt_setMsgState(&realMe->client, &localSend);
}

static void 
parseError(MQTTProt *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = ((SyncRegion *)me)->itsMQTTProt;
    mqtt_parseError(&realMe->client, &localRecv);
}

static void 
noConsumed(MQTTProt *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = ((SyncRegion *)me)->itsMQTTProt;
    mqtt_noConsumed(&realMe->client, &localRecv);
}

static void 
cleanBuf(MQTTProt *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = ((SyncRegion *)me)->itsMQTTProt;
    mqtt_cleanBuf(&realMe->client, &localRecv);
}

static void 
recvMsgError(MQTTProt *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = ((SyncRegion *)me)->itsMQTTProt;
    mqtt_recvMsgError(&realMe->client, &localRecv);
}

static void 
initSendAll(MQTTProt *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = ((SyncRegion *)me)->itsMQTTProt;
    mqtt_initSendAll(&realMe->client, &localSend);
}

static void 
initSendOk(MQTTProt *const me, RKH_EVT_T *pe)
{
    (void *)me;
    (void *)pe;
}

static void 
sendOneMsg(MQTTProt *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = ((SyncRegion *)me)->itsMQTTProt;
    mqtt_sendOneMsg(&realMe->client, &localSend);
}

static void 
endSendAll(MQTTProt *const me, RKH_EVT_T *pe)
{
     mqtt_endSendAll(&me->client);
}

static void 
nextSend(MQTTProt *const me, RKH_EVT_T *pe)
{
    mqtt_nextSend(&localSend);
}

static void 
handleRecvMsg(MQTTProt *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = ((SyncRegion *)me)->itsMQTTProt;
    mqtt_handleRecvMsg(&realMe->client, &localRecv);
}

static void 
activateSync(MQTTProt *const me, RKH_EVT_T *pe)
{
    RKH_SMA_POST_FIFO(me, &evActivateObj, me);
}

/* ............................. Entry actions ............................. */
static void 
enWaitToConnect(MQTTProt *const me, RKH_EVT_T *pe)
{
    RKH_TMR_INIT(&me->tryConnTmr, &evWaitConnectToutObj, NULL);
    RKH_TMR_ONESHOT(&me->tryConnTmr, RKH_UPCAST(RKH_SMA_T, me), 
                    WAIT_CONNECT_TIME);
}

static void 
brokerConnect(MQTTProt *const me, RKH_EVT_T *pe)
{
    me->operRes = mqtt_connect(&me->client, 
                               "publishing_client", 
                               NULL, NULL, 0, NULL, NULL, 0, 400);
}

static void 
enWaitSync(MQTTProt *const me, RKH_EVT_T *pe)
{
    SyncRegion *realMe;

    realMe = (SyncRegion *)me;
    RKH_TMR_INIT(&realMe->syncTmr, &evWaitSyncToutObj, NULL);
    RKH_TMR_ONESHOT(&realMe->syncTmr, 
                    RKH_UPCAST(RKH_SMA_T, realMe->itsMQTTProt), 
                    SYNC_TIME);
}

static void 
recvAll(MQTTProt *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = ((SyncRegion *)me)->itsMQTTProt;
    RKH_SMA_POST_FIFO(conMgr, &evRecvObj, realMe);
}

static void 
sendAll(MQTTProt *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = ((SyncRegion *)me)->itsMQTTProt;
    RKH_SET_STATIC_EVENT(RKH_UPCAST(RKH_EVT_T, &evSendObj), evSend);
    evSendObj.size = 0;
    memcpy(evSendObj.buf, localSend.msg->start, localSend.msg->size);
    RKH_SMA_POST_FIFO(conMgr, &evSendObj, realMe);
}

static void 
enWaitToPublish(MQTTProt *const me, RKH_EVT_T *pe)
{
    RKH_TMR_INIT(&me->publishTmr, &evWaitPublishToutObj, NULL);
    RKH_TMR_PERIODIC(&me->publishTmr, RKH_UPCAST(RKH_SMA_T, me), 0, 
                     PUBLISH_TIME);
}

/* ............................. Exit actions .............................. */
static void 
exWaitToConnect(MQTTProt *const me, RKH_EVT_T *pe)
{
    rkh_tmr_stop(&me->tryConnTmr);
}

static void 
exWaitToPublish(MQTTProt *const me, RKH_EVT_T *pe)
{
    rkh_tmr_stop(&me->publishTmr);
}

static void 
exWaitSync(MQTTProt *const me, RKH_EVT_T *pe)
{
    SyncRegion *realMe;

    realMe = (SyncRegion *)me;
    rkh_tmr_stop(&realMe->syncTmr);
}

/* ................................ Guards ................................. */
static rbool_t 
isConnectOk(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return RKH_UPCAST(MQTTProt, me)->operRes == MQTT_OK ? RKH_TRUE: RKH_FALSE;
}

static rbool_t 
isConsumed(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return mqtt_isConsumed(&localRecv);
}

static rbool_t 
isUnpackError(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return mqtt_isUnpackError(&localRecv);
}

static rbool_t 
isNotError(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return mqtt_isNotError(&localRecv);
}

static rbool_t 
isRecvBufFull(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return mqtt_isRecvBufFull(&localRecv);
}

static rbool_t 
isInitOk(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return mqtt_isInitOk(&localSend);
}

static rbool_t 
isThereMsg(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return mqtt_isThereMsg(&localSend);
}

static rbool_t 
isNotResend(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return localSend.resend == 0;
}

static rbool_t 
isSetMsgStateOk(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    return mqtt_isSetMsgStateResult(&localSend);
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
    RKH_SM_INIT((RKH_SM_T *)&(me->itsSyncRegion), syncRegion, 0, HCAL, Sync_idle, 
                NULL, NULL);
}

/* ------------------------------ End of file ------------------------------ */
