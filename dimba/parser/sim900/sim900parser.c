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
SSP_DCLR_NORMAL_NODE at, initStr, at_plus_c, 
                     at_plus_cpin, at_plus_creg, pinStatus, wpinSet, pinSet,
                     plus_c, plus_creg;

static rui8_t isURC;

/* ----------------------- Local function prototypes ----------------------- */
static void cmd_ok(unsigned char pos);
static void sim_pin(unsigned char pos);
static void sim_error(unsigned char pos);
static void sim_ready(unsigned char pos);
static void isURC_set(unsigned char pos);
static void registered(unsigned char pos);
static void registered(unsigned char pos);
static void no_registered(unsigned char pos);

/* ---------------------------- Local functions ---------------------------- */

SSP_CREATE_NORMAL_NODE(rootCmdParser);
SSP_CREATE_BR_TABLE(rootCmdParser)
	SSPBR("AT", NULL,     &at),
	SSPBR("+C", NULL,     &plus_c),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(at);
SSP_CREATE_BR_TABLE(at)
	SSPBR("E",  NULL,       &initStr),
	SSPBR("+C", NULL,       &at_plus_c),
	SSPBR("OK\r\n", cmd_ok, &rootCmdParser),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(initStr);
SSP_CREATE_BR_TABLE(initStr)
	SSPBR("OK\r\n", cmd_ok, &rootCmdParser),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(at_plus_c);
SSP_CREATE_BR_TABLE(at_plus_c)
	SSPBR("PIN",            NULL,   &at_plus_cpin),
	SSPBR("REG?\r\n\r\n",   NULL,   &at_plus_creg),
	SSPBR("\r\n",   NULL,  &rootCmdParser),
SSP_END_BR_TABLE

/* --------------------------------------------------------------- */
/* ---------------------------- AT+CPIN --------------------------- */
SSP_CREATE_NORMAL_NODE(at_plus_cpin);
SSP_CREATE_BR_TABLE(at_plus_cpin)
	SSPBR("?\r\n\r\n",    NULL,  &pinStatus),
	SSPBR("=",            NULL,  &wpinSet),
	SSPBR("\r\n",         NULL,  &rootCmdParser),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(pinStatus);
SSP_CREATE_BR_TABLE(pinStatus)
	SSPBR("SIM PIN", sim_pin,   &rootCmdParser),
	SSPBR("ERROR",   sim_error, &rootCmdParser),
	SSPBR("READY",   sim_ready, &rootCmdParser),
	SSPBR("\r\n",    NULL,      &rootCmdParser),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(wpinSet);
SSP_CREATE_BR_TABLE(wpinSet)
	SSPBR("\r\n\r\n",   NULL,   &pinSet),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(pinSet);
SSP_CREATE_BR_TABLE(pinSet)
	SSPBR("OK\r\n", cmd_ok, &rootCmdParser),
	SSPBR("\r\n",   NULL,   &rootCmdParser),
SSP_END_BR_TABLE

/* --------------------------------------------------------------- */
/* --------------------------- AT+CREG --------------------------- */
SSP_CREATE_NORMAL_NODE(at_plus_creg);
SSP_CREATE_BR_TABLE(at_plus_creg)
	SSPBR("1,",      NULL,  &plus_creg),
	SSPBR("\r\n",    NULL,  &rootCmdParser),
SSP_END_BR_TABLE

/* --------------------------------------------------------------- */
/* -------------------- Unsolicited +CREG ... -------------------- */
SSP_CREATE_NORMAL_NODE(plus_c);
SSP_CREATE_BR_TABLE(plus_c)
	SSPBR("REG:",   isURC_set, &plus_creg),
	SSPBR("\r\n",   NULL,      &rootCmdParser),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(plus_creg);
SSP_CREATE_BR_TABLE(plus_creg)
	SSPBR("0",     no_registered, &rootCmdParser),
	SSPBR("1",     registered,    &rootCmdParser),
	SSPBR("2",     no_registered, &rootCmdParser),
	SSPBR("3",     no_registered, &rootCmdParser),
	SSPBR("4",     no_registered, &rootCmdParser),
	SSPBR("5",     registered,    &rootCmdParser),
	SSPBR("\r\n",  NULL,          &rootCmdParser),
SSP_END_BR_TABLE

/* --------------------------------------------------------------- */
static void
sendModResp_noArgs(RKH_SIG_T sig)
{
    ModMgrResp *p;
    RKH_SIG_T ModMgrSignal = evResponse;
       
    if(isURC)
    {
        isURC = 0;
        ModMgrSignal = evURC;
    }

    p = RKH_ALLOC_EVT( ModMgrResp, ModMgrSignal, &sim900parser );
    p->fwdEvt = sig;
    RKH_SMA_POST_FIFO( modMgr, RKH_UPCAST(RKH_EVT_T, p), &sim900parser );
}

static void
isURC_set(unsigned char pos)
{
    (void)pos;

    isURC = 1;
}

static void
cmd_ok(unsigned char pos)
{
    (void)pos;
    
    sendModResp_noArgs(evOk);
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
void
parser_init(void)
{
   RKH_TR_FWK_ACTOR(&sim900parser, "sim900parser");
}

/* ------------------------------ End of file ------------------------------ */
