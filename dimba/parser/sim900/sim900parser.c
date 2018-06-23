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
#include "conmgr.h"
#include <stdlib.h>

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
#define END_OF_RECV_STR     "\r\nOK\r\n"
#define YEAR2K              2000
#define LTBUFF_SIZE         5

/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static rui8_t sim900parser;
SSP_DCLR_NORMAL_NODE at, waitOK, at_plus, at_plus_c, at_plus_ci, 
                     at_plus_cip, at_plus_cips, 
                     at_plus_cipsta, at_plus_ciprxget,
                     at_plus_ciprxget_2, at_plus_ciprxget_2_wdata,
                     at_plus_cipstatus, at_plus_cipstatus_ip,
                     at_plus_cipstatus_sta, 
                     at_plus_cipstatus_c, at_plus_cipstatus_connect,
                     at_plus_cipstart,
                     at_plus_cipclose,
                     at_plus_cipsend, at_plus_cipsending, at_plus_cipsent,
                     at_plus_cpin, at_plus_creg, pinStatus, wpinSet, pinSet,
                     plus_c, plus_creg, at_plus_cipstatus, at_plus_cifsr,
                     netClockSync,
                     at_plus_cclk, cclk_end;

SSP_DCLR_TRN_NODE at_plus_ciprxget_data, cclk_year, cclk_month, cclk_day,
                  cclk_hour, cclk_min, at_plus_gsn;

static rui8_t isURC;

static unsigned char *prx;
static ReceivedEvt *precv;

static LocalTimeEvt localTimeEvt;
static Time *lTime;
static char ltbuf[LTBUFF_SIZE];
static char *plt;

static ImeiEvt imeiEvt;
static char *pImei;

/* ----------------------- Local function prototypes ----------------------- */
static void cmd_ok(unsigned char pos);
static void cmd_error(unsigned char pos);
static void sim_pin(unsigned char pos);
static void sim_error(unsigned char pos);
static void sim_ready(unsigned char pos);
static void isURC_set(unsigned char pos);
static void registered(unsigned char pos);
static void registered(unsigned char pos);
static void no_registered(unsigned char pos);
static void ipInitial(unsigned char pos);
static void ipStart(unsigned char pos);
static void ipStatus(unsigned char pos);
static void ipGprsAct(unsigned char pos);
static void ipDone(unsigned char pos);
static void connecting(unsigned char pos);
static void closed(unsigned char pos);
static void connected(unsigned char pos);
static void disconnected(unsigned char pos);
static void data_init(unsigned char pos);
static void data_collect(unsigned char c);
static void data_ready(unsigned char pos);
static void netClock_rcv(unsigned char pos);
static void lTimeInit(unsigned char pos);
static void yearCollect(unsigned char c);

#define monthCollect    yearCollect
#define dayCollect      yearCollect
#define hourCollect     yearCollect
#define minCollect      yearCollect

static void yearGet(unsigned char pos);
static void monthGet(unsigned char pos);
static void dayGet(unsigned char pos);
static void hourGet(unsigned char pos);
static void minGet(unsigned char pos);
static void lTimeGet(unsigned char pos);
static void imeiInit(unsigned char pos);
static void imeiCollect(unsigned char c);
static void imeiSet(unsigned char pos);

/* ---------------------------- Local functions ---------------------------- */

SSP_CREATE_NORMAL_NODE(rootCmdParser);
SSP_CREATE_BR_TABLE(rootCmdParser)
	SSPBR("STATE",      NULL,     &rootCmdParser),
	SSPBR("CONNECT OK", NULL,     &rootCmdParser),
	SSPBR("AT",         NULL,     &at),
	SSPBR("+C",         NULL,     &plus_c),
	SSPBR("*PSUTTZ",    isURC_set, &netClockSync),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(at);
SSP_CREATE_BR_TABLE(at)
	SSPBR("E",  NULL,       &waitOK),
	SSPBR("+", NULL,       &at_plus),
	SSPBR("OK\r\n", cmd_ok, &rootCmdParser),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(at_plus);
SSP_CREATE_BR_TABLE(at_plus)
	SSPBR("GSN\r\n\r\n", imeiInit,  &at_plus_gsn),
	SSPBR("C",           NULL,      &at_plus_c),
	SSPBR("OK\r\n",     cmd_ok,    &rootCmdParser),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(at_plus_c);
SSP_CREATE_BR_TABLE(at_plus_c)
	SSPBR("PIN",            NULL,   &at_plus_cpin),
	SSPBR("REG?\r\n\r\n",   NULL,   &at_plus_creg),
	SSPBR("STT=",           NULL,   &waitOK),
	SSPBR("I",              NULL,   &at_plus_ci),
	SSPBR("LTS=1",          NULL,   &waitOK),
	SSPBR("CLK?",           NULL,   &at_plus_cclk),
	SSPBR("\r\n",   NULL,  &rootCmdParser),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(at_plus_ci);
