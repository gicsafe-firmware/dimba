/*
 *	sleep.h
 *	
 *	Programmer: Darío S. Baliña.
 *	Date : 26/05/06.
 */

#ifndef __SLEEP_H__
#define __SLEEP_H__

#include <windows.h>

#define _1SEC	(1000/SLEEP_BASE)
#define SLEEP_BASE	1

/*
 * sleep: Performs a delay of msec in the execution
 * 			msec must be especified as a multiplo of 
 * 			RTI cadence MSEQBASE.
 */
#define sleep(x)	Sleep(x)

#endif
