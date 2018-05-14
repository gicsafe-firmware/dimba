/**
 *  \file       sim900parser.c
 *  \brief      ...
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.09  LeFr  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  2018.05.09  LeFr  v1.0.00  Initial version
 *  2018.05.14  DaBa  v1.0.01  Sync response
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "sim900parser.h"
#include "signals.h"
#include "modmgr.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static rui8_t sim900parser;

/* ----------------------- Local function prototypes ----------------------- */
static void cmd_ok(unsigned char data);

/* ---------------------------- Local functions ---------------------------- */

SSP_CREATE_NORMAL_NODE(rootCmdParser);
SSP_CREATE_BR_TABLE(rootCmdParser)
	SSPBR("OK\r\n", cmd_ok, &rootCmdParser),
SSP_END_BR_TABLE


static void
cmd_ok(unsigned char data)
{
    ModMgrResp *p;

    (void)data;
    
    p = RKH_ALLOC_EVT( ModMgrResp, evResponse, &sim900parser );
    p->fwdEvt = evSync;
    RKH_SMA_POST_FIFO( modMgr, RKH_UPCAST(RKH_EVT_T, p), &sim900parser );
}


/* ---------------------------- Global functions --------------------------- */
/* ------------------------------ End of file ------------------------------ */
