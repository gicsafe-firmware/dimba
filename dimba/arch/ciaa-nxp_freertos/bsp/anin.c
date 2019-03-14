/**
 *  \file       anin.c
 *  \brief      Implementation of Analog Inputs adquisition and filtering.
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
#include "rkh.h"
#include "sapi.h"
#include "anin.h"
#include "mTimeCfg.h"
#include "emaFilter.h"
#include "anSampler.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
#define ANINS_EMA_ALPHA     2
#define SAPI_ADC_CH_OFFSET  66

/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static adc_t anIns[NUM_ANIN_SIGNALS];
static const uint8_t chMap[NUM_ANIN_SIGNALS] = {AI0, AI1, AI2, AI3};
static int currChannel;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
static void
anIn_adcStart(int channel)
{
    uint8_t lpcAdcChannel = SAPI_ADC_CH_OFFSET - chMap[channel];

    Chip_ADC_EnableChannel(LPC_ADC0, lpcAdcChannel, ENABLE);
    Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
}

static uint16_t
anIn_adcRead(int channel)
{   
    uint8_t lpcAdcChannel = SAPI_ADC_CH_OFFSET - chMap[channel];
    uint16_t analogValue = 0;
    int i;

    while(
       (Chip_ADC_ReadStatus(LPC_ADC0, lpcAdcChannel, ADC_DR_DONE_STAT) != SET)
    )
       ++i;

    Chip_ADC_ReadValue( LPC_ADC0, lpcAdcChannel, &analogValue );

    Chip_ADC_EnableChannel( LPC_ADC0, lpcAdcChannel, DISABLE );

    return analogValue;
}


/* ---------------------------- Global functions --------------------------- */
void
anIn_init(void)
{
    adcConfig(ADC_ENABLE);

    currChannel = anIn0; 
    anIn_adcStart(currChannel);
}

void
anIn_captureAndFilter(void)
{
    uint16_t value;

    value = anIn_adcRead(currChannel);
    anIns[currChannel] = emaFilter_LowPass(value, 
                                           anIns[currChannel],
                                           ANINS_EMA_ALPHA);

    if(++currChannel > anIn3)
        currChannel = anIn0;

    anIn_adcStart(currChannel);
}

adc_t
anIn_get(int channel)
{
    if(channel > NUM_ANIN_SIGNALS)
        return 0;

    return anIns[channel];
}

void
anIn_update(void)
{
   anSampler_put();
}

/* ------------------------------ End of file ------------------------------ */
