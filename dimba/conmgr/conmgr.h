/**
 *  \file       conmgr.h
 *  \brief      Specification of connection and protocol manager.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.15  DaBa  v1.0.01  Initial version
 *  2018.05.07  LeFr  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Baliña      db@vortexmakes.com
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* --------------------------------- Module -------------------------------- */
#ifndef __CONMGR_H__
#define __CONMGR_H__

/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"
#include "epoch.h"
#include "modMgr.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */

/**
 * Specifies time waiting for GSM modem first response.
 */
#define SYNC_DELAY_TIME     RKH_TIME_MS(1000)

/**
 * Specifies the maximum tries seeking for GSM modem response. 
 */
#define MAX_SYNC_RETRY      5

/**
 * Specifies time waiting for GSM modem network registration.
 */
#define REGISTRATION_TIME   RKH_TIME_MS(60000)

/**
 * Specifies a time delay after general failure before to restart connection.
 */
#define FAILURE_TRY_DELAY   RKH_TIME_MS(10000)

/**
 * Specifies the maximum tries for setting APN configuration.
 */
#define MAX_CONFIG_RETRY    5

/**
 * Specifies the time waiting for Time Sync Message by Network.
 */
#define WAIT_NETCLOCK_TIME  RKH_TIME_MS(10000)

/**
 * Specifies a time delay after config error before to restart connection.
 */
#define CONFIG_TRY_DELAY    RKH_TIME_MS(3000)

/**
 * Specifies maximum server connection attemps.
 */
#define MAX_CONNECT_RETRY   3

/**
 * Specifies a time delay between server connection attemps.
 */
#define CONNECT_TRY_DELAY   RKH_TIME_MS(5000)

/**
 * Specifies a time delay between server close and reopen.
 */
#define REOPEN_DELAY        RKH_TIME_MS(5000)

/**
 * Specifies connections statusÂ´s polling period.
 */
#define CONNSTATUS_PERIOD   RKH_TIME_MS(2000)

/**
 * Specifies sizeof send / receive buffers.
 */
#define SEND_BUFF_SIZE      1024
#define RECV_BUFF_SIZE      1024

/**
 *  ImeiEvt process definitions
 */
#define IMEI_LENGTH         15
#define IMEI_BUF_SIZE       IMEI_LENGTH + 1

/**
 * GSM Network Provider and Connection specific configurations.
 */
/* .................................. APN .................................. */
    #define MOVISTAR_APN_ADDR   "INTERNER.GPRS.UNIFON.COM.AR"  
    #define MOVISTAR_APN_USER   "WAP"
    #define MOVISTAR_APN_PASS   "WAP"

    #define CLARO_APN_ADDR      "INTERNET.CTIMOVIL.COM.AR"
    #define CLARO_APN_USER      "CLAROGPRS"
    #define CLARO_APN_PASS      "CLAROGPRS999"

#define CONNECTION_APN          CLARO_APN_ADDR
#define CONNECTION_USER         CLARO_APN_USER
#define CONNECTION_PASSWORD     CLARO_APN_PASS

/* .................................. SIM PIN .............................. */
#define SIM_PIN_NUMBER          9474

/* ................................. Server ................................ */
    #define YIPIES_SERVER       "71.6.135.77"
    #define YIPIES_PORT         "33499"

    #define MOSQUITTO_SERVER    "test.mosquitto.org"
    #define MOSQUITTO_PORT      "1883"

    #define LINSSE_SERVER       "linsse.com.ar"
    #define LINSSE_PORT         "2000"

    #define DOCKLIGHT_SERVER    "181.230.72.86"
    #define DOCKLIGHT_PORT      "33498"

    #define DOCKLIGHT_BIS_SERVER "200.127.3.140"
    #define DOCKLIGHT_BIS_PORT   "33498"

#define CONNECTION_PROT         "TCP"
#define CONNECTION_DOMAIN       LINSSE_SERVER 
#define CONNECTION_PORT         LINSSE_PORT

/* ................................ Signals ................................ */
/* ........................ Declares active object ......................... */
RKH_SMA_DCLR(conMgr);

/* ------------------------------- Data types ------------------------------ */
typedef struct SendEvt SendEvt;
struct SendEvt
{
    RKH_EVT_T evt;
    unsigned char buf[SEND_BUFF_SIZE];
    ruint size;
};

typedef struct ReceivedEvt ReceivedEvt;
struct ReceivedEvt
{
    RKH_EVT_T evt;
    unsigned char buf[RECV_BUFF_SIZE];
    ruint size;
};

typedef struct LocalTimeEvt LocalTimeEvt;
struct LocalTimeEvt
{
    ModMgrResp e;
    Time time;
};

typedef struct ImeiEvt ImeiEvt;
struct ImeiEvt
{
    ModMgrResp e;
    char buf[IMEI_BUF_SIZE];
};

/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
ReceivedEvt * ConMgr_ReceiveDataGetRef(void);
char * ConMgr_Imei(void);
char * ConMgr_ImeiSNR(void);

/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
