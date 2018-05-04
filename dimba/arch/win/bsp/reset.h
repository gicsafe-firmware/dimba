/*
 *	reset.h
 *
 *	Routines for Reset MCU
 *
 */

#ifndef __RESET_H__
#define __RESET_H__

#include <windows.h>

/* 
 * reset_now: If in debugg mode stop the execution,
 *			  resets the MCU in other case.
 */
#define reset_now()		__debugbreak()

#endif
