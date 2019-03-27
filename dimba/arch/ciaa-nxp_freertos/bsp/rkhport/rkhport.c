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
 *  \file       rkhport.c
 *  \brief      FreeRTOS v10.0.1 port
 *
 *  \ingroup    port
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.09.23  FrBu Initial version
 *  2019.03.18  DaBa Fix post queue fifo/lifo and priority assignment
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  FrBu  Franco Bucafusco
 *  DaBa  Dario Bali#a       db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"
#include "rkhfwk_dynevt.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
RKH_MODULE_NAME(rkhport)
RKH_MODULE_VERSION(rkhport, 1.00)
RKH_MODULE_DESC(rkhport, "FreeRTOS v10.0.1")

/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static ruint tickScaler;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */

void
vApplicationTickHook(void)
{
    if (tickScaler && --tickScaler)
    {
        return;
    }

    tickScaler = configTICK_RATE_HZ / RKH_CFG_FWK_TICK_RATE_HZ;

    rkh_tmr_tick(0);
}

static
void
thread_function(void *arg)
{
    RKH_SMA_T *sma;

    RKH_SR_ALLOC();

    sma = (RKH_SMA_T *)arg;
    while (sma->running != (rbool_t)0)
    {
        RKH_EVT_T *e = rkh_sma_get(sma);
        RKH_SMA_DISPATCH((RKH_SM_T *)arg, e);
        RKH_FWK_GC(e, arg);
    }

    rkh_sma_unregister(sma);

    RKH_TR_SMA_TERM(sma, RKH_GET_PRIO(sma));

    vTaskDelete(NULL);
}

/* ---------------------------- Global functions --------------------------- */
const
char *
rkhport_get_version(void)
{
    return RKH_MODULE_GET_VERSION();
}

const
char *
rkhport_get_desc(void)
{
    return RKH_MODULE_GET_DESC();
}

void
rkh_sma_block(RKH_SMA_T *const me)
{
    (void)me;
}

void
rkh_sma_setReady(RKH_SMA_T *const me)
{
    (void)me;
}

void
rkh_sma_setUnready(RKH_SMA_T *const me)
{
    (void)me;
}

void
rkh_fwk_init(void)
{
}

void
rkh_fwk_enter(void)
{
    RKH_SR_ALLOC();

    RKH_HOOK_START();   /* RKH start-up callback */
    RKH_TR_FWK_EN();

    vTaskStartScheduler();

    RKH_TRC_CLOSE();    /* cleanup the trace session */
}

void
rkh_fwk_exit(void)
{
    RKH_SR_ALLOC();
    RKH_TR_FWK_EX();
    RKH_HOOK_EXIT();    /* RKH cleanup callback */
}

void
rkh_sma_terminate(RKH_SMA_T *sma)
{
    sma->running = (rbool_t)0;
    vQueueDelete(&sma->equeue);
}

void
rkh_sma_activate(RKH_SMA_T *sma, const RKH_EVT_T **qs, RKH_QUENE_T qsize,
                 void *stks, rui32_t stksize)
{
    rui8_t prio;
    BaseType_t rv;

    RKH_REQUIRE(qs == (const RKH_EVT_T **)0);

    sma->equeue = xQueueCreate(qsize, sizeof(void*));

    rkh_sma_register(sma);
    rkh_sm_init((RKH_SM_T *)sma);

    prio = RKH_LOWEST_PRIO - RKH_GET_PRIO(sma) + tskIDLE_PRIORITY;

    rv = xTaskCreate(thread_function,                   /* function */
                     "freertos task",                   /* name */
                     stksize / sizeof(StackType_t),     /* stack size */
                     sma,                               /* function argument */
                     prio,                              /* priority */
                     &sma->thread);

    RKH_ASSERT(rv == pdTRUE);

    sma->running = (rbool_t)1;
}

#if defined(RKH_USE_TRC_SENDER)
void
rkh_sma_post_fifo(RKH_SMA_T *sma, const RKH_EVT_T *e,
                  const void *const sender)
#else
void
rkh_sma_post_fifo(RKH_SMA_T *sma, const RKH_EVT_T *e)
#endif
{
    BaseType_t taskwoken = pdFALSE;
    BaseType_t rv;

    RKH_HOOK_SIGNAL(e);

    RKH_INC_REF(e);

    rv = xQueueSendFromISR(sma->equeue, &e, &taskwoken);

    RKH_ASSERT(rv == pdTRUE);

    RKH_TR_SMA_FIFO(sma, e, sender, e->pool, e->nref, 0, 0);

//    portYIELD_FROM_ISR(taskwoken);
}

#if (RKH_CFG_QUE_PUT_LIFO_EN == RKH_ENABLED) && \
    (defined(RKH_USE_TRC_SENDER))
void
rkh_sma_post_lifo(RKH_SMA_T *sma, const RKH_EVT_T *e,
                  const void *const sender)
#else
void
rkh_sma_post_lifo(RKH_SMA_T *sma, const RKH_EVT_T *e)
#endif
{
    BaseType_t taskwoken = pdFALSE;
    BaseType_t rv;

    RKH_HOOK_SIGNAL(e);

    RKH_INC_REF(e);

    rv = xQueueSendToFrontFromISR(sma->equeue, &e, &taskwoken);

    RKH_ASSERT(rv == pdTRUE);

    RKH_TR_SMA_LIFO(sma, e, sender, e->pool, e->nref, 0, 0);

//    portYIELD_FROM_ISR(taskwoken);
}

RKH_EVT_T *
rkh_sma_get(RKH_SMA_T *sma)
{
    BaseType_t rv;
    RKH_EVT_T *e;

    RKH_SR_ALLOC();

    rv = xQueueReceive(sma->equeue, &e, portMAX_DELAY);

    RKH_ASSERT(rv == pdTRUE);

    RKH_TR_SMA_GET(sma, e, e->pool, e->nref, 0, 0);

    return e;
}

/* ------------------------------ End of file ------------------------------ */