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
 *  DaBa  Dario Bali�a db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include <stdio.h>
#include <string.h>
#include "rkh.h"
#include "rkhtmr.h"
#include "signals.h"
#include "mqttProt.h"
#include "conmgr.h"
#include "mqtt.h"
#include "epoch.h"
#include "date.h"

/* ----------------------------- Local macros ------------------------------ */
/* ......................... Declares active object ........................ */
typedef struct MQTTProt MQTTProt;
typedef struct SyncRegion SyncRegion;

/* ................... Declares states and pseudostates .................... */
RKH_DCLR_BASIC_STATE Sync_Idle, Sync_WaitSync, Sync_Receiving, 
                     Sync_EndCycle, Sync_Sending,
                     Client_Idle, Client_TryConnect,
                     Client_AwaitingAck, Client_WaitToPublish, 
                     Client_WaitToUse0, Client_WaitToUse1;
RKH_DCLR_COMP_STATE Sync_Active, Client_Connected;
RKH_DCLR_CHOICE_STATE Sync_C10, Sync_C12, Sync_C14, Sync_C25, Sync_C31,
                      Sync_C32, Sync_C36, Sync_C38,
                      Client_C7, Client_C15, Client_C20;

/* ........................ Declares initial action ........................ */
static void init(MQTTProt *const me, RKH_EVT_T *pe);

/* ........................ Declares effect actions ........................ */
static void publish(MQTTProt *const me, RKH_EVT_T *pe);
static void initRecvAll(SyncRegion *const me, RKH_EVT_T *pe);
static void recvFail(SyncRegion *const me, RKH_EVT_T *pe);
static void parseRecv(SyncRegion *const me, RKH_EVT_T *pe);
static void sendMsgFail(SyncRegion *const me, RKH_EVT_T *pe);
static void setMsgState(SyncRegion *const me, RKH_EVT_T *pe);
static void parseError(SyncRegion *const me, RKH_EVT_T *pe);
static void noConsumed(SyncRegion *const me, RKH_EVT_T *pe);
static void cleanBuf(SyncRegion *const me, RKH_EVT_T *pe);
static void recvMsgError(SyncRegion *const me, RKH_EVT_T *pe);
static void initSendAll(SyncRegion *const me, RKH_EVT_T *pe);
static void initSendOk(SyncRegion *const me, RKH_EVT_T *pe);
static void sendOneMsg(SyncRegion *const me, RKH_EVT_T *pe);
static void endSendAll(SyncRegion *const me, RKH_EVT_T *pe);
static void nextSend(SyncRegion *const me, RKH_EVT_T *pe);
static void handleRecvMsg(SyncRegion *const me, RKH_EVT_T *pe);
static void activateSync(MQTTProt *const me, RKH_EVT_T *pe);
static void releaseUse(SyncRegion *const me, RKH_EVT_T *pe);
static void deactivateSync(MQTTProt *const me, RKH_EVT_T *pe);
static void reconnect(SyncRegion *const me, RKH_EVT_T *pe);
static void reconnectSync(SyncRegion *const me, RKH_EVT_T *pe);

/* ......................... Declares entry actions ........................ */
static void enAwaitingAck(MQTTProt *const me, RKH_EVT_T *pe);
static void brokerConnect(MQTTProt *const me, RKH_EVT_T *pe);
static void enWaitToPublish(MQTTProt *const me, RKH_EVT_T *pe);
static void enWaitSync(SyncRegion *const me, RKH_EVT_T *pe);
static void recvAll(SyncRegion *const me, RKH_EVT_T *pe);
static void sendAll(SyncRegion *const me, RKH_EVT_T *pe);

