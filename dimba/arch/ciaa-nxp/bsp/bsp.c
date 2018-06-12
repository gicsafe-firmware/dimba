/**
 *  \file       bsp.c
 *  \brief      BSP for CIAA-NXP Prototype
 *
 *  \ingroup    bsp
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2017.04.14  DaBa  v0.0.01  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Baliña       db@vortexmakes.com
 *  LeFr  Leandro Francucci  lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include <stdio.h>
#include "rkh.h"
#include "sapi.h"
#include "uartisr.h"
#include "bsp.h"

#include "signals.h"
#include "modpwr.h"
#include "modmgr.h"
#include "modcmd.h"
#include "conmgr.h"
#include "CirBuffer.h"
#include "mTime.h"
#include "din.h"
#include "anin.h"
#include "anSampler.h"
#include "ioChgDet.h"
#include "epoch.h"
#include "rtime.h"

RKH_THIS_MODULE

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static RKH_TS_T tstamp;
static ModCmdRcvHandler cmdParser;
static RKH_ROM_STATIC_EVENT(e_Term, evTerminate);
static RKH_ROM_STATIC_EVENT(e_Close, evClose);
static RKH_ROM_STATIC_EVENT(e_Ok, evOk);
static const Time t = { 0, 36, 23, 11, 6, 2018, 2, 0 };

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
void
bsp_init(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    boardConfig();

    modPwr_init();
    dIn_init();
	anIn_init();

    //rtime_set(&t);
}

void
bsp_timeTick(void)
{
    ++tstamp;
    modPwr_ctrl();
    mTime_tick();
}

RKH_TS_T
rkh_trc_getts(void)
{
    return tstamp;
}

static
void
gsm_rx_isr( unsigned char byte )
{
    cmdParser(byte);
}

void
bsp_serial_open(int ch)
{
    (void)ch;

    uartConfig(UART_232, 19200);
    uartIsr_rxEnable(UART_232, gsm_rx_isr);
    cmdParser = ModCmd_init();
}

void
bsp_serial_close(int ch)
{
    (void)ch;
}

void
bsp_serial_puts(int ch, char *p)
{
    while(*p!='\0')
    {
        uartWriteByte(UART_232, *p);
        ++p;
    }
}

void
bsp_serial_putnchar(int ch, unsigned char *p, ruint ndata)
{
    while(ndata && (ndata-- != 0))
    {
        uartWriteByte(UART_232, *p);
        ++p;
    }
}

/* ------------------------------ File footer ------------------------------ */
