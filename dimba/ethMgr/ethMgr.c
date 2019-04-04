/**
 *  \file       ethMgr.c
 *  \brief      Implementation of connection and protocol manager.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.15  DaBa  v1.0.01  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Bali#a      db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"
#include <string.h>
#include "ethMgr.h"
#include "mqttProt.h"
#include "dimbaCfg.h"
#include "signals.h"
#include "bsp.h"


/* ----------------------------- Local macros ------------------------------ */


/* ......................... Declares active object ........................ */
typedef struct EthMgr EthMgr;

/* ................... Declares states and pseudostates .................... */
RKH_DCLR_BASIC_STATE EthMgr_inactive, EthMgr_active;

/* ........................ Declares initial action ........................ */
static void init(EthMgr *const me, RKH_EVT_T *pe);

/* ........................ Declares effect actions ........................ */
static void open(EthMgr *const me, RKH_EVT_T *pe);
static void close(EthMgr *const me, RKH_EVT_T *pe);

/* ......................... Declares entry actions ........................ */
/* ......................... Declares exit actions ......................... */
/* ............................ Declares guards ............................ */

/* ........................ States and pseudostates ........................ */
RKH_CREATE_BASIC_STATE(EthMgr_inactive, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(EthMgr_inactive)
    RKH_TRREG(evOpen, NULL, open, &EthMgr_active),
RKH_END_TRANS_TABLE

RKH_CREATE_BASIC_STATE(EthMgr_active, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(EthMgr_active)
    RKH_TRREG(evClose, NULL, close, &EthMgr_inactive),
RKH_END_TRANS_TABLE

/* ............................. Active object ............................. */
struct EthMgr
{
    RKH_SMA_T ao;       /* base structure */
    RKH_TMR_T timer;    
};

RKH_SMA_CREATE(EthMgr, ethMgr, 2, HCAL, &EthMgr_inactive, init, NULL);
RKH_SMA_DEF_PTR(ethMgr);

/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
/*
 *  Declare and allocate the 'e_tout' event.
 *  The 'e_tout' event with TIMEOUT signal never changes, so it can be
 *  statically allocated just once by means of RKH_ROM_STATIC_EVENT() macro.
 */
static RKH_STATIC_EVENT(e_tout, evToutDelay);

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */

/* ............................ Initial action ............................. */
static void
init(EthMgr *const me, RKH_EVT_T *pe)
{
	(void)pe;

    RKH_TR_FWK_AO(me);

    RKH_TR_FWK_TIMER(&me->timer);
    RKH_TR_FWK_STATE(me, &EthMgr_inactive);
    RKH_TR_FWK_STATE(me, &EthMgr_active);
    RKH_TR_FWK_SIG(evOpen);
    RKH_TR_FWK_SIG(evClose);

    RKH_TMR_INIT(&me->timer, &e_tout, NULL);
}

/* ............................ Effect actions ............................. */
static void
open(EthMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    (void)me;

    bsp_eth_open();
}

static void
close(EthMgr *const me, RKH_EVT_T *pe)
{
    (void)pe;
    (void)me;

}

/* ............................. Entry actions ............................. */
/* ............................. Exit actions ............................. */
/* ................................ Guards ................................. */
/* ---------------------------- Global functions --------------------------- */
/* ------------------------------ End of file ------------------------------ */