/* ......................... Declares exit actions ......................... */
static void exAwaitingAck(MQTTProt *const me, RKH_EVT_T *pe);
static void exWaitToPublish(MQTTProt *const me, RKH_EVT_T *pe);
static void exWaitSync(SyncRegion *const me, RKH_EVT_T *pe);

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
static rbool_t isLocked(const RKH_SM_T *me, RKH_EVT_T *pe);
static rbool_t isReconnect(const RKH_SM_T *me, RKH_EVT_T *pe);

/* ........................ States and pseudostates ........................ */
RKH_CREATE_BASIC_STATE(Sync_Idle, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(Sync_Idle)
    RKH_TRREG(evActivate, NULL, NULL, &Sync_Active),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(Sync_Active, NULL, NULL, RKH_ROOT, 
                             &Sync_WaitSync, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(Sync_Active)
    RKH_TRREG(evNetDisconnected, NULL, NULL, &Sync_Idle),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Sync_WaitSync, enWaitSync, exWaitSync, &Sync_Active, 
                       NULL);
RKH_CREATE_TRANS_TABLE(Sync_WaitSync)
    RKH_TRREG(evDeactivate, NULL, reconnect, &Sync_Idle),
    RKH_TRREG(evWaitSyncTout, NULL, NULL, &Sync_C36),
RKH_END_TRANS_TABLE

RKH_CREATE_CHOICE_STATE(Sync_C36);
RKH_CREATE_BRANCH_TABLE(Sync_C36)
	RKH_BRANCH(isReconnect, reconnectSync, &Sync_WaitSync),
	RKH_BRANCH(ELSE,        initRecvAll, &Sync_Receiving),
RKH_END_BRANCH_TABLE

RKH_CREATE_BASIC_STATE(Sync_Receiving, recvAll, NULL, &Sync_Active, NULL);
RKH_CREATE_TRANS_TABLE(Sync_Receiving)
    RKH_TRREG(evReceived, NULL, parseRecv, &Sync_C10),
    RKH_TRREG(evRecvFail, NULL, recvFail, &Sync_EndCycle),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Sync_EndCycle, NULL, NULL, &Sync_Active, NULL);
RKH_CREATE_TRANS_TABLE(Sync_EndCycle)
    RKH_TRCOMPLETION(NULL, releaseUse, &Sync_WaitSync),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_Idle, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(Client_Idle)
    RKH_TRREG(evNetConnected, NULL, activateSync, &Client_Connected),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(Client_Connected, NULL, NULL, RKH_ROOT, 
                             &Client_C15, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(Client_Connected)
    RKH_TRREG(evNetDisconnected, NULL, NULL, &Client_Idle),
    RKH_TRCOMPLETION(NULL, deactivateSync, &Client_Idle),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_TryConnect, brokerConnect, NULL, 
                       &Client_Connected, NULL);
RKH_CREATE_TRANS_TABLE(Client_TryConnect)
    RKH_TRCOMPLETION(NULL, NULL, &Client_C7),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_AwaitingAck, NULL, NULL, &Client_Connected, NULL);
RKH_CREATE_TRANS_TABLE(Client_AwaitingAck)
    RKH_TRREG(evConnAccepted, NULL, NULL, &Client_WaitToPublish),
    RKH_TRREG(evWaitConnectTout, NULL, NULL, &Client_ConnectedFinal),
    RKH_TRREG(evConnRefused, NULL, NULL, &Client_ConnectedFinal),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_WaitToPublish, enWaitToPublish, exWaitToPublish, 
                       &Client_Connected, NULL);
RKH_CREATE_TRANS_TABLE(Client_WaitToPublish)
    RKH_TRREG(evWaitPublishTout, NULL, NULL, &Client_C20),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_WaitToUse0, NULL, NULL, &Client_Connected, NULL);
RKH_CREATE_TRANS_TABLE(Client_WaitToUse0)
    RKH_TRREG(evUnlocked, NULL, NULL, &Client_TryConnect),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_WaitToUse1, NULL, NULL, &Client_Connected, NULL);
RKH_CREATE_TRANS_TABLE(Client_WaitToUse1)
    RKH_TRREG(evUnlocked, NULL, publish, &Client_WaitToPublish),
