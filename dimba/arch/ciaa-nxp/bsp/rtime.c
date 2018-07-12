/**
 *  \file       rtime.c
 *  \brief      Implementation of RTC abstraction for CIAA-NXP bsp.
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
#include "rtime.h"
#include "sapi.h"

/* ----------------------------- Local macros ------------------------------ */
#define RunLed_init()      gpioConfig(DO4, GPIO_OUTPUT)
#define RunLed_toggle()    gpioToggle(DO4)

/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static Time t;
static rtc_t rtc;
static rtc_t rtcDft = { 1970, 1, 1, 1, 0, 0, 0 };

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
void
rtime_init(void)
{
    RunLed_init();
    RunLed_toggle();

	Chip_RTC_Init(LPC_RTC);
    Chip_RTC_Enable(LPC_RTC, ENABLE);
    rtcRead(&rtc);

    if( (rtc.year < 1970) ||
		(rtc.month < 1) || (rtc.month > 12) ||
    	(rtc.mday < 1) || (rtc.mday > 31) ||
		(rtc.wday < 1) || (rtc.wday > 7) ||
		(rtc.hour < 0) || (rtc.hour > 23) ||
		(rtc.min < 0) || (rtc.min > 59) ||
		(rtc.sec < 0) || (rtc.sec > 59)
      )
    {
    	rtcWrite(&rtcDft);
    }
}

Time *
rtime_get(void)
{
    RunLed_toggle();
    rtcRead(&rtc);

    t.tm_sec = rtc.sec;
    t.tm_min = rtc.min;
    t.tm_hour = rtc.hour;
    t.tm_mday = rtc.mday;
    t.tm_mon = rtc.month;
    t.tm_year = rtc.year;
    t.tm_wday = rtc.wday;
    t.tm_isdst = 0;

    return &t;
}

void
rtime_set(Time *pt)
{
   rtc.sec = pt->tm_sec;
   rtc.min = pt->tm_min;
   rtc.hour = pt->tm_hour;
   rtc.wday = pt->tm_wday;
   rtc.mday = pt->tm_mday;
   rtc.month = pt->tm_mon;
   rtc.year = pt->tm_year;

   rtcWrite(&rtc);
}

/* ------------------------------ End of file ------------------------------ */
