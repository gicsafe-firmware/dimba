/**
 *  \file       modcmdSim900.c
 *  \brief      Implementation of module command abstraction.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.07  LeFr  v1.0.00  Initial version
 *  2018.05.15  DaBa  v1.0.01  sync
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 *  DaBa  Darío Baliña      db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"
#include "rkhfwk_dynevt.h"
#include "modcmd.h"
#include "modmgr.h"
#include "conmgr.h"
#include "sim900parser.h"
#include "signals.h"
#include <string.h>
#include <stdio.h>

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
typedef struct CmdTbl CmdTbl;
struct CmdTbl
{
    ModCmd sync;
    ModCmd initStr;
    ModCmd getPinStatus;
    ModCmd setPin;
    ModCmd getRegStatus;
    ModCmd enableUnsolicitedReg;
    /* other string commands */
};

/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static SSP sim900Parser;
static RKH_SMA_T *sender;

static const CmdTbl cmdTbl =
{
    {RKH_INIT_STATIC_EVT(evCmd), 
     "AT\r\n", 
     &conMgr, 
     RKH_TIME_MS(300), RKH_TIME_MS(100)},

    {RKH_INIT_STATIC_EVT(evCmd), 
     "ATE1\r\n", 
     &conMgr, 
     RKH_TIME_MS(300), RKH_TIME_MS(100)},

    {RKH_INIT_STATIC_EVT(evCmd), 
     "AT+CPIN?\r\n", 
     &conMgr, 
     RKH_TIME_MS(1500), RKH_TIME_MS(100)},

    {RKH_INIT_STATIC_EVT(evCmd), 
     "AT+CPIN=%d\r\n", 
     &conMgr, 
     RKH_TIME_MS(300), RKH_TIME_MS(100)},

    {RKH_INIT_STATIC_EVT(evCmd), 
     "AT+CREG?\r\n", 
     &conMgr, 
     RKH_TIME_MS(300), RKH_TIME_MS(100)},

    {RKH_INIT_STATIC_EVT(evCmd), 
     "AT+CREG=1\r\n", 
     &conMgr, 
     RKH_TIME_MS(300), RKH_TIME_MS(100)},
};

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
static void
doSearch(unsigned char c)
{
    ssp_doSearch(&sim900Parser, c);
}

static void
sendModCmd_noArgs(const ModCmd *p)
{
    ModMgrEvt *evtCmd;

    sender = *p->aoDest;
    evtCmd = RKH_ALLOC_EVT(ModMgrEvt, evCmd, sender);

    strcpy(evtCmd->cmd, p->fmt);

    evtCmd->args.fmt = p->fmt;
    evtCmd->args.aoDest = p->aoDest;
    evtCmd->args.waitResponseTime = p->waitResponseTime;
    evtCmd->args.interCmdTime = p->interCmdTime;

    RKH_SMA_POST_FIFO(modMgr, RKH_UPCAST(RKH_EVT_T, evtCmd), sender);
}

static void
sendModCmd_rui16(const ModCmd *p, rui16_t arg)
{
    ModMgrEvt *evtCmd;

    sender = *p->aoDest;
    evtCmd = RKH_ALLOC_EVT(ModMgrEvt, evCmd, sender);

    sprintf(evtCmd->cmd, p->fmt, arg);

    evtCmd->args.fmt = p->fmt;
    evtCmd->args.aoDest = p->aoDest;
    evtCmd->args.waitResponseTime = p->waitResponseTime;
    evtCmd->args.interCmdTime = p->interCmdTime;

    RKH_SMA_POST_FIFO(modMgr, RKH_UPCAST(RKH_EVT_T, evtCmd), sender);
}

/* ---------------------------- Global functions --------------------------- */
ModCmdRcvHandler
ModCmd_init(void)
{
  	ssp_init(&sim900Parser, &rootCmdParser);
    return &doSearch;
}

void 
ModCmd_sync(void)
{
    sendModCmd_noArgs(&cmdTbl.sync);
}

void 
ModCmd_initStr(void)
{
    sendModCmd_noArgs(&cmdTbl.initStr);
}

void 
ModCmd_getPinStatus(void)
{
    sendModCmd_noArgs(&cmdTbl.getPinStatus);
}

void 
ModCmd_setPin(rui16_t pin)
{
    sendModCmd_rui16(&cmdTbl.setPin, pin);
}

void 
ModCmd_getRegStatus(void)
{
    sendModCmd_noArgs(&cmdTbl.getRegStatus);
}

void 
ModCmd_UnsolicitedRegStatus(void)
{
    sendModCmd_noArgs(&cmdTbl.enableUnsolicitedReg);
}

/* ------------------------------ End of file ------------------------------ */
