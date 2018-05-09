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
#include "modcmd.h"
#include "modmgr.h"
#include "conmgr.h"
#include "ssp.h"
#include <string.h>

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
typedef struct CmdTbl CmdTbl;
struct CmdTbl
{
    ModCmd sync;
    /*...*/
};

/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static ModMgrEvt evtCmd;
static const CmdTbl cmdTbl =
{
    {RKH_INIT_STATIC_EVT(evModCmd), 
     "at\r\n", 
     conMgr, 
     RKH_TIME_MS(300), RKH_TIME_MS(100)}
    /*{...
       ...}*/
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
  	ssp_init(&sim900Parser, &root);
    return &doSearch;
}

void 
ModCmd_sync(void)
{
    strcpy(evtCmd.cmd, cmdTbl.sync.fmt);
    evtCmd.args = cmdTbl.sync;
    RKH_SMA_POST_FIFO(modMgr, RKH_UPCAST(RKH_EVT_T, &evtCmd), 0);
}

/* ------------------------------ End of file ------------------------------ */
