/**************************************************************************/
/*  Copyright 2006 by VisionMagic Ltd.                                    */
/*  All rights reserved. Property of VisionMagic Ltd.                     */
/**************************************************************************/
 
/*
 *  ======== vmd642_cpld.c ========
 *  CPLD module
 */

#include <csl.h>
#include "vmd642.h"

/* Read an 8-bit value from a CPLD register */
Uint8 VMD642_rget(Int16 regnum)
{
    Uint8 *pdata;
    
    /* Return lower 8 bits of register */
    pdata = (Uint8 *)(VMD642_CPLD_BASE + regnum);
    return (*pdata & 0xff);
}

/* Write an 8-bit value to a CPLD register */
void VMD642_rset(Int16 regnum, Uint8 regval)
{
    Uint8 *pdata;
    
    /* Write lower 8 bits of register */
    pdata = (Uint8 *)(VMD642_CPLD_BASE + regnum);
    *pdata = (regval & 0xff);
}

/* Spin in a delay loop for delay iterations */
void VMD642_wait(Uint32 delay)
{
    volatile Uint32 i, n;
    
    n = 0;
    for (i = 0; i < delay; i++)
    {
        n = n + 1;
    }
}

/* Spin in a delay loop for delay microseconds */
void VMD642_waitusec(Uint32 delay)
{
    VMD642_wait(delay * 21);
}



