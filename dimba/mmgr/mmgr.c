/**
 *  \file       mmgr.c
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
#include "mmgr.h"
#include "bsp.h"

/* ----------------------------- Local macros ------------------------------ */
#define PONG_WAIT_TIME          RKH_TIME_SEC(2)
#define PING_DELAY_TIME         RKH_TIME_SEC(1)

/* ......................... Declares active object ........................ */
typedef struct Mmgr Mmgr;

/* ................... Declares states and pseudostates .................... */
RKH_DCLR_BASIC_STATE idle;

/* ........................ Declares initial action ........................ */
static void init(Mmgr *const me, RKH_EVT_T *pe);

/* ........................ Declares effect actions ........................ */
static void close(Mmgr *const me, RKH_EVT_T *pe);
static void send_ping(Mmgr *const me, RKH_EVT_T *pe);
static void timeout(Mmgr *const me, RKH_EVT_T *pe);
static void rcv_pong(Mmgr *const me, RKH_EVT_T *pe);

/* ......................... Declares entry actions ........................ */
/* ......................... Declares exit actions ......................... */
/* ............................ Declares guards ............................ */
/* ........................ States and pseudostates ........................ */
RKH_CREATE_BASIC_STATE(idle, NULL, NULL, RKH_ROOT, NULL);
RKH_CREATE_TRANS_TABLE(idle)
    RKH_TRINT(evTerminate,  NULL, NULL),
RKH_END_TRANS_TABLE

/* ............................. Active object ............................. */
struct Mmgr
{
    RKH_SMA_T sma;      /* base structure */
    RKH_TMR_T timer;    /* which is responsible for toggling the LED */
                        /* posting the TIMEOUT signal event to active object */
                        /* 'mmgr' */
};

RKH_SMA_CREATE(Mmgr, mmgr, 0, HCAL, &idle, init, NULL);
RKH_SMA_DEF_PTR(mmgr);

/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
/*
 *  Declare and allocate the 'e_tout' event.
 *  The 'e_tout' event with TIMEOUT signal never changes, so it can be
 *  statically allocated just once by means of RKH_ROM_STATIC_EVENT() macro.
 */
static RKH_ROM_STATIC_EVENT(e_tout, evTimeout);

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ............................ Initial action ............................. */
static void
init(Mmgr *const me, RKH_EVT_T *pe)
{
	(void)pe;

    RKH_TR_FWK_AO(me);
    RKH_TR_FWK_QUEUE(&RKH_UPCAST(RKH_SMA_T, me)->equeue);
    RKH_TR_FWK_STATE(me, &idle);
    RKH_TR_FWK_TIMER(&me->timer);
    RKH_TR_FWK_SIG(evTerminate);
	RKH_TR_FWK_SIG(evTimeout);

    RKH_TMR_INIT(&me->timer, &e_tout, NULL);

    bsp_serial_open();
}

/* ............................ Effect actions ............................. */
/* ............................. Entry actions ............................. */
/* ............................. Exit actions .............................. */
/* ................................ Guards ................................. */
/* ---------------------------- Global functions --------------------------- */
/* ------------------------------ End of file ------------------------------ */
