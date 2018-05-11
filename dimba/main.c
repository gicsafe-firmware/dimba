/**
 *  \file       main.c
 *  \brief      Example application.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2016.03.17  LeFr  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci  lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"
#include "rkhfwk_dynevt.h"
#include "modmgr.h"
#include "bsp.h"


/* ----------------------------- Local macros ------------------------------ */
#define QSTO_SIZE           4

#define SIZEOF_EP0STO       16
#define SIZEOF_EP0_BLOCK    sizeof(RKH_EVT_T)
#define SIZEOF_EP1STO       8
#define SIZEOF_EP1_BLOCK    sizeof(ModCmd)
#define SIZEOF_EP2STO       8
#define SIZEOF_EP2_BLOCK    sizeof(ModCmd)

/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static RKH_EVT_T *qsto[QSTO_SIZE];
static rui8_t evPool0Sto[SIZEOF_EP0STO], 
              evPool1Sto[SIZEOF_EP1STO], 
              evPool2Sto[SIZEOF_EP0STO];

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
int
main(int argc, char *argv[])
{
    bsp_init(argc, argv);
    
    rkh_dynEvt_init();
    rkh_fwk_registerEvtPool(evPool0Sto, SIZEOF_EP0STO, SIZEOF_EP0_BLOCK);
    rkh_fwk_registerEvtPool(evPool1Sto, SIZEOF_EP1STO, SIZEOF_EP1_BLOCK);
    rkh_fwk_registerEvtPool(evPool2Sto, SIZEOF_EP0STO, SIZEOF_EP2_BLOCK);

    RKH_SMA_ACTIVATE(modMgr, qsto, QSTO_SIZE, 0, 0);
    rkh_fwk_enter();

    RKH_TRC_CLOSE();
    return 0;
}

/* ------------------------------ End of file ------------------------------ */
