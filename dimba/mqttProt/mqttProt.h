/**
 *  \file       mqttProt.h
 *  \brief      MQTT Client.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.06.05  LeFr  v1.0.00   Initial version
 *  2018.05.02  DaBa  v1.0.00   Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 *  DaBa  Dario Bali�a db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* --------------------------------- Module -------------------------------- */
#ifndef __MQTTPROT_H__
#define __MQTTPROT_H__

/* ----------------------------- Include files ----------------------------- */
#include "rkh.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
/* ................................ Signals ................................ */
/* ........................ Declares active object ......................... */
RKH_SMA_DCLR(mqttProt);

/* ------------------------------- Data types ------------------------------ */
typedef struct AppData AppData;
struct AppData
{
    rui8_t *data;
    rui16_t size;
};

typedef rui16_t (*MQTTProtPublish)(AppData *appMsg);

typedef struct MQTTProtCfg MQTTProtCfg;
struct MQTTProtCfg
{
    rui16_t publishTime;    /* in secs */
    rui16_t syncTime;       /* in secs */
    char clientId[23];
    rui16_t keepAlive;      /* in secs */
    char topic[16];
    rui8_t qos;             /* 0, 1 or 2 */
};

/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
void MQTTProt_ctor(MQTTProtCfg *config, MQTTProtPublish publisher);

/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
