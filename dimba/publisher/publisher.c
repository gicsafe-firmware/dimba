/**
 *  \file       publisher.c
 *  \brief      Implementation JSON messaje formating and MQTT publish.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.06.22  DaBa  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa Dar�o Bali�a  db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include <string.h>
#include <stdlib.h>
#include "publisher.h"
#include "ioChgDet.h"
#include "anSampler.h"
#include "jWrite.h"
#include "conmgr.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
char dataBuf[1024];

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
static rui16_t
getNextPublishTime(int nSamples)
{
    int x, th;
    rui16_t pubTime;

    for (th = (MAX_AN_NUM_SAMPLES >> 1), x = 0; 
         x < NUM_PUBTIME_STEPS; 
         ++x, th >>= 1)
    {
        if (nSamples > th)
        {
            break;
        }
    }
    pubTime = (rui16_t)(MAX_PUBLISH_TIME >> (NUM_PUBTIME_STEPS - x));
    return (pubTime < 8) ? 8 : pubTime; /* Set minimal publish time to 8s */
}

/* ---------------------------- Global functions --------------------------- */
rui16_t
publishDimba(AppData *appMsg)
{
    AnSampleSet anSet;
    IOChg ioChg[NUM_DI_SAMPLES_GET];
    int n, i, j;
    double value;

    jwOpen( dataBuf, sizeof(dataBuf), JW_OBJECT, JW_COMPACT );

    
    jwObj_int("id", atoi(ConMgr_imeiSNR()));
    jwObj_int("sl", ConMgr_sigLevel());

    n = anSampler_getSet(&anSet, NUM_AN_SAMPLES_GET);

    if(n>0)
    {
        jwObj_object("anIn");
            jwObj_int("ts", anSet.timeStamp);
            jwObj_int("tsm", AN_SAMPLING_RATE_SEC);
            jwObj_array("an");
            for(i=0; i < NUM_AN_SIGNALS; ++i)
            {
                jwArr_array();
                for(j=0; j<n; ++j)
                {
                	value = ((anSet.anSignal[i][j] & 0xFF00)>>8) * 100;
                    value += (anSet.anSignal[i][j] & 0x00FF);
                    value /= 100;
                    jwArr_double(value);
                }
                jwEnd();
            }
            jwEnd();
        jwEnd();
    }

    n = IOChgDet_get(ioChg, NUM_DI_SAMPLES_GET);
    if(n > 0)
    {
        jwObj_array("dInChg");
        for(i=0; i < n; ++i)
        {
            jwArr_object();
                jwObj_double("ts", ioChg[i].timeStamp);
                jwObj_int("dIn", ioChg[i].signalId);
                jwObj_int("val", ioChg[i].signalValue);
            jwEnd();
        }
        jwEnd();
    }
    
    jwClose();

    appMsg->data = (rui8_t *)dataBuf;
    appMsg->size = (rui16_t)strlen(dataBuf);
    return getNextPublishTime(anSampler_getNumSamples());
}

/* ------------------------------ End of file ------------------------------ */
