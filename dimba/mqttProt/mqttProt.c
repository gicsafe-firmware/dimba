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
RKH_DCLR_BASIC_STATE Sync_Idle, Sync_WaitSync, Sync_Receiving, 
                     Sync_EndCycle, Sync_Sending,
                     Client_Idle, Client_ConnRefused, Client_TryConnect,
                     Client_WaitToConnect, Client_AwaitingAck,
                     Client_WaitToPublish, Client_WaitToUse0,
                     Client_WaitToUse1;
RKH_DCLR_COMP_STATE Sync_Active, Client_Connected;
RKH_DCLR_CHOICE_STATE Sync_C10, Sync_C12, Sync_C14, Sync_C25, Sync_C31,
                      Sync_C32, Sync_C38,
                      Client_C7, Client_C15, Client_C20;

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
static void releaseUse(MQTTProt *const me, RKH_EVT_T *pe);

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
static rbool_t isLocked(const RKH_SM_T *me, RKH_EVT_T *pe);

/* ........................ States and pseudostates ........................ */
RKH_CREATE_BASIC_STATE(Sync_Idle, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(Sync_Idle)
    RKH_TRREG(evActivate, NULL, NULL, &Sync_Active),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(Sync_Active, NULL, NULL, RKH_ROOT, 
                             &Sync_WaitSync, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(Sync_Active)
    RKH_TRREG(evDeactivate, NULL, releaseUse, &Sync_Idle),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Sync_WaitSync, enWaitSync, exWaitSync, &Sync_Active, 
                       NULL);
RKH_CREATE_TRANS_TABLE(Sync_WaitSync)
    RKH_TRREG(evWaitSyncTout, NULL, initRecvAll, &Sync_Receiving),
RKH_END_TRANS_TABLE

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

RKH_CREATE_BASIC_STATE(Client_ConnRefused, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(Client_ConnRefused)
    RKH_TRREG(evNetDisconnected, NULL, NULL, &Client_Idle),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(Client_Connected, NULL, NULL, RKH_ROOT, 
                             &Client_C15, NULL,
                             RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(Client_Connected)
    RKH_TRREG(evConnRefused, NULL, NULL, &Client_ConnRefused),
    RKH_TRREG(evNetDisconnected, NULL, NULL, &Client_Idle),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_TryConnect, brokerConnect, NULL, 
                       &Client_Connected, NULL);
RKH_CREATE_TRANS_TABLE(Client_TryConnect)
    RKH_TRCOMPLETION(NULL, NULL, &Client_C7),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_WaitToConnect, enWaitToConnect, exWaitToConnect, 
                       &Client_Connected, NULL);
RKH_CREATE_TRANS_TABLE(Client_WaitToConnect)
    RKH_TRREG(evWaitConnectTout, NULL, NULL, &Client_Connected),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(Client_AwaitingAck, NULL, NULL, &Client_Connected, NULL);
RKH_CREATE_TRANS_TABLE(Client_AwaitingAck)
    RKH_TRREG(evConnAccepted, NULL, NULL, &Client_WaitToPublish),
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
	RKH_BRANCH(ELSE,        NULL, &Client_WaitToConnect),
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
};

RKH_SMA_CREATE(MQTTProt, mqttProt, 2, HCAL, &Client_Idle, init, NULL);
RKH_SMA_DEF_PTR(mqttProt);
RKH_SM_CONST_CREATE(syncRegion, 3, HCAL, &Sync_Idle, NULL, NULL);