SSP_CREATE_BR_TABLE(at_plus_ci)
	SSPBR("FSR\r\n",      NULL,  &at_plus_cifsr),
	SSPBR("ICR\r\n",      NULL,  &waitOK),
	SSPBR("P",            NULL,  &at_plus_cip),
	SSPBR("\r\n",         NULL,  &rootCmdParser),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(at_plus_cip);
SSP_CREATE_BR_TABLE(at_plus_cip)
	SSPBR("S",            NULL,  &at_plus_cips),
	SSPBR("CLOSE",        NULL,  &at_plus_cipclose),
	SSPBR("RXGET",        NULL,  &at_plus_ciprxget),
	SSPBR("\r\n",         NULL,  &rootCmdParser),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(at_plus_cips);
SSP_CREATE_BR_TABLE(at_plus_cips)
	SSPBR("TA",          NULL,  &at_plus_cipsta),
	SSPBR("END",         NULL,  &at_plus_cipsend),
	SSPBR("HUT",         NULL,  &waitOK),
	SSPBR("\r\n",        NULL,  &rootCmdParser),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(at_plus_cipsta);
SSP_CREATE_BR_TABLE(at_plus_cipsta)
	SSPBR("TUS\r\n",      NULL,  &at_plus_cipstatus),
	SSPBR("RT=",          NULL,  &at_plus_cipstart),
	SSPBR("\r\n",         NULL,  &rootCmdParser),
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
/* --------------------------- AT+CCCLK --------------------------- */
SSP_CREATE_NORMAL_NODE(at_plus_cclk);
SSP_CREATE_BR_TABLE(at_plus_cclk)
	SSPBR("+CCLK: \"",  lTimeInit,  &cclk_year),
SSP_END_BR_TABLE

SSP_CREATE_TRN_NODE(cclk_year, yearCollect);
SSP_CREATE_BR_TABLE(cclk_year)
	SSPBR("/",      yearGet,  &cclk_month),
SSP_END_BR_TABLE

SSP_CREATE_TRN_NODE(cclk_month, monthCollect);
SSP_CREATE_BR_TABLE(cclk_month)
	SSPBR("/",      monthGet,  &cclk_day),
SSP_END_BR_TABLE

SSP_CREATE_TRN_NODE(cclk_day, dayCollect);
SSP_CREATE_BR_TABLE(cclk_day)
	SSPBR(",",      dayGet,  &cclk_hour),
SSP_END_BR_TABLE

SSP_CREATE_TRN_NODE(cclk_hour, hourCollect);
SSP_CREATE_BR_TABLE(cclk_hour)
	SSPBR(":",      hourGet,  &cclk_min),
SSP_END_BR_TABLE

SSP_CREATE_TRN_NODE(cclk_min, minCollect);
SSP_CREATE_BR_TABLE(cclk_min)
	SSPBR(":",      minGet,  &cclk_end),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(cclk_end);
SSP_CREATE_BR_TABLE(cclk_end)
	SSPBR("OK\r\n", lTimeGet,  &rootCmdParser),
SSP_END_BR_TABLE

/* --------------------------------------------------------------- */
/* ------------------------ AT+CIPRXGET -------------------------- */
SSP_CREATE_NORMAL_NODE(at_plus_ciprxget);
SSP_CREATE_BR_TABLE(at_plus_ciprxget)
	SSPBR("1\r\n\r\nOK\r\n", cmd_ok,  &rootCmdParser),
	SSPBR("2,",              NULL,    &at_plus_ciprxget_2),
	SSPBR("\r\n",            NULL,    &rootCmdParser),
SSP_END_BR_TABLE

/* --------------------------------------------------------------- */
/* ------------------------ AT+CIPRXGET=2 ------------------------ */
SSP_CREATE_NORMAL_NODE(at_plus_ciprxget_2);
SSP_CREATE_BR_TABLE(at_plus_ciprxget_2)
	SSPBR("+CIPRXGET: 2",      NULL,   &at_plus_ciprxget_2_wdata),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(at_plus_ciprxget_2_wdata);
SSP_CREATE_BR_TABLE(at_plus_ciprxget_2_wdata)
	SSPBR("\r\n",      data_init,   &at_plus_ciprxget_data),
SSP_END_BR_TABLE

SSP_CREATE_TRN_NODE(at_plus_ciprxget_data, data_collect);
SSP_CREATE_BR_TABLE(at_plus_ciprxget_data)
	SSPBR(END_OF_RECV_STR, data_ready,   &rootCmdParser),
SSP_END_BR_TABLE

