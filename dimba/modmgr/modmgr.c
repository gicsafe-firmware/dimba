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
 *  DaBa  Dario Bali�a db@vortexmakes.com
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"
#include "signals.h"
#include "modmgr.h"
#include "conmgr.h"
#include "bsp.h"

/* ----------------------------- Local macros ------------------------------ */
#define SIZEOF_QDEFER   4
/* ......................... Declares ModMgr_active object ........................ */
typedef struct ModMgr ModMgr;

/* ................... Declares states and pseudostates .................... */
RKH_DCLR_BASIC_STATE ModMgr_inactive, ModMgr_idle, ModMgr_inProgress, 
                     ModMgr_waitInterCmdDelay;
RKH_DCLR_COMP_STATE ModMgr_active;
RKH_DCLR_COND_STATE ModMgr_chkInterCmdDelay, ModMgr_chkDataCmd;

/* ........................ Declares initial action ........................ */
static void initialization(ModMgr *const me, RKH_EVT_T *pe);

/* ........................ Declares effect actions ........................ */
static void defer(ModMgr *const me, RKH_EVT_T *pe);
static void notifyURC(ModMgr *const me, RKH_EVT_T *pe);
static void sendCmd(ModMgr *const me, RKH_EVT_T *pe);
static void sendData(ModMgr *const me, RKH_EVT_T *pe);
static void sendResponse(ModMgr *const me, RKH_EVT_T *pe);
static void noResponse(ModMgr *const me, RKH_EVT_T *pe);
static void startDelay(ModMgr *const me, RKH_EVT_T *pe);
static void moreCmd(ModMgr *const me, RKH_EVT_T *pe);

/* ......................... Declares entry actions ........................ */
static void setupResponse(ModMgr *const me);

/* ......................... Declares exit actions ......................... */
/* ............................ Declares guards ............................ */
rbool_t isInterCmdTime(ModMgr *const me, RKH_EVT_T *pe);
rbool_t isDataCmd(ModMgr *const me, RKH_EVT_T *pe);

/* ........................ States and pseudostates ........................ */
RKH_CREATE_BASIC_STATE(ModMgr_inactive, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(ModMgr_inactive)
    RKH_TRREG(evOpen, NULL, NULL, &ModMgr_active),
RKH_END_TRANS_TABLE

RKH_CREATE_COMP_REGION_STATE(ModMgr_active, NULL, NULL, RKH_ROOT, 
                              &ModMgr_idle, NULL,
                              RKH_NO_HISTORY, NULL, NULL, NULL, NULL);
RKH_CREATE_TRANS_TABLE(ModMgr_active)
    RKH_TRINT(evCmd, NULL, defer),
    RKH_TRINT(evURC, NULL, notifyURC),
    RKH_TRINT(evClose,  NULL, defer),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(ModMgr_idle, NULL, NULL, &ModMgr_active, NULL);
RKH_CREATE_TRANS_TABLE(ModMgr_idle)
    RKH_TRREG(evCmd, NULL, NULL, &ModMgr_chkDataCmd),
RKH_END_TRANS_TABLE

RKH_CREATE_COND_STATE(ModMgr_chkDataCmd);
RKH_CREATE_BRANCH_TABLE(ModMgr_chkDataCmd)
    RKH_BRANCH(isDataCmd,   sendData,  &ModMgr_inProgress),
    RKH_BRANCH(ELSE,        sendCmd,   &ModMgr_inProgress),
RKH_END_BRANCH_TABLE

RKH_CREATE_BASIC_STATE(ModMgr_inProgress, setupResponse, NULL,
												&ModMgr_active, NULL);
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
    RKH_SMA_T ao;           /* base structure */
    RKH_TMR_T timer;        /* which is responsible for wait responce and
                               intercmd delay */
    ModMgrEvt *pCmd;        /* stores a reference to current command */
};

RKH_SMA_CREATE(ModMgr, modMgr, 0, HCAL, &ModMgr_inactive, initialization, 
               NULL);
RKH_SMA_DEF_PTR(modMgr);

/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static RKH_STATIC_EVENT(e_tout, evToutWaitResponse);
static RKH_STATIC_EVENT(e_noResp, evNoResponse);
static RKH_QUEUE_T qDefer;
static ModMgrEvt *qDefer_sto[SIZEOF_QDEFER];