RKH_END_TRANS_TABLE

RKH_CREATE_CHOICE_STATE(Client_C7);
RKH_CREATE_BRANCH_TABLE(Client_C7)
	RKH_BRANCH(isConnectOk, NULL, &Client_AwaitingAck),
	RKH_BRANCH(ELSE,        NULL, &Client_ConnectedFinal),
RKH_END_BRANCH_TABLE

RKH_CREATE_CHOICE_STATE(Client_C15);
RKH_CREATE_BRANCH_TABLE(Client_C15)
	RKH_BRANCH(isLocked, NULL, &Client_WaitToUse0),
	RKH_BRANCH(ELSE,     NULL, &Client_TryConnect),
RKH_END_BRANCH_TABLE

RKH_CREATE_CHOICE_STATE(Client_C20);
RKH_CREATE_BRANCH_TABLE(Client_C20)
	RKH_BRANCH(isLocked, NULL,      &Client_WaitToUse1),
	RKH_BRANCH(ELSE,     publish,   &Client_WaitToPublish),
RKH_END_BRANCH_TABLE

RKH_CREATE_CHOICE_STATE(Sync_C10);
RKH_CREATE_BRANCH_TABLE(Sync_C10)
	RKH_BRANCH(isConsumed,      handleRecvMsg,  &Sync_C12),
	RKH_BRANCH(isUnpackError,   parseError,     &Sync_EndCycle),
	RKH_BRANCH(ELSE,            noConsumed,     &Sync_C14),
RKH_END_BRANCH_TABLE

RKH_CREATE_CHOICE_STATE(Sync_C12);
RKH_CREATE_BRANCH_TABLE(Sync_C12)
	RKH_BRANCH(isNotError,  cleanBuf,       &Sync_Receiving),
	RKH_BRANCH(ELSE,        recvMsgError,   &Sync_EndCycle),
RKH_END_BRANCH_TABLE

RKH_CREATE_CHOICE_STATE(Sync_C14);
RKH_CREATE_BRANCH_TABLE(Sync_C14)
	RKH_BRANCH(isRecvBufFull,   NULL,           &Sync_EndCycle),
	RKH_BRANCH(ELSE,            initSendAll,    &Sync_C25),
RKH_END_BRANCH_TABLE

RKH_CREATE_CHOICE_STATE(Sync_C25);
RKH_CREATE_BRANCH_TABLE(Sync_C25)
	RKH_BRANCH(isInitOk,    initSendOk, &Sync_C31),
	RKH_BRANCH(ELSE,        NULL,       &Sync_EndCycle),
RKH_END_BRANCH_TABLE

RKH_CREATE_CHOICE_STATE(Sync_C31);
RKH_CREATE_BRANCH_TABLE(Sync_C31)
	RKH_BRANCH(isThereMsg,  sendOneMsg, &Sync_C32),
	RKH_BRANCH(ELSE,        endSendAll, &Sync_EndCycle),
RKH_END_BRANCH_TABLE

RKH_CREATE_CHOICE_STATE(Sync_C32);
RKH_CREATE_BRANCH_TABLE(Sync_C32)
	RKH_BRANCH(isNotResend, nextSend,   &Sync_C31),
	RKH_BRANCH(ELSE,        NULL,       &Sync_Sending),
RKH_END_BRANCH_TABLE

RKH_CREATE_BASIC_STATE(Sync_Sending, sendAll, NULL, &Sync_Active, NULL);
RKH_CREATE_TRANS_TABLE(Sync_Sending)
    RKH_TRREG(evSendFail, NULL, sendMsgFail, &Sync_EndCycle),
    RKH_TRREG(evSent, NULL, setMsgState, &Sync_C38),
RKH_END_TRANS_TABLE