/* --------------------------------------------------------------- */
/* ------------------------ AT+CIPSTART -------------------------- */
SSP_CREATE_NORMAL_NODE(at_plus_cipstart);
SSP_CREATE_BR_TABLE(at_plus_cipstart)
	SSPBR("OK",     cmd_ok,    &rootCmdParser),
	SSPBR("ERROR",  cmd_error, &rootCmdParser),
SSP_END_BR_TABLE

/* --------------------------------------------------------------- */
/* ------------------------ AT+CIPSTATUS ------------------------- */
SSP_CREATE_NORMAL_NODE(at_plus_cipstatus);
SSP_CREATE_BR_TABLE(at_plus_cipstatus)
	SSPBR("IP ",    NULL,  &at_plus_cipstatus_ip),
	SSPBR("C",      NULL,  &at_plus_cipstatus_c),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(at_plus_cipstatus_ip);
SSP_CREATE_BR_TABLE(at_plus_cipstatus_ip)
	SSPBR("INITIAL",    ipInitial,  &rootCmdParser),
	SSPBR("STA",        NULL,       &at_plus_cipstatus_sta),
	SSPBR("GPRSACT",    ipGprsAct,  &rootCmdParser),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(at_plus_cipstatus_sta);
SSP_CREATE_BR_TABLE(at_plus_cipstatus_sta)
	SSPBR("TUS",     ipStatus,   &rootCmdParser),
	SSPBR("RT",      ipStart,    &rootCmdParser),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(at_plus_cipstatus_c);
SSP_CREATE_BR_TABLE(at_plus_cipstatus_c)
	SSPBR("LOSED",     closed,     &rootCmdParser),
	SSPBR("ONNECT",    NULL,       &at_plus_cipstatus_connect),
SSP_END_BR_TABLE

SSP_CREATE_NORMAL_NODE(at_plus_cipstatus_connect);
SSP_CREATE_BR_TABLE(at_plus_cipstatus_connect)
	SSPBR("ING",     connecting,     &rootCmdParser),
	SSPBR(" OK",     connected,      &rootCmdParser),
SSP_END_BR_TABLE

/* --------------------------------------------------------------- */
/* ------------------------- AT+CIPSEND -------------------------- */
SSP_CREATE_NORMAL_NODE(at_plus_cipsend);
SSP_CREATE_BR_TABLE(at_plus_cipsend)
#ifdef _SEND_WITH_TERMINATOR
	SSPBR(">", cmd_ok,  &at_plus_cipsending),
#else
	SSPBR(">", cmd_ok,  &at_plus_cipsent),
#endif
SSP_END_BR_TABLE

#ifdef _SEND_WITH_TERMINATOR
SSP_CREATE_NORMAL_NODE(at_plus_cipsending);
SSP_CREATE_BR_TABLE(at_plus_cipsending)
	SSPBR("\x1A", NULL,  &at_plus_cipsent),
SSP_END_BR_TABLE
#endif

SSP_CREATE_NORMAL_NODE(at_plus_cipsent);
SSP_CREATE_BR_TABLE(at_plus_cipsent)
	SSPBR("SEND OK\r\n", cmd_ok,  &rootCmdParser),
SSP_END_BR_TABLE

/* --------------------------------------------------------------- */
/* ------------------------- AT+CIPCLOSE ------------------------- */
SSP_CREATE_NORMAL_NODE(at_plus_cipclose);
SSP_CREATE_BR_TABLE(at_plus_cipclose)
	SSPBR("CLOSE OK", disconnected,  &rootCmdParser),
SSP_END_BR_TABLE

/* --------------------------------------------------------------- */
/* -------------------------- AT+CIFSR --------------------------- */
SSP_CREATE_NORMAL_NODE(at_plus_cifsr);
SSP_CREATE_BR_TABLE(at_plus_cifsr)
	SSPBR(".",       ipDone,      &rootCmdParser),
SSP_END_BR_TABLE

/* --------------------------------------------------------------- */
/* --------------------------- initStr --------------------------- */
/* --------------------------- AT+CSTT --------------------------- */
/* --------------------------- AT+CIICT -------------------------- */
SSP_CREATE_NORMAL_NODE(waitOK);
SSP_CREATE_BR_TABLE(waitOK)
	SSPBR("OK\r\n",     cmd_ok, &rootCmdParser),
	SSPBR("ERROR\r\n",  NULL,   &rootCmdParser),
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
/* -------------------- Unsolicited *PSUTTZ... ------------------- */
SSP_CREATE_NORMAL_NODE(netClockSync);
SSP_CREATE_BR_TABLE(netClockSync)
	SSPBR("\r\n",   netClock_rcv, &rootCmdParser),
SSP_END_BR_TABLE

