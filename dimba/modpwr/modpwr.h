/**
 *  \file       modpwr.h
 *  \brief      Specification of modpwr abstraction.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.06.05  DaBa  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  DaBa  Dario Bali�a       db@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* --------------------------------- Module -------------------------------- */
#ifndef __MODPWR_H__
#define __MODPWR_H__

/* ----------------------------- Include files ----------------------------- */
/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
/* ------------------------------- Data types ------------------------------ */
/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
#ifdef MODPWR_CTRL_ENABLE

void modPwr_init(void);
void modPwr_ctrl(void);
void modPwr_on(void);
void modPwr_off(void);

#else

#define modPwr_init();
#define modPwr_ctrl();
#define modPwr_on();
#define modPwr_off();

#endif

/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */
