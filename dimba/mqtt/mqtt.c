/**
 *  \file       mqtt.c
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
#include "mqtt.h"

/* ----------------------------- Local macros ------------------------------ */
/* ......................... Declares ModMgr_active object ........................ */
typedef struct Mqtt Mqtt;

/* ................... Declares states and pseudostates .................... */
RKH_DCLR_BASIC_STATE Mqtt_inactive

/* ........................ Declares initial action ........................ */
static void initialization(ModMgr *const me, RKH_EVT_T *pe);

/* ........................ Declares effect actions ........................ */
/* ......................... Declares entry actions ........................ */
/* ......................... Declares exit actions ......................... */
/* ............................ Declares guards ............................ */
/* ........................ States and pseudostates ........................ */
RKH_CREATE_BASIC_STATE(Mqtt_inactive, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(Mqtt_inactive)
RKH_END_TRANS_TABLE

/* ............................. Active object ............................. */
struct Mqtt
{
    RKH_SMA_T ao;           /* base structure */
};

RKH_SMA_CREATE(Mqtt, mqtt, 0, HCAL, &Mqtt_inactive, initialization, 
               NULL);
RKH_SMA_DEF_PTR(mqtt);

/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ............................ Initial action ............................. */
static void
initialization(ModMgr *const me, RKH_EVT_T *pe)
{
	(void)pe;

    RKH_TR_FWK_AO(me);
    RKH_TR_FWK_QUEUE(&RKH_UPCAST(RKH_SMA_T, me)->equeue);
    RKH_TR_FWK_STATE(me, &Mqtt_inactive);
}

/* ............................ Effect actions ............................. */
/* ............................. Entry actions ............................. */
/* ............................. Exit actions .............................. */
/* ................................ Guards ................................. */
/* ---------------------------- Global functions --------------------------- */
/* ------------------------------ End of file ------------------------------ */
