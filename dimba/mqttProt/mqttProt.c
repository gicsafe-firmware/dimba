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
RKH_DCLR_BASIC_STATE MQTTProt_inactive;

/* ........................ Declares initial action ........................ */
static void initialization(MQTTProt *const me, RKH_EVT_T *pe);

/* ........................ Declares effect actions ........................ */
static void dataReceived(MQTTProt *const me, RKH_EVT_T *pe);

/* ......................... Declares entry actions ........................ */
/* ......................... Declares exit actions ......................... */
/* ............................ Declares guards ............................ */
/* ........................ States and pseudostates ........................ */
RKH_CREATE_BASIC_STATE(MQTTProt_inactive, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(MQTTProt_inactive)
    RKH_TRINT(evReceived, NULL, dataReceived),
RKH_END_TRANS_TABLE

/* ............................. Active object ............................. */
struct MQTTProt
{
    RKH_SMA_T ao;           /* base structure */
};

RKH_SMA_CREATE(MQTTProt, mqttProt, 2, HCAL, &MQTTProt_inactive, initialization, 
               NULL);
RKH_SMA_DEF_PTR(mqttProt);

/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ............................ Initial action ............................. */
static void
initialization(MQTTProt *const me, RKH_EVT_T *pe)
{
	(void)pe;

    RKH_TR_FWK_AO(me);
    RKH_TR_FWK_QUEUE(&RKH_UPCAST(RKH_SMA_T, me)->equeue);
    RKH_TR_FWK_STATE(me, &MQTTProt_inactive);
}

/* ............................ Effect actions ............................. */

#include <stdio.h>

static void
dataReceived(MQTTProt *const me, RKH_EVT_T *pe)
{
	(void)me;

    printf("\r\n MQTT received[%d]: %s\r\n", 
            RKH_UPCAST(ReceivedEvt,pe)->size,
            RKH_UPCAST(ReceivedEvt,pe)->buf);
}

/* ............................. Entry actions ............................. */
/* ............................. Exit actions .............................. */
/* ................................ Guards ................................. */
/* ---------------------------- Global functions --------------------------- */
/* ------------------------------ End of file ------------------------------ */