RKH_CREATE_CHOICE_STATE(Sync_C38);
RKH_CREATE_BRANCH_TABLE(Sync_C38)
	RKH_BRANCH(isSetMsgStateOk, nextSend,   &Sync_C31),
	RKH_BRANCH(ELSE,            NULL,       &Sync_EndCycle),
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
    MQTTProtCfg *config;
    MQTTProtPublish publisher;
    const char *errorStr;
};

RKH_SMA_CREATE(MQTTProt, mqttProt, 2, HCAL, &Client_Idle, init, NULL);
RKH_SMA_DEF_PTR(mqttProt);
RKH_SM_CONST_CREATE(syncRegion, 3, HCAL, &Sync_Idle, NULL, NULL);

/* ------------------------------- Constants ------------------------------- */
static const MQTTProtCfg configDft =
{
    60, 5, "publishing_cLient", 400, "date_time", 0
};

/* ---------------------------- Local data types --------------------------- */
typedef struct ConnRefusedEvt ConnRefusedEvt;
struct ConnRefusedEvt
{
    RKH_EVT_T evt;
    enum MQTTConnackReturnCode code;
};

/* ---------------------------- Global variables --------------------------- */
RKH_SM_T *MQTTProt_syncRegion;

/* ---------------------------- Local variables ---------------------------- */
static RKH_ROM_STATIC_EVENT(evWaitConnectToutObj, evWaitConnectTout);
static RKH_ROM_STATIC_EVENT(evWaitPublishToutObj, evWaitPublishTout);
static RKH_ROM_STATIC_EVENT(evWaitSyncToutObj, evWaitSyncTout);
static RKH_ROM_STATIC_EVENT(evRecvObj, evRecv);
static RKH_ROM_STATIC_EVENT(evActivateObj, evActivate);
static RKH_ROM_STATIC_EVENT(evDeactivateObj, evDeactivate);
static RKH_ROM_STATIC_EVENT(evConnAcceptedObj, evConnAccepted);
static RKH_ROM_STATIC_EVENT(evUnlockedObj, evUnlocked);
static RKH_ROM_STATIC_EVENT(evRestartObj, evRestart);
static SendEvt evSendObj;
static ConnRefusedEvt evConnRefusedObj;
static LocalSendAll localSend;
static LocalRecvAll localRecv;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
static rui16_t
pubDft(AppData *appMsg)
{
    static char application_message[128];
    Epoch timer;
    char timebuf[26];
    Time tm_info;

    if (appMsg != (AppData *)0)
    {
        timer = epoch_get();
        mk_date(timer, &tm_info);
        tm_info.tm_hour -= 3; /* GMT-3 (BsAs) */
        str_time(timebuf, &tm_info);
        snprintf(application_message, sizeof(application_message), 
                "The time is %s", timebuf);
        appMsg->data = (rui8_t *)application_message;
        appMsg->size = (rui16_t)strlen(application_message) + 1;
    }
    return 0;
}

static int
configClient(MQTTProt *const me, MQTTProtCfg *config)
{
    int result = 1;

    if (config->publishTime != 0 ||
        config->syncTime != 0 ||
        config->keepAlive != 0 ||
        config->topic != (const char *)0 ||
        config->clientId != (const char *)0)
    {
        result = 0;
        me->config = config;
    }
    else 
    {
        me->config = (MQTTProtCfg *)&configDft;
    }
    return result;
}

static void 
dispatch(RKH_SMA_T *me, void *arg)
{
    SyncRegion *region;
    
    region = &(RKH_DOWNCAST(MQTTProt, me)->itsSyncRegion);
    rkh_sm_dispatch((RKH_SM_T *)me, (RKH_EVT_T *)arg);
    rkh_sm_dispatch(RKH_UPCAST(RKH_SM_T, region), (RKH_EVT_T *)arg);
}

