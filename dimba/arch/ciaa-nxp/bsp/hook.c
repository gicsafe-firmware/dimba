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
 *  RKH web site:   http://sourceforge.net/projects/rkh-reactivesys/
 *  e-mail:         francuccilea@gmail.com
 *  ---------------------------------------------------------------------------
 */

/**
 *  \file       hook.c
 *  \brief      RKH hooks functions for CIAA-NXP
 *
 *  \ingroup    bsp
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2017.04.14  DaBa  v2.4.05  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Baliña     db@vortexmakes.com
 */
/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"
#include "bsp.h"
#include "sapi.h"

RKH_THIS_MODULE

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
#define BSP_TICK_RATE_MS    (1000/RKH_CFG_FWK_TICK_RATE_HZ)

/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
#if defined(RKH_USE_TRC_SENDER)
static rui8_t rkhtick;
#endif

/* ----------------------- Local function prototypes ----------------------- */
static bool_t tickHook(void *p);

/* ---------------------------- Local functions ---------------------------- */
static bool_t
tickHook(void *p)
{
    RKH_TIM_TICK(&rkhtick);
    return 0;
}

/* ---------------------------- Global functions --------------------------- */
void
rkh_hook_start(void)
{    
    tickConfig(BSP_TICK_RATE_MS, tickHook);
    RKH_TR_FWK_ACTOR(&rkhtick, "rkhtick");
}

void
rkh_hook_exit(void)
{
    RKH_TRC_FLUSH();
}

void
rkh_hook_timetick(void)
{
    bsp_timeTick();
}

void
rkh_hook_idle(void)             /* called within critical section */
{
	RKH_ENA_INTERRUPT();
    RKH_TRC_FLUSH();
}

/* ------------------------------ File footer ------------------------------ */
