/**
 *  \file       main.c
 *  \brief      Example application.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.17  LeFr  Initial version
 *  2018.06.17  Daba  Publisher
 *  2019.03.19  Daba  Ethernet Connection Manager
 *  2019.03.29  Daba  FreeRtos integration
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci  lf@vortexmakes.com
 *  DaBa  Dario Bali#a       db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include <string.h>
#include <stdio.h>
#include "rkh.h"
#include "rkhfwk_dynevt.h"
#include "bsp.h"
#include "signals.h"
#include "mqttProt.h"
#include "dimbaCfg.h"
#include "ethMgr.h"
#include "conmgr.h"
#include "modmgr.h"
#include "mTime.h"
#include "epoch.h"
#include "anSampler.h"
#include "ioChgDet.h"
#include "CirBuffer.h"
#include "publisher.h"

/* ----------------------------- Local macros ------------------------------ */
#define MQTTPROT_QSTO_SIZE  16
#define ETHMGR_QSTO_SIZE    8
#define CONMGR_QSTO_SIZE    8
#define MODMGR_QSTO_SIZE    8

#define SIZEOF_EP0STO       16
#define SIZEOF_EP0_BLOCK    sizeof(RKH_EVT_T)
#define SIZEOF_EP1STO       128
#define SIZEOF_EP1_BLOCK    sizeof(ModCmd)
#define SIZEOF_EP2STO       512
#define SIZEOF_EP2_BLOCK    sizeof(ModMgrEvt)

/* ------------------------------- Constants ------------------------------- */
#define MODMGR_STK_SIZE         512
#define CONMGR_STK_SIZE         512
#define MQTTPROT_STK_SIZE       512
#define ETHMGR_STK_SIZE         512

/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static RKH_THREAD_STK_TYPE ModMgrStack[MODMGR_STK_SIZE];
static RKH_EVT_T *ModMgr_qsto[MODMGR_QSTO_SIZE];

static RKH_THREAD_STK_TYPE ConMgrStack[CONMGR_STK_SIZE];
static RKH_EVT_T *ConMgr_qsto[CONMGR_QSTO_SIZE];

static RKH_THREAD_STK_TYPE EthMGRStack[ETHMGR_STK_SIZE];
static RKH_EVT_T *EthMgr_qsto[CONMGR_QSTO_SIZE];

static RKH_THREAD_STK_TYPE MQTTProtStack[MQTTPROT_STK_SIZE];
static RKH_EVT_T *MQTTProt_qsto[MQTTPROT_QSTO_SIZE];

static rui8_t evPool0Sto[SIZEOF_EP0STO],
              evPool1Sto[SIZEOF_EP1STO],
              evPool2Sto[SIZEOF_EP2STO];

static RKH_ROM_STATIC_EVENT(e_Open, evOpen);
static MQTTProtCfg mqttProtCfg;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
static void
setupTraceFilters(void)
{
    RKH_FILTER_ON_GROUP(RKH_TRC_ALL_GROUPS);
    RKH_FILTER_ON_EVENT(RKH_TRC_ALL_EVENTS);
    RKH_FILTER_OFF_EVENT(MODCMD_USR_TRACE);
    RKH_FILTER_OFF_EVENT(ETH_USR_TRACE);
    RKH_FILTER_OFF_GROUP_ALL_EVENTS(RKH_TG_USR);
    RKH_FILTER_OFF_EVENT(RKH_TE_TMR_TOUT);
    RKH_FILTER_OFF_EVENT(RKH_TE_SM_STATE);
    /* RKH_FILTER_OFF_EVENT(RKH_TE_SMA_FIFO); */
    /* RKH_FILTER_OFF_EVENT(RKH_TE_SMA_LIFO); */
    /* RKH_FILTER_OFF_EVENT(RKH_TE_SM_TS_STATE); */
    RKH_FILTER_OFF_EVENT(RKH_TE_SM_DCH);
    RKH_FILTER_OFF_SMA(modMgr);
    RKH_FILTER_OFF_SMA(conMgr);
    RKH_FILTER_OFF_SMA(ethMgr);
    RKH_FILTER_OFF_SMA(mqttProt);
    RKH_FILTER_OFF_ALL_SIGNALS();
}

/* ---------------------------- Global functions --------------------------- */
void
dimbaCfg_clientId(char *pid)
{
    strcpy(mqttProtCfg.clientId, pid);
}

void
dimbaCfg_topic(char *t)
{
    sprintf(mqttProtCfg.topic, "/dimba/%s", t);
}

void
rkh_startupTask(void *pvParameter)
{
    setupTraceFilters();

    RKH_TRC_OPEN();

    rkh_dynEvt_init();
    rkh_fwk_registerEvtPool(evPool0Sto, SIZEOF_EP0STO, SIZEOF_EP0_BLOCK);
    rkh_fwk_registerEvtPool(evPool1Sto, SIZEOF_EP1STO, SIZEOF_EP1_BLOCK);
    rkh_fwk_registerEvtPool(evPool2Sto, SIZEOF_EP2STO, SIZEOF_EP2_BLOCK);

    mqttProtCfg.publishTime = MAX_PUBLISH_TIME;
    mqttProtCfg.syncTime = 4;
    mqttProtCfg.keepAlive = 400;
    mqttProtCfg.qos = 1;
    strcpy(mqttProtCfg.clientId, "");
    strcpy(mqttProtCfg.topic, "");
    MQTTProt_ctor(&mqttProtCfg, publishDimba);

    RKH_SMA_ACTIVATE(conMgr, ConMgr_qsto, CONMGR_QSTO_SIZE,
                     ConMgrStack, CONMGR_STK_SIZE);
    RKH_SMA_ACTIVATE(ethMgr, EthMgr_qsto, ETHMGR_QSTO_SIZE,
                     EthMGRStack, ETHMGR_STK_SIZE);
    RKH_SMA_ACTIVATE(modMgr, ModMgr_qsto, MODMGR_QSTO_SIZE,
                     ModMgrStack, MODMGR_STK_SIZE);
    RKH_SMA_ACTIVATE(mqttProt, MQTTProt_qsto, MQTTPROT_QSTO_SIZE,
                     MQTTProtStack, MQTTPROT_STK_SIZE);

    RKH_SMA_POST_FIFO(conMgr, &e_Open, 0);
    RKH_SMA_POST_FIFO(ethMgr, &e_Open, 0);

    rkh_fwk_enter();

    vTaskDelete(NULL);
}

int
main(int argc, char *argv[])
{
    bsp_init(argc, argv);

    anSampler_init();
    IOChgDet_init();
    epoch_init();
    mTime_init();

    rkh_fwk_init();

    vTaskStartScheduler();

    return 0;
}

/* ------------------------------ End of file ------------------------------ */