static void
connack_response_callback(enum MQTTConnackReturnCode return_code)
{
    MQTTProt *me;
    RKH_EVT_T *evt;

    me = RKH_DOWNCAST(MQTTProt, mqttProt);
    if (return_code == MQTT_CONNACK_ACCEPTED)
    {
        evt = RKH_UPCAST(RKH_EVT_T, &evConnAcceptedObj);
    }
    else
    {
        evConnRefusedObj.code = return_code;
        evt = RKH_UPCAST(RKH_EVT_T, &evConnRefusedObj);
    }
    RKH_SMA_POST_FIFO(RKH_UPCAST(RKH_SMA_T, me), evt, me);
}

/* ............................ Initial action ............................. */
static void
init(MQTTProt *const me, RKH_EVT_T *pe)
{
	(void)pe;

    RKH_TR_FWK_AO(me);
    RKH_TR_FWK_AO(MQTTProt_syncRegion);
    RKH_TR_FWK_QUEUE(&RKH_UPCAST(RKH_SMA_T, me)->equeue);
    RKH_TR_FWK_STATE(me, &Client_Idle);
    RKH_TR_FWK_STATE(me, &Sync_Idle);
    RKH_TR_FWK_STATE(me, &Sync_WaitSync);
    RKH_TR_FWK_STATE(me, &Sync_Receiving);
    RKH_TR_FWK_STATE(me, &Sync_EndCycle);
    RKH_TR_FWK_STATE(me, &Sync_Sending);
    RKH_TR_FWK_STATE(me, &Client_Idle);
    RKH_TR_FWK_STATE(me, &Client_TryConnect);
    RKH_TR_FWK_STATE(me, &Client_AwaitingAck);
    RKH_TR_FWK_STATE(me, &Client_WaitToPublish);
    RKH_TR_FWK_STATE(me, &Sync_Active);
    RKH_TR_FWK_STATE(me, &Client_Connected);
    RKH_TR_FWK_STATE(me, &Client_WaitToUse0);
    RKH_TR_FWK_STATE(me, &Client_WaitToUse1);
    RKH_TR_FWK_STATE(me, &Sync_C10);
    RKH_TR_FWK_STATE(me, &Sync_C12);
    RKH_TR_FWK_STATE(me, &Sync_C14);
    RKH_TR_FWK_STATE(me, &Sync_C25);
    RKH_TR_FWK_STATE(me, &Sync_C31);
    RKH_TR_FWK_STATE(me, &Sync_C32);
    RKH_TR_FWK_STATE(me, &Sync_C38);
    RKH_TR_FWK_STATE(me, &Client_C7);
    RKH_TR_FWK_STATE(me, &Client_C15);
    RKH_TR_FWK_STATE(me, &Client_C20);
    RKH_TR_FWK_SIG(evConnAccepted);
    RKH_TR_FWK_SIG(evConnRefused);
    RKH_TR_FWK_SIG(evActivate);
    RKH_TR_FWK_SIG(evWaitConnectTout);
    RKH_TR_FWK_SIG(evWaitPublishTout);
    RKH_TR_FWK_SIG(evWaitSyncTout);
    RKH_TR_FWK_SIG(evUnlocked);
    RKH_TR_FWK_SIG(RKH_COMPLETION_EVENT);
    RKH_FILTER_OFF_SMA(MQTTProt_syncRegion);

    RKH_SET_STATIC_EVENT(RKH_UPCAST(RKH_EVT_T, &evSendObj), evSend);
    RKH_SET_STATIC_EVENT(RKH_UPCAST(RKH_EVT_T, &evConnRefusedObj), 
                         evConnRefused);

    me->client.connack_response_callback = connack_response_callback;
    rkh_sm_init(RKH_UPCAST(RKH_SM_T, &me->itsSyncRegion));
}

