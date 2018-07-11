/**
 *  \file       publisher.h
 *  \brief      Specification JSON messaje formating and MQTT publish.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.06.22  DaBa  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa Darío Baliña  db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* --------------------------------- Module -------------------------------- */
#ifndef __PUBLISHER_H__
#define __PUBLISHER_H__

/* ----------------------------- Include files ----------------------------- */
#include "mqttProt.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
#define MAX_PUBLISH_TIME    64
#define NUM_AN_SAMPLES_GET  ((MAX_PUBLISH_TIME / 10) + 4)
#define NUM_DI_SAMPLES_GET  8
#define NUM_PUBTIME_STEPS   3

/* ------------------------------- Data types ------------------------------ */
/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
rui16_t publishDimba(AppData *appMsg);

/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
