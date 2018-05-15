/*
 *  --------------------------------------------------------------------------
 *
 *                                Framework RKH
 *                                -------------
 *
 *            State-machine framework for reactive embedded systems
 *
 *                      Copyright (C) 2010 Leandro Francucci.
 *          All rights reserved. Protected by international copyright laws.
 *
 *
 *  RKH is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any
 *  later version.
 *
 *  RKH is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with RKH, see copying.txt file.
 *
 *  Contact information:
 *  RKH site: http://vortexmakes.com/que-es/
 *  RKH GitHub: https://github.com/vortexmakes/RKH
 *  RKH Sourceforge: https://sourceforge.net/projects/rkh-reactivesys/
 *  e-mail: lf@vortexmakes.com
 *  ---------------------------------------------------------------------------
 */

/**
 *  \file       bsp_blinky.c
 *  \brief      BSP for 80x86 OS win32
 *
 *  \ingroup    bsp
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2017.04.14  LeFr  v2.4.05  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci  lf@vortexmakes.com
 *  DaBa  Dario Baliña       dariosb@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include <stdio.h>

#include "signals.h"
#include "modmgr.h"
#include "conmgr.h"

#include "bsp.h"
#include "getopt.h"
#include "rkh.h"
#include "trace_io_cfg.h"
#include "wserial.h"
#include "wserdefs.h"
#include "modcmd.h"

RKH_THIS_MODULE

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
#define ESC         0x1B
#define DIMBA_CFG_OPTIONS    "st:f:p:m:h"

/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
SERIAL_T serials[ NUM_CHANNELS ] =
{
	{	"COM8",	19200, 8, PAR_NONE, STOP_1, 0 },	// COM1
};

/* ---------------------------- Local variables ---------------------------- */
static ModCmdRcvHandler cmdParser;
static char *opts = (char *)DIMBA_CFG_OPTIONS;
static const char *helpMessage =
{
    "\nOption usage:\n"
	"\t -s silence\n"
    "\t -f File name for binary trace output\n"
    "\t -t ipaddr of TCP trace client\n"
    "\t -p port of TCP trace client\n"
    "\t -m GSM Module Serial Port\n"
    "\t -h (help)\n"
};

static RKH_ROM_STATIC_EVENT(e_Term, evTerminate);
static RKH_ROM_STATIC_EVENT(e_Open, evOpen);
static RKH_ROM_STATIC_EVENT(e_Sync, evSync);

static void ser_rx_isr(unsigned char byte);
static void ser_tx_isr(void);
static SERIAL_CBACK_T ser_cback =
{ ser_rx_isr, NULL, NULL, ser_tx_isr, NULL, NULL, NULL };

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
static void
printBanner(void)
{
    printf("DIMBA.\n\n");
    printf("RKH version      = %s\n", RKH_RELEASE);
    printf("Port version     = %s\n", rkhport_get_version());
    printf("Port description = %s\n\n", rkhport_get_desc());
	printf("Description: \n\n"
		"Sistema de monitoreo remoto de pasos a nivel\n");

    printf("1.- Press ESC to quit \n\n\n");
}

static void
processCmdLineOpts(int argc, char **argv)
{
    int c;

    while ((c = getopt(argc, argv, opts)) != EOF)
        switch (c)
        {
            case 'm':
                strcpy(serials[GSM_PORT].com_name, optarg);
                break;

			case 's':
                trace_io_silence();
				break;

            case 'f':
                trace_io_setFileName(optarg);
                break;

            case 't':
                trace_io_setTcpIpAddr(optarg);
                break;

            case 'p':
                trace_io_setTcpPort((short)atoi(optarg));
                break;

            case '?':
            case 'h':
                printf(helpMessage);
                break;
        }
}

/* ---------------------------- Global functions --------------------------- */
void
bsp_init(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    printBanner();

    processCmdLineOpts(argc, argv);

    rkh_fwk_init();

    RKH_FILTER_ON_GROUP(RKH_TRC_ALL_GROUPS);
    RKH_FILTER_ON_EVENT(RKH_TRC_ALL_EVENTS);
    RKH_FILTER_OFF_EVENT(RKH_TE_TMR_TOUT);
    RKH_FILTER_OFF_EVENT(RKH_TE_SM_STATE);
    RKH_FILTER_OFF_EVENT(RKH_TE_SMA_FIFO);
    RKH_FILTER_OFF_EVENT(RKH_TE_SMA_LIFO);
    RKH_FILTER_OFF_EVENT(RKH_TE_FWK_GC);
    RKH_FILTER_OFF_EVENT(RKH_TE_FWK_AE);
    RKH_FILTER_OFF_EVENT(RKH_TE_FWK_GCR);
    RKH_FILTER_OFF_SMA(modMgr);
    RKH_FILTER_OFF_SMA(conMgr);
    RKH_FILTER_OFF_ALL_SIGNALS();

    RKH_TRC_OPEN();
}

void
bsp_keyParser(int c)
{
    switch(c)
    {
        case ESC:
            RKH_SMA_POST_FIFO(modMgr, &e_Term, 0);
            rkhport_fwk_stop();
            break;

        case 'o':
            RKH_SMA_POST_FIFO(conMgr, &e_Open, 0);
            break;

        case 's':
            RKH_SMA_POST_FIFO(conMgr, &e_Sync, 0);
            break;
    }
}

void
bsp_timeTick(void)
{
}

static
void
ser_rx_isr( unsigned char byte )
{
    cmdParser(byte);
}

static
void
ser_tx_isr( void )
{
}

void
bsp_serial_open(int ch)
{
    init_serial_hard(ch, &ser_cback );
    connect_serial(ch);
    cmdParser = ModCmd_init();
}

void
bsp_serial_close(int ch)
{
	disconnect_serial(ch);
	deinit_serial_hard(ch);
}

void
bsp_serial_puts(int ch, char *p)
{
    while(*p!='\0')
    {
        tx_data(ch, *p);
        ++p;
    }
}

/* ------------------------------ File footer ------------------------------ */