/* ............................ Effect actions ............................. */
static void 
publish(MQTTProt *const me, RKH_EVT_T *pe)
{
    AppData appMsg;
    rui16_t pubTime;

    pubTime = (*me->publisher)(&appMsg);
    if (pubTime != 0)
    {
        me->config->publishTime = pubTime;
    }
    me->operRes = mqtt_publish(&me->client, 
                               me->config->topic, 
                               appMsg.data, 
                               appMsg.size, 
                               (me->config->qos << 1) & 0x06);
}

static void 
initRecvAll(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    mqtt_initRecvAll();
}

static void 
recvFail(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    localRecv.rv = MQTT_ERROR_SOCKET_ERROR;
    mqtt_recvFail(&realMe->client, &localRecv); /* an error occurred */
}

static void 
parseRecv(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;
    ReceivedEvt *evt;

    realMe = me->itsMQTTProt;
    evt = RKH_DOWNCAST(ReceivedEvt, pe);

    memcpy(realMe->client.recv_buffer.curr, evt->buf, evt->size);
    localRecv.rv = evt->size;
    mqtt_parseRecv(&realMe->client, &localRecv);
}

static void 
sendMsgFail(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    localSend.tmp = MQTT_ERROR_SOCKET_ERROR;
    mqtt_sendMsgFail(&realMe->client, &localSend);
}

static void 
setMsgState(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    mqtt_setMsgState(&realMe->client, &localSend);
}

static void 
parseError(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    mqtt_parseError(&realMe->client, &localRecv);
}

static void 
noConsumed(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    mqtt_noConsumed(&realMe->client, &localRecv);
}

static void 
cleanBuf(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    mqtt_cleanBuf(&realMe->client, &localRecv);
}

static void 
recvMsgError(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    mqtt_recvMsgError(&realMe->client, &localRecv);
}

static void 
initSendAll(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    mqtt_initSendAll(&realMe->client, &localSend);
}

static void 
initSendOk(SyncRegion *const me, RKH_EVT_T *pe)
{
    (void *)me;
    (void *)pe;
}

static void 
sendOneMsg(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    mqtt_sendOneMsg(&realMe->client, &localSend);
}

static void 
endSendAll(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    mqtt_endSendAll(&realMe->client);
}

static void 
nextSend(SyncRegion *const me, RKH_EVT_T *pe)
{
    mqtt_nextSend(&localSend);
}

static void 
handleRecvMsg(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    mqtt_handleRecvMsg(&realMe->client, &localRecv);
}

static void 
activateSync(MQTTProt *const me, RKH_EVT_T *pe)
{
    RKH_SMA_POST_FIFO(RKH_UPCAST(RKH_SMA_T, me), 
                      RKH_UPCAST(RKH_EVT_T, &evActivateObj), 
                      me);
}

static void 
releaseUse(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    RKH_SMA_POST_FIFO(RKH_UPCAST(RKH_SMA_T, realMe), 
                      RKH_UPCAST(RKH_EVT_T, &evUnlockedObj), 
                      me);
}

static void 
deactivateSync(MQTTProt *const me, RKH_EVT_T *pe)
{
    RKH_SMA_POST_FIFO(RKH_UPCAST(RKH_SMA_T, me), 
                      RKH_UPCAST(RKH_EVT_T, &evDeactivateObj), 
                      me);
}

static void 
reconnect(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    RKH_SMA_POST_FIFO(conMgr, &evRestartObj, realMe);
}

static void 
reconnectSync(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    RKH_SMA_POST_LIFO(RKH_UPCAST(RKH_SMA_T, realMe), 
                      RKH_UPCAST(RKH_EVT_T, &evDeactivateObj), 
                      realMe);
}

/* ............................. Entry actions ............................. */
static void 
enAwaitingAck(MQTTProt *const me, RKH_EVT_T *pe)
{
    RKH_TMR_INIT(&me->tryConnTmr, &evWaitConnectToutObj, NULL);
    RKH_TMR_ONESHOT(&me->tryConnTmr, RKH_UPCAST(RKH_SMA_T, me), 
                    RKH_TIME_SEC(120));
}

