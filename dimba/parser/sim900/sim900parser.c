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
SSP_DCLR_NORMAL_NODE pin;
SSP_DCLR_TRN_NODE reg;

/* ----------------------- Local function prototypes ----------------------- */
static void cmd_ok(unsigned char pos);
static void sim_pin(unsigned char pos);
static void sim_error(unsigned char pos);
static void sim_ready(unsigned char pos);
static void registered(unsigned char pos);
static void no_registered(unsigned char pos);

/* ---------------------------- Local functions ---------------------------- */

SSP_CREATE_NORMAL_NODE(rootCmdParser);
SSP_CREATE_BR_TABLE(rootCmdParser)
	SSPBR("OK\r\n", cmd_ok, &rootCmdParser),
	SSPBR("+CPIN: ", NULL, &pin),
	SSPBR("+CREG: ", NULL, &reg),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(pin);
SSP_CREATE_BR_TABLE(pin)
	SSPBR("SIM PIN\r\n",    sim_pin,   &rootCmdParser),
	SSPBR("SIM ERROR\r\n",  sim_error, &rootCmdParser),
	SSPBR("READY\r\n",      sim_ready, &rootCmdParser),
	SSPBR("\r\n",           NULL,      &rootCmdParser),
SSP_END_BR_TABLE

SSP_CREATE_TRN_NODE(reg, NULL);
SSP_CREATE_BR_TABLE(reg)
	SSPBR(",0\r\n",    no_registered, &rootCmdParser),
	SSPBR(",1\r\n",    registered,    &rootCmdParser),
	SSPBR(",2\r\n",    no_registered, &rootCmdParser),
	SSPBR(",3\r\n",    no_registered, &rootCmdParser),
	SSPBR(",4\r\n",    no_registered, &rootCmdParser),
	SSPBR(",5\r\n",    registered,    &rootCmdParser),
	SSPBR("\r\n",      NULL,          &rootCmdParser),
SSP_END_BR_TABLE

static void
sendModResp_noArgs(RKH_SIG_T sig)
{
    ModMgrResp *p;

    p = RKH_ALLOC_EVT( ModMgrResp, evResponse, &sim900parser );
    p->fwdEvt = sig;
    RKH_SMA_POST_FIFO( modMgr, RKH_UPCAST(RKH_EVT_T, p), &sim900parser );
}

static void
cmd_ok(unsigned char pos)
{
    (void)pos;
    
    sendModResp_noArgs(evSync);
}

static void
sim_pin(unsigned char pos)
{
    (void)pos;
    
    sendModResp_noArgs(evSimPin);
}

static void
sim_error(unsigned char pos)
{
    (void)pos;
    
    sendModResp_noArgs(evSimError);
}

static void
sim_ready(unsigned char pos)
{
    (void)pos;
    
    sendModResp_noArgs(evSimReady);
}

static void
registered(unsigned char pos)
{
    (void)pos;
    
    sendModResp_noArgs(evReg);
}

static void
no_registered(unsigned char pos)
{
    (void)pos;
    
    sendModResp_noArgs(evNoReg);
}

/* ---------------------------- Global functions --------------------------- */
/* ------------------------------ End of file ------------------------------ */
