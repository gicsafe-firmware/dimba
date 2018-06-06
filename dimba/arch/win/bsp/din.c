/**
 *  \file       din.c
 *  \brief      Implementation of Digital Inputs HAL and change detection.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.17  DaBa  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Baliña       db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include <conio.h>
#include <windows.h>

#include "rkh.h"
#include "IOChgDet.h"
#include "din.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static unsigned char dins[NUM_DIN_SIGNALS];
static unsigned char dinsKb[NUM_DIN_SIGNALS];
static int tick;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
static
DWORD WINAPI
isr_dinThread(LPVOID par)      /* Win32 thread to emulate keyboard ISR */
{
    (void)par;
    unsigned char s;

    while (rkhport_fwk_is_running())
    {
        s = (unsigned char)(_getch() - '0');

        if(s > NUM_DIN_SIGNALS)
            continue;
        
        dinsKb[s] ^= dinsKb[s];
    }
    return 0;
}

/* ---------------------------- Global functions --------------------------- */
void
din_init(void)
{
    DWORD thkdinId;
    HANDLE hthDin;

    hthDin = CreateThread(NULL, 1024, &isr_dinThread, 0, 0, &thkdinId);
    memset(dins, 0, sizeof(dins));
    memset(dinsKb, 0, sizeof(dins));
    
    tick = DIN_SCAN_PERIOD;
}

void
din_scan(void)
{
    unsigned char i;

    if(tick && (--tick != 0))
        return;
        
    tick = DIN_SCAN_PERIOD;

    for(i=0; i < NUM_DIN_SIGNALS; ++i)
    {
        if(dins[i] != dinsKb[i])
        {
            IOChgDet_put(i, dinsKb[i]);
            dins[i] = dinsKb[i];
        }
    }
}

/* ------------------------------ End of file ------------------------------ */