/* ----------------------- Local function prototypes ----------------------- */
static void forwardModMgrEvt(RKH_SMA_T *ao, RKH_EVT_T *pe);

/* ---------------------------- Local functions ---------------------------- */
static void
forwardModMgrEvt(RKH_SMA_T *ao, RKH_EVT_T *pe)
{
    ModMgrResp *presp;

    presp = (ModMgrResp *)(pe);

    presp->evt.e = presp->fwdEvt;

    RKH_SMA_POST_FIFO(ao, RKH_UPCAST(RKH_EVT_T, presp), modMgr);
}

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
    RKH_TR_FWK_SIG(evToutWaitResponse);
    RKH_TR_FWK_SIG(evTimeout);
    RKH_TR_FWK_SIG(evResponse);
    RKH_TR_FWK_SIG(evNoResponse);
	RKH_TR_FWK_TUSR(MODCMD_USR_TRACE);

    rkh_queue_init(&qDefer, (const void **)qDefer_sto, SIZEOF_QDEFER, 
                CV(0));
 
    RKH_TMR_INIT(&me->timer, &e_tout, NULL);

    bsp_serial_open(GSM_PORT);
}

/* ............................ Effect actions ............................. */
static void
defer(ModMgr *const me, RKH_EVT_T *pe)
{
    (void)me;

    rkh_sma_defer(&qDefer, pe);
}

static void
notifyURC(ModMgr *const me, RKH_EVT_T *pe)
{
    (void)me;

    forwardModMgrEvt(conMgr, pe);
}

static void
sendCmd(ModMgr *const me, RKH_EVT_T *pe)
{
    RKH_FWK_RSV( pe );
    me->pCmd = RKH_UPCAST(ModMgrEvt, pe);

    bsp_serial_puts(GSM_PORT, me->pCmd->cmd);

}

static void
sendData(ModMgr *const me, RKH_EVT_T *pe)
{
    RKH_FWK_RSV( pe );
    me->pCmd = RKH_UPCAST(ModMgrEvt, pe);

    bsp_serial_putnchar(GSM_PORT, me->pCmd->data, me->pCmd->nData);
#ifdef _SEND_WITH_TERMINATOR
    bsp_serial_puts(GSM_PORT, ModCmd_endOfXmitStr());
#endif
}

static void
sendResponse(ModMgr *const me, RKH_EVT_T *pe)
{
    forwardModMgrEvt((RKH_SMA_T *)*(me->pCmd->args.aoDest), pe);
}

static void
noResponse(ModMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    
    RKH_SMA_POST_FIFO((RKH_SMA_T *)*(me->pCmd->args.aoDest), 
                            &e_noResp, modMgr);

    RKH_FWK_GC(RKH_CAST(RKH_EVT_T, me->pCmd), me);
}

static void
startDelay(ModMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;

    RKH_SET_STATIC_EVENT(&e_tout, evTimeout);
    RKH_TMR_ONESHOT(&me->timer, RKH_UPCAST(RKH_SMA_T, me), 
                    me->pCmd->args.interCmdTime);
}

static void
moreCmd(ModMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;

    RKH_FWK_GC(RKH_CAST(RKH_EVT_T, me->pCmd), me);

    rkh_sma_recall((RKH_SMA_T *)me, &qDefer);
}

/* ............................. Entry actions ............................. */
static void
setupResponse(ModMgr *const me)
{
    RKH_SET_STATIC_EVENT(&e_tout, evToutWaitResponse);
    RKH_TMR_ONESHOT(&me->timer, RKH_UPCAST(RKH_SMA_T, me), 
                    me->pCmd->args.waitResponseTime);
    
    RKH_TRC_USR_BEGIN(MODCMD_USR_TRACE)
        RKH_TUSR_STR(me->pCmd->cmd);
    RKH_TRC_USR_END();
}

/* ............................. Exit actions .............................. */
/* ................................ Guards ................................. */
rbool_t
isInterCmdTime(ModMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    
    return (me->pCmd->args.interCmdTime != 0) ? RKH_TRUE : RKH_FALSE;
}

rbool_t
isDataCmd(ModMgr *const me, RKH_EVT_T *pe)
{
    (void)me;
    
    return (RKH_UPCAST(ModMgrEvt, pe)->data != NULL) ? RKH_TRUE : RKH_FALSE;
}

/* ---------------------------- Global functions --------------------------- */
/* ------------------------------ End of file ------------------------------ */
