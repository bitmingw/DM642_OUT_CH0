/*
 *  Copyright 2006 by VisionMagic Ltd.
 *  All rights reserved. Property of VisionMagic Ltd.
 */

/*
 *  ======== vmd642.h ========
 *
 *  This file contains VMD642-A board initialization and CPLD register access
 *  functions.
 *
 */

#ifndef __VMD642_INCLUDED__
#define __VMD642_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#include <csl.h>
#include <csl_i2c.h>
#include <csl_gpio.h>

/*
 *  Note:  Bit definitions for each register field
 *         needs to be supplied here for the CPLD
 *	       and other board periperals.
 */

/* Compatability definitions */
#define NULL                 0

/* CPLD address definitions */
#define VMD642_CPLD_BASE       0x90080000

/* CPLD Register Indices */
#define VMD642_IOOUT       0x12 //write only
#define VMD642_IOINPUT     0x13 //read ony

/* Macros of moving functions */
#define HOLDER_MOV_STAY  0
#define HOLDER_MOV_LEFT  1
#define HOLDER_MOV_RIGHT 2

/* Read an 8-bit value from a CPLD register */
Uint8 VMD642_rget(Int16 regnum);

/* Write an 8-bit value to a CPLD register */
void VMD642_rset(Int16 regnum, Uint8 regval);

/* Spin in a delay loop for delay iterations */
void VMD642_wait(Uint32 delay);

/* Spin in a delay loop for delay microseconds */
void VMD642_waitusec(Uint32 delay);


#ifdef __cplusplus
}
#endif

#endif
