/**
 *  \file       modcmdSim900.c
 *  \brief      Implementation of module command abstraction.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.07  LeFr  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  2018.05.07  LeFr  v1.0.00  Initial version
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

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
typedef struct CmdTbl CmdTbl;
struct CmdTbl
{
    ModCmd sync;
    /* other string commands */
};

/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static SSP sim900Parser;
static RKH_SMA_T *sender;

static const CmdTbl cmdTbl =
{
    {RKH_INIT_STATIC_EVT(evCmd), 
     "at\r\n", 
     &conMgr, 
     RKH_TIME_MS(300), RKH_TIME_MS(100)}
};

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
static void
doSearch(unsigned char c)
{
    ssp_doSearch(&sim900Parser, c);
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
    ModMgrEvt *evtCmd;

    sender = *cmdTbl.sync.aoDest;
    evtCmd = RKH_ALLOC_EVT(ModMgrEvt, 0, sender);
    strcpy(evtCmd->cmd, cmdTbl.sync.fmt);
    evtCmd->args = cmdTbl.sync;

    RKH_SMA_POST_FIFO(modMgr, RKH_UPCAST(RKH_EVT_T, evtCmd), sender);
}

/* ------------------------------ End of file ------------------------------ */