/* ------------------------------- Constants ------------------------------- */
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
static RKH_ROM_STATIC_EVENT(evConnAcceptedObj, evConnAccepted);
static RKH_ROM_STATIC_EVENT(evUnlockedObj, evUnlocked);
static SendEvt evSendObj;
static ConnRefusedEvt evConnRefusedObj;
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
    RKH_TR_FWK_STATE(me, &Client_ConnRefused);
    RKH_TR_FWK_STATE(me, &Client_TryConnect);
    RKH_TR_FWK_STATE(me, &Client_WaitToConnect);
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
    RKH_FILTER_OFF_SMA(MQTTProt_syncRegion);

    RKH_SET_STATIC_EVENT(RKH_UPCAST(RKH_EVT_T, &evSendObj), evSend);
    RKH_SET_STATIC_EVENT(RKH_UPCAST(RKH_EVT_T, &evConnRefusedObj), 
                         evConnRefused);

    me->client.connack_response_callback = connack_response_callback;
    mqtt_init(&me->client, 0, me->sendbuf, sizeof(me->sendbuf), 
              me->recvbuf, sizeof(me->recvbuf), 0);
}

/* ............................ Effect actions ............................. */

#include "ioChgDet.h"
#include "anSampler.h"

#define NUM_AN_SAMPLES_GET  10
#define NUM_DI_SAMPLES_GET  8

char application_message[512];

static
int
getMessage(void)
{
    AnSampleSet anSet;
    IOChg ioChg[NUM_DI_SAMPLES_GET];
    int n, l, i, j;
    char *p;

    n = anSampler_getSet(&anSet, NUM_AN_SAMPLES_GET);
   
    p = application_message;
    l = 0;

    if(n > 0)
    {
        for(i=0; i < NUM_AN_SIGNALS; ++i)
        {
            l += sprintf(p + l, "ts:%u, AN[%d], %d[sec]", 
                    anSet.timeStamp, AN_SAMPLING_RATE_SEC, i);

            for(j=0; j<n; ++j)
            {
                l += sprintf(p + l,", %d.%02d",
                               (char)((anSet.anSignal[i][j] & 0xFF00) >> 8),
                               (char)(anSet.anSignal[i][j] & 0x00FF));
            }

            l += sprintf(p + l, "\r\n");
        }
    }

    n = IOChgDet_get(ioChg, NUM_DI_SAMPLES_GET);
    
    for(i=0; i < n; ++i)
    {
        l += sprintf(p + l, "ts:%u, DI[%d]:%d\r\n", ioChg[i].timeStamp,
                                                    ioChg[i].signalId,
                                                    ioChg[i].signalValue );
    }

    return l;
}

static void 
publish(MQTTProt *const me, RKH_EVT_T *pe)
{
    const char *topic;
    int size;

    /* Get digital input changes and analog input samples */
    /* Format a payload to send */
    /* mqtt_publish(...); */

    topic = "dimba/test";

    size = getMessage();
    
    me->operRes = mqtt_publish(&me->client, 
                               topic, 
                               application_message, 
                               size,
                               MQTT_PUBLISH_QOS_1);
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
    MQTTProt *realMe;

    realMe = ((SyncRegion *)me)->itsMQTTProt;
    mqtt_endSendAll(&realMe->client);
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
    RKH_SMA_POST_FIFO(RKH_UPCAST(RKH_SMA_T, me), 
                      RKH_UPCAST(RKH_EVT_T, &evActivateObj), 
                      me);
}

static void 
releaseUse(MQTTProt *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

    realMe = ((SyncRegion *)me)->itsMQTTProt;
    RKH_SMA_POST_FIFO(RKH_UPCAST(RKH_SMA_T, realMe), 
                      RKH_UPCAST(RKH_EVT_T, &evUnlockedObj), 
                      me);
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
    evSendObj.size = localSend.msg->size;
    memcpy(evSendObj.buf, localSend.msg->start, localSend.msg->size);
    RKH_SMA_POST_FIFO(conMgr, RKH_UPCAST(RKH_EVT_T, &evSendObj), realMe);
}

static void 
enWaitToPublish(MQTTProt *const me, RKH_EVT_T *pe)
{
    RKH_TMR_INIT(&me->publishTmr, &evWaitPublishToutObj, NULL);
    RKH_TMR_ONESHOT(&me->publishTmr, RKH_UPCAST(RKH_SMA_T, me), PUBLISH_TIME);
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

/* ---------------------------- Global functions --------------------------- */
void
MQTTProt_ctor(void)
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
}

/* ------------------------------ End of file ------------------------------ */