/* ---------------------------- AT+GSN --------------------------- */
SSP_CREATE_TRN_NODE(at_plus_gsn, imeiCollect);
SSP_CREATE_BR_TABLE(at_plus_gsn)
	SSPBR("OK\r\n", imeiSet, &rootCmdParser),
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
cmd_error(unsigned char pos)
{
    (void)pos;
    
    sendModResp_noArgs(evError);
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

static void
ipInitial(unsigned char pos)
{
    (void)pos;
    
    sendModResp_noArgs(evIPInitial);
}

static void
ipStart(unsigned char pos)
{
    (void)pos;
    
    sendModResp_noArgs(evIPStart);
}

static void
ipStatus(unsigned char pos)
{
    (void)pos;
    
    sendModResp_noArgs(evIPStatus);
}

static void
ipGprsAct(unsigned char pos)
{
    (void)pos;
    
    sendModResp_noArgs(evIPGprsAct);
}

static void
ipDone(unsigned char pos)
{
    (void)pos;

    sendModResp_noArgs(evIP);
}

static void
connecting(unsigned char pos)
{
    (void)pos;

    sendModResp_noArgs(evConnecting);
}

static void
closed(unsigned char pos)
{
    (void)pos;

    sendModResp_noArgs(evClosed);
}

static void
connected(unsigned char pos)
{
    (void)pos;

    sendModResp_noArgs(evConnected);
}

static void
disconnected(unsigned char pos)
{
    (void)pos;

    sendModResp_noArgs(evDisconnected);
}

void
data_init(unsigned char c)
{
    (void)c;

    precv = ConMgr_ReceiveDataGetRef();
    precv->size = 0;
    prx = precv->buf;
}

static void
data_collect(unsigned char c)
{
    *prx = c;
    ++prx;
    ++precv->size;
}

static void
data_ready(unsigned char pos)
{
    (void)pos;
    
    *prx = '\0'; 
    precv->size -= (sizeof(END_OF_RECV_STR) - 1);
   
    sendModResp_noArgs(evOk);
}

static void
netClock_rcv(unsigned char pos)
{
    (void)pos;
    
    sendModResp_noArgs(evNetClockSync);
}

static void
lTimeInit(unsigned char pos)
{
	(void)pos;

    plt = ltbuf;
    *plt = '\0';

    lTime = &localTimeEvt.time;
}

static void
yearCollect(unsigned char c)
{
    if(plt >= ltbuf + LTBUFF_SIZE)
        return;

    *plt = c;
    ++plt;
}

static void
yearGet(unsigned char pos)
{
	(void)pos;

    *plt = '\0';

    lTime->tm_year = (short)(YEAR2K + atoi(ltbuf));

    plt = ltbuf;
}

static void
monthGet(unsigned char pos)
{
	(void)pos;

    *plt = '\0';

    lTime->tm_mon = (unsigned char)atoi(ltbuf);

    plt = ltbuf;
}

static void
dayGet(unsigned char pos)
{
	(void)pos;

    *plt = '\0';

    lTime->tm_mday = (unsigned char)atoi(ltbuf);

    plt = ltbuf;
}

static void
hourGet(unsigned char pos)
{
	(void)pos;

    *plt = '\0';

    lTime->tm_hour = (unsigned char)atoi(ltbuf);

    plt = ltbuf;
}

static void
minGet(unsigned char pos)
{
	(void)pos;

    *plt = '\0';

    lTime->tm_min = (unsigned char)atoi(ltbuf);

    plt = ltbuf;
}

static void
lTimeGet(unsigned char pos)
{
	(void)pos;

    lTime->tm_sec = 30;

    RKH_SET_STATIC_EVENT(RKH_UPCAST(RKH_EVT_T, &localTimeEvt), evResponse);

    localTimeEvt.e.fwdEvt = evLocalTime;
        
    RKH_SMA_POST_FIFO(modMgr, RKH_UPCAST(RKH_EVT_T, &localTimeEvt),
						      &sim900parser);
}

static void
imeiInit(unsigned char pos)
{
    (void)pos;

    pImei = imeiEvt.buf;
}

static void
imeiCollect(unsigned char c)
{
    if(pImei >= (imeiEvt.buf + sizeof(imeiEvt.buf) - 1))
        return;
    
    *pImei = c;
    ++pImei;
}

static void
imeiSet(unsigned char pos)
{
	(void)pos;

    imeiEvt.buf[IMEI_LENGTH] = '\0';    

    RKH_SET_STATIC_EVENT(RKH_UPCAST(RKH_EVT_T, &imeiEvt), evResponse);

    imeiEvt.e.fwdEvt = evImei;
        
    RKH_SMA_POST_FIFO(modMgr, RKH_UPCAST(RKH_EVT_T, &imeiEvt),
						      &sim900parser);
}

/* ---------------------------- Global functions --------------------------- */
void
parser_init(void)
{
    RKH_TR_FWK_ACTOR(&sim900parser, "sim900parser");
}

/* ------------------------------ End of file ------------------------------ */
