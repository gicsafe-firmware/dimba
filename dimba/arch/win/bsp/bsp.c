/**
 *  \file       bsp.c
 *  \brief      BSP for 80x86 OS win32
 *
 *  \ingroup    bsp
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.23  DaBa  v0.0.01  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Baliña       db@vortexmakes.com
 *  LeFr  Leandro Francucci  lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include <stdio.h>

#include "signals.h"
#include "modpwr.h"
#include "modmgr.h"
#include "conmgr.h"
#include "CirBuffer.h"
#include "mTime.h"
#include "din.h"
#include "anin.h"
#include "anSampler.h"
#include "ioChgDet.h"
#include "epoch.h"

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
#define ESC                 0x1B
#define DIMBA_CFG_OPTIONS   "st:f:p:m:h"

#define TEST_TX_PACKET      "----o Ping"
#define TEST_RX_PACKET      "o---- Pong"

/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
SERIAL_T serials[ NUM_CHANNELS ] =
{
	{	"COM8",	19200, 8, PAR_NONE, STOP_1, 0 },	// COM1
};

/* ---------------------------- Local variables ---------------------------- */
static rui8_t bsp;
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
static RKH_ROM_STATIC_EVENT(e_Close, evClose);
static RKH_ROM_STATIC_EVENT(e_Ok, evOk);
static RKH_ROM_STATIC_EVENT(e_Recv, evRecv);

static SendEvt e_Send;

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

    modPwr_init();
    dIn_init();
	anIn_init();
    anSampler_init();
    IOChgDet_init();
    epoch_init();
    
    mTime_init();

    rkh_fwk_init();

    RKH_FILTER_ON_GROUP(RKH_TRC_ALL_GROUPS);
    RKH_FILTER_ON_EVENT(RKH_TRC_ALL_EVENTS);
	RKH_FILTER_OFF_EVENT(MODCMD_USR_TRACE);
	RKH_FILTER_OFF_GROUP_ALL_EVENTS(RKH_TG_USR);
    //RKH_FILTER_OFF_EVENT(RKH_TE_TMR_TOUT);
    RKH_FILTER_OFF_EVENT(RKH_TE_SM_STATE);
    RKH_FILTER_OFF_EVENT(RKH_TE_SMA_FIFO);
    RKH_FILTER_OFF_EVENT(RKH_TE_SMA_LIFO);
    //RKH_FILTER_OFF_SMA(modMgr);
    RKH_FILTER_OFF_SMA(conMgr);
    RKH_FILTER_OFF_ALL_SIGNALS();

    RKH_TRC_OPEN();

    RKH_TR_FWK_ACTOR(&bsp, "bsp");
}

#define NUM_AN_SAMPLES_GET  10
#define NUM_DI_SAMPLES_GET  8

static
void
send_signalsFrame(void)
{
    AnSampleSet anSet;
    IOChg ioChg[NUM_DI_SAMPLES_GET];
    int n, l, i, j;
    char *p;

    RKH_SET_STATIC_EVENT(RKH_UPCAST(RKH_EVT_T, &e_Send), evSend);
   
    n = anSampler_getSet(&anSet, NUM_AN_SAMPLES_GET);
   
    p = (char *)(e_Send.buf);
    l = 0;

    for(i=0; i < NUM_AN_SIGNALS; ++i)
    {
        l += sprintf(p + l, "ts:%u, AN[%d]", anSet.timeStamp, i);
        for(j=0; j<n; ++j)
        {
            l += sprintf(p + l,", %d", anSet.anSignal[i][j]);
        }
        l += sprintf(p + l, "\r\n");
    }

    n = IOChgDet_get(ioChg, NUM_DI_SAMPLES_GET);
    
    for(i=0; i < n; ++i)
    {
        l += sprintf(p + l, "ts:%u, DI[%d]:%d\r\n", ioChg[i].timeStamp,
                                                       ioChg[i].signalId,
                                                       ioChg[i].signalValue );
    }

    e_Send.size = l;

    RKH_SMA_POST_FIFO(conMgr, RKH_UPCAST(RKH_EVT_T, &e_Send), &bsp);
}


void
bsp_keyParser(int c)
{
    switch(c)
    {
        case ESC:
            RKH_SMA_POST_FIFO(modMgr, &e_Term, &bsp);
            rkhport_fwk_stop();
            break;

        case 'o':
            RKH_SMA_POST_FIFO(conMgr, &e_Open, &bsp);
            break;

        case 'c':
            RKH_SMA_POST_FIFO(conMgr, &e_Close, &bsp);
            break;

        case 'r':
            RKH_SMA_POST_FIFO(conMgr, &e_Recv, &bsp);
            break;

        case 's':
            RKH_SET_STATIC_EVENT(RKH_UPCAST(RKH_EVT_T, &e_Send), evSend);
            e_Send.size = strlen(TEST_TX_PACKET);

            memcpy(e_Send.buf, (unsigned char *)TEST_TX_PACKET, e_Send.size);

            RKH_SMA_POST_FIFO(conMgr, RKH_UPCAST(RKH_EVT_T, &e_Send), &bsp);
            break;

        case 'a':
            send_signalsFrame();
            break;


        default:
            break;
    }
}

void
bsp_timeTick(void)
{
    mTime_tick();
}

static
void
ser_rx_isr( unsigned char byte )
{
    cmdParser(byte);
	putchar(byte);
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

void
bsp_serial_putnchar(int ch, unsigned char *p, ruint ndata)
{
    while(ndata && (ndata-- != 0))
    {
        tx_data(ch, *p);
        ++p;
    }
}

/* ------------------------------ File footer ------------------------------ */
