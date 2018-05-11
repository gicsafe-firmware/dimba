/**
 *  \file       modmgr.c
 *  \brief      AT-command Module Manager.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.02  DaBa  v1.0.00  Initial version
 *  2018.05.11  LeFr  v1.0.01
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Baliña db@vortexmakes.com
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"
#include "signals.h"
#include "modmgr.h"
#include "bsp.h"

/* ----------------------------- Local macros ------------------------------ */
/* ......................... Declares ModMgr_active object ........................ */
typedef struct ModMgr ModMgr;

/* ................... Declares states and pseudostates .................... */
RKH_DCLR_BASIC_STATE ModMgr_inactive, ModMgr_idle, ModMgr_inProgress, 
                     ModMgr_waitInterCmdDelay;
RKH_DCLR_COMP_STATE ModMgr_active;
RKH_DCLR_COND_STATE ModMgr_chkInterCmdDelay;

/* ........................ Declares initial action ........................ */
static void initialization(ModMgr *const me, RKH_EVT_T *pe);

/* ........................ Declares effect actions ........................ */
static void defer(ModMgr *const me, RKH_EVT_T *pe);
static void notifyURC(ModMgr *const me, RKH_EVT_T *pe);
static void sendCmd(ModMgr *const me, RKH_EVT_T *pe);
static void sendResponse(ModMgr *const me, RKH_EVT_T *pe);
static void noResponse(ModMgr *const me, RKH_EVT_T *pe);
static void startDelay(ModMgr *const me, RKH_EVT_T *pe);
static void moreCmd(ModMgr *const me, RKH_EVT_T *pe);

/* ......................... Declares entry actions ........................ */
/* ......................... Declares exit actions ......................... */
/* ............................ Declares guards ............................ */
rbool_t isInterCmdTime(const RKH_SM_T *me, RKH_EVT_T *pe);

/* ........................ States and pseudostates ........................ */
RKH_CREATE_BASIC_STATE(ModMgr_inactive, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(ModMgr_inactive)
    RKH_TRREG(evOpen, NULL, NULL, &ModMgr_active),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(ModMgr_active, NULL, NULL, RKH_ROOT, &ModMgr_idle, NULL,
                              RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(ModMgr_active)
    RKH_TRINT(evCmd, NULL, defer),
    RKH_TRINT(evURC, NULL, notifyURC),
    RKH_TRINT(evClose,  NULL, defer),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ModMgr_idle, NULL, NULL, &ModMgr_active, NULL);
RKH_CREATE_TRANS_TABLE(ModMgr_idle)
    RKH_TRREG(evCmd, NULL, sendCmd, &ModMgr_inProgress),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ModMgr_inProgress, NULL, NULL, &ModMgr_active, NULL);
RKH_CREATE_TRANS_TABLE(ModMgr_inProgress)
    RKH_TRREG(evResponse, NULL, sendResponse, &ModMgr_chkInterCmdDelay),
    RKH_TRREG(evToutWaitResponse, NULL, noResponse, &ModMgr_idle),
RKH_END_TRANS_TABLE

RKH_CREATE_COND_STATE(ModMgr_chkInterCmdDelay);
RKH_CREATE_BRANCH_TABLE(ModMgr_chkInterCmdDelay)
    RKH_BRANCH(isInterCmdTime, startDelay,  &ModMgr_waitInterCmdDelay),
    RKH_BRANCH(ELSE,           moreCmd,   &ModMgr_idle),
RKH_END_BRANCH_TABLE

RKH_CREATE_BASIC_STATE(ModMgr_waitInterCmdDelay, NULL, NULL, 
                       &ModMgr_active, NULL);
RKH_CREATE_TRANS_TABLE(ModMgr_waitInterCmdDelay)
    RKH_TRREG(evTimeout, NULL, moreCmd, &ModMgr_idle),
RKH_END_TRANS_TABLE

/* ............................. Active object ............................. */
struct ModMgr
{
    RKH_SMA_T ao;       /* base structure */
    RKH_TMR_T timer;    /* which is responsible for intercmd delay */
};

RKH_SMA_CREATE(ModMgr, modMgr, 0, HCAL, &ModMgr_inactive, initialization, 
               NULL);
RKH_SMA_DEF_PTR(modMgr);

/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static RKH_ROM_STATIC_EVENT(e_tout, evTimeout);
static RKH_QUEUE_T queueReq;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ............................ Initial action ............................. */
static void
initialization(ModMgr *const me, RKH_EVT_T *pe)
{
	(void)pe;

    RKH_TR_FWK_AO(me);
    RKH_TR_FWK_QUEUE(&RKH_UPCAST(RKH_SMA_T, me)->equeue);
    RKH_TR_FWK_STATE(me, &ModMgr_inactive);
    RKH_TR_FWK_STATE(me, &ModMgr_active);
    RKH_TR_FWK_STATE(me, &ModMgr_idle);
    RKH_TR_FWK_STATE(me, &ModMgr_inProgress);
    RKH_TR_FWK_STATE(me, &ModMgr_chkInterCmdDelay);
    RKH_TR_FWK_STATE(me, &ModMgr_waitInterCmdDelay);
    RKH_TR_FWK_TIMER(&me->timer);
    RKH_TR_FWK_SIG(evTerminate);
	RKH_TR_FWK_SIG(evTimeout);

    RKH_TMR_INIT(&me->timer, &e_tout, NULL);

    bsp_serial_open();
}

/* ............................ Effect actions ............................. */
static void
defer(ModMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;
}

static void
notifyURC(ModMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;
}

static void
sendCmd(ModMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;
}

static void
sendResponse(ModMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;
}

static void
noResponse(ModMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;
}

static void
startDelay(ModMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;
}

static void
moreCmd(ModMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;
}

/* ............................. Entry actions ............................. */
/* ............................. Exit actions .............................. */
/* ................................ Guards ................................. */
rbool_t
isInterCmdTime(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    (void)me;
    (void)pe;
    return RKH_TRUE;
}

/* ---------------------------- Global functions --------------------------- */
/* ------------------------------ End of file ------------------------------ */
