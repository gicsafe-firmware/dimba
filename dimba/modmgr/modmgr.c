/**
 *  \file       modmgr.c
 *  \brief      AT-command Module Manager.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.02  DaBa  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Baliña db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"
#include "dimbaevt.h"
#include "modmgr.h"
#include "bsp.h"

/* ----------------------------- Local macros ------------------------------ */
/* ......................... Declares active object ........................ */
typedef struct ModMgr ModMgr;

/* ................... Declares states and pseudostates .................... */
RKH_DCLR_BASIC_STATE inactive, idle, inProgress, waitInterCmd;
RKH_DCLR_COMP_STATE active;
RKH_DCLR_COND_STATE chkInterCmdDelay;

/* ........................ Declares initial action ........................ */
static void init(ModMgr *const me, RKH_EVT_T *pe);

/* ........................ Declares effect actions ........................ */
static void eventDefer(ModMgr *const me, RKH_EVT_T *pe);
static void urcForward(ModMgr *const me, RKH_EVT_T *pe);
static void cmdSend(ModMgr *const me, RKH_EVT_T *pe);
static void respForward(ModMgr *const me, RKH_EVT_T *pe);
static void interCmdStart(ModMgr *const me, RKH_EVT_T *pe);
static void cmdRecall(ModMgr *const me, RKH_EVT_T *pe);

/* ......................... Declares entry actions ........................ */
/* ......................... Declares exit actions ......................... */
/* ............................ Declares guards ............................ */
rbool_t interCmdDelay(const RKH_SM_T *sma, RKH_EVT_T *pe);

/* ........................ States and pseudostates ........................ */
RKH_CREATE_BASIC_STATE(inactive, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(inactive)
    RKH_TRREG(evOpen, NULL, NULL, &active),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(active, NULL, NULL, RKH_ROOT, &idle, NULL,
                              RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(active)
    RKH_TRINT(evModCmd, NULL, eventDefer),
    RKH_TRINT(evModUrc, NULL, urcForward),
    RKH_TRINT(evClose,  NULL, eventDefer),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(idle, NULL, NULL, &active, NULL);
RKH_CREATE_TRANS_TABLE(idle)
    RKH_TRREG(evModCmd, NULL, cmdSend, &inProgress),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(inProgress, NULL, NULL, &active, NULL);
RKH_CREATE_TRANS_TABLE(inProgress)
RKH_TRREG(evModResp, NULL, respForward, &chkInterCmdDelay),
RKH_END_TRANS_TABLE

RKH_CREATE_COND_STATE(chkInterCmdDelay);
RKH_CREATE_BRANCH_TABLE(chkInterCmdDelay)
    RKH_BRANCH(interCmdDelay,  interCmdStart,   &waitInterCmd),
    RKH_BRANCH(ELSE,           cmdRecall,       &idle),
RKH_END_BRANCH_TABLE

RKH_CREATE_BASIC_STATE(waitInterCmd, NULL, NULL, &active, NULL);
RKH_CREATE_TRANS_TABLE(waitInterCmd)
    RKH_TRREG(evTimeout, NULL, cmdRecall, &idle),
RKH_END_TRANS_TABLE

/* ............................. Active object ............................. */
struct ModMgr
{
    RKH_SMA_T ao;       /* base structure */
    RKH_TMR_T timer;    /* which is responsible for intercmd delay */
};

RKH_SMA_CREATE(ModMgr, modmgr, 0, HCAL, &inactive, init, NULL);
RKH_SMA_DEF_PTR(modmgr);

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
init(ModMgr *const me, RKH_EVT_T *pe)
{
	(void)pe;

    RKH_TR_FWK_AO(me);
    RKH_TR_FWK_QUEUE(&RKH_UPCAST(RKH_SMA_T, me)->equeue);
    RKH_TR_FWK_STATE(me, &inactive);
    RKH_TR_FWK_STATE(me, &active);
    RKH_TR_FWK_STATE(me, &idle);
    RKH_TR_FWK_STATE(me, &inProgress);
    RKH_TR_FWK_STATE(me, &chkInterCmdDelay);
    RKH_TR_FWK_STATE(me, &waitInterCmd);
    RKH_TR_FWK_TIMER(&me->timer);
    RKH_TR_FWK_SIG(evTerminate);
	RKH_TR_FWK_SIG(evTimeout);

    RKH_TMR_INIT(&me->timer, &e_tout, NULL);

    bsp_serial_open();
}

/* ............................ Effect actions ............................. */
static void
eventDefer(ModMgr *const me, RKH_EVT_T *pe)
{

}

static void
urcForward(ModMgr *const me, RKH_EVT_T *pe)
{

}

static void
cmdSend(ModMgr *const me, RKH_EVT_T *pe)
{

}

static void
respForward(ModMgr *const me, RKH_EVT_T *pe)
{

}

static void
interCmdStart(ModMgr *const me, RKH_EVT_T *pe)
{

}

static void
cmdRecall(ModMgr *const me, RKH_EVT_T *pe)
{

}

/* ............................. Entry actions ............................. */
/* ............................. Exit actions .............................. */
/* ................................ Guards ................................. */
rbool_t
interCmdDelay(const RKH_SM_T *sma, RKH_EVT_T *pe)
{
    return RKH_TRUE;
}

/* ---------------------------- Global functions --------------------------- */
/* ------------------------------ End of file ------------------------------ */