static void 
brokerConnect(MQTTProt *const me, RKH_EVT_T *pe)
{
    mqtt_init(&me->client, 0, me->sendbuf, sizeof(me->sendbuf), 
              me->recvbuf, sizeof(me->recvbuf), 0);
    me->operRes = mqtt_connect(&me->client, 
                               me->config->clientId, 
                               NULL, NULL, 0, NULL, NULL, 0, 
                               me->config->keepAlive);
    me->errorStr = mqtt_error_str(me->operRes);
}

static void 
enWaitSync(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    RKH_TMR_INIT(&me->syncTmr, &evWaitSyncToutObj, NULL);
    RKH_TMR_ONESHOT(&me->syncTmr, 
                    RKH_UPCAST(RKH_SMA_T, realMe), 
                    RKH_TIME_SEC(realMe->config->syncTime));
}

static void 
recvAll(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    RKH_SMA_POST_FIFO(conMgr, &evRecvObj, realMe);
}

static void 
sendAll(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = me->itsMQTTProt;
    evSendObj.size = localSend.msg->size;
    memcpy(evSendObj.buf, localSend.msg->start, localSend.msg->size);
    RKH_SMA_POST_FIFO(conMgr, RKH_UPCAST(RKH_EVT_T, &evSendObj), realMe);
}

static void 
enWaitToPublish(MQTTProt *const me, RKH_EVT_T *pe)
{
    RKH_TMR_INIT(&me->publishTmr, &evWaitPublishToutObj, NULL);
    RKH_TMR_ONESHOT(&me->publishTmr, RKH_UPCAST(RKH_SMA_T, me), 
                    RKH_TIME_SEC(me->config->publishTime));
}

/* ............................. Exit actions .............................. */
static void 
exAwaitingAck(MQTTProt *const me, RKH_EVT_T *pe)
{
    rkh_tmr_stop(&me->tryConnTmr);
}

static void 
exWaitToPublish(MQTTProt *const me, RKH_EVT_T *pe)
{
    rkh_tmr_stop(&me->publishTmr);
}

static void 
exWaitSync(SyncRegion *const me, RKH_EVT_T *pe)
{
    rkh_tmr_stop(&me->syncTmr);
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

static rbool_t 
isLocked(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;
    const RKH_ST_T *inState;

    realMe = RKH_DOWNCAST(MQTTProt, me);
    inState = ((RKH_SM_T *)&(realMe->itsSyncRegion))->state;
    return (rbool_t)(inState == (const RKH_ST_T *)&Sync_Receiving || 
                     inState == (const RKH_ST_T *)&Sync_Sending ||
                     inState == (const RKH_ST_T *)&Sync_EndCycle);
}

static rbool_t 
isReconnect(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    SyncRegion *realMe;

    realMe = RKH_DOWNCAST(SyncRegion, me);
    return mqtt_isReconnect(&realMe->itsMQTTProt->client);
}

/* ---------------------------- Global functions --------------------------- */
void
MQTTProt_ctor(MQTTProtCfg *config, MQTTProtPublish publisher)
{
    MQTTProt *me;

    me = RKH_DOWNCAST(MQTTProt, mqttProt);
    me->vtbl = rkhSmaVtbl;
    me->vtbl.task = dispatch;
    rkh_sma_ctor(RKH_UPCAST(RKH_SMA_T, me), &me->vtbl);

    me->itsSyncRegion.itsMQTTProt = me;
    RKH_SM_INIT((RKH_SM_T *)&(me->itsSyncRegion), syncRegion, 0, HCAL, 
                Sync_Idle, NULL, NULL);
    MQTTProt_syncRegion = (RKH_SM_T *)&(me->itsSyncRegion);
    configClient(me, config);
    me->publisher = (publisher != (MQTTProtPublish)0) ? publisher : pubDft;
}

/* ------------------------------ End of file ------------------------------ */
