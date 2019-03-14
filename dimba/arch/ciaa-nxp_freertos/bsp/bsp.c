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
#define ModStatus_init()    gpioConfig(DO5, GPIO_OUTPUT)
#define ModStatus(b)        gpioWrite(DO5, b)
#define ModStatus_toggle()  gpioToggle(DO5)
#define RegStatus_init()    gpioConfig(DO6, GPIO_OUTPUT)
#define RegStatus(b)        gpioWrite(DO6, b)
#define NetStatus_init()    gpioConfig(DO7, GPIO_OUTPUT)
#define NetStatus(b)        gpioWrite(DO7, b)

/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static RKH_TS_T tstamp;
static ModCmdRcvHandler cmdParser;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
void
bsp_init(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    boardConfig();
    ModStatus_init();
    ModStatus(0);
    RegStatus(UnregisteredSt);
    NetStatus_init();
    NetStatus(DisconnectedSt);

    modPwr_init();
    dIn_init();
	anIn_init();

    rtime_init();
}

void
bsp_timeTick(void)
{
    ++tstamp;
    
    dIn_scan();
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

    cmdParser = ModCmd_init();
    uartConfig(UART_232, 19200);
    uartIsr_rxEnable(UART_232, gsm_rx_isr);
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

void
bsp_regStatus(Status_t status)
{
    RegStatus(status);
}

void 
bsp_netStatus(Status_t status)
{
    NetStatus(status);
}

void 
bsp_modStatusToggle(void)
{
    ModStatus_toggle();
}


/* ------------------------------ File footer ------------------------------ */
