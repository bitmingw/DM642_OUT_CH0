/**************************************************************************/
/*  Copyright 2006 by VisionMagic Ltd.                                    */
/*  All rights reserved. Property of VisionMagic Ltd.                     */
/**************************************************************************/
 
/*
 *  ======== vmd642_uart.c ========
 *  UART module
 */
 
#include <csl.h>

#include "vmd642.h"
#include "vmd642_uart.h"

/***************************************************************************/
/*  ======== VMD642_UART_rset ========								   */
/*  Set a UART register,设置寄存器										   */
/***************************************************************************/
void VMD642_UART_rset(VMD642_UART_Handle hUart, 
						Int16 regnum, 
						Int16 regval)
{
    Int16 regindex, lcr;
    
    /* Register index is determined by lower 3 bits and the target UART */
    
    regindex = regnum & 0x7;
    if (hUart == 1)
        regindex += 8;
    /*是否为高位的寄存器*/
    /* If regnum between 0x08 and 0x0F, set bit 7 of LCR to access register */
    if ((regnum & 0x18) == 0x8)
    {
        lcr = VMD642_UART_rget(hUart, VMD642_UART_LCR);
        VMD642_waitusec(1);
        VMD642_UART_rset(hUart, VMD642_UART_LCR, lcr | 0x80);
        VMD642_waitusec(1);
        VMD642_rset(regindex, regval);
        VMD642_waitusec(1);
        VMD642_UART_rset(hUart, VMD642_UART_LCR, lcr);
        VMD642_waitusec(1);
    }
    else
    {
        
        VMD642_rset(regindex, regval);
        VMD642_waitusec(1);
    }
}
/**********************************************************************/
/*  ======== VMD642_UART_rget ========							  */
/*  Get the value of a UART register,读出寄存器的值					  */
/**********************************************************************/
Int16 VMD642_UART_rget(VMD642_UART_Handle hUart, 
						 Int16 regnum)
{
    Int16 regindex, returnval, lcr;
    
    /* Register index is determined by lower 3 bits and the target UART */
    regindex = regnum & 0x7;
    if (hUart == 1)
        regindex += 8;
    
    /* If regnum between 0x08 and 0x0F, set bit 7 of LCR to access register */
    if ((regnum & 0x18) == 0x8)
    {
        lcr = VMD642_UART_rget(hUart, VMD642_UART_LCR);
        VMD642_UART_rset(hUart, VMD642_UART_LCR, lcr | 0x80);
        returnval = VMD642_rget(regindex);
        VMD642_UART_rset(hUart, VMD642_UART_LCR, lcr);
    }
    else
    {
        returnval = VMD642_rget(regindex);
    }
    
    return returnval;
}


/**************************************************************************/
/*  ======== VMD642_UART_open ========                                 */
/*  Initialize UART and return handle									  */
/*	描述：打开相应的串口，并进行相应的配置								  */
/*	输入参数：														      */
/*		devid:	选择UART的通路											  */
/*		baudrate: 选择正确的波待率										  */
/*	    config:   其它配置其它的参数组									  */
/**************************************************************************/
VMD642_UART_Handle VMD642_UART_open(Int16 devid, 
										Int16 baudrate, 
										VMD642_UART_Config *config)
{
    VMD642_UART_Handle hUart;
    Int16 dl = 0;
    /* Assign handle */
    hUart = devid;
    
    /* Set registers based on config structure */

    /*设置MCR*/
    VMD642_UART_rset(hUart, VMD642_UART_MCR,config -> regs[3]);
    /*将EFR寄存器的第4位关闭*/
    VMD642_UART_rset(hUart, VMD642_UART_EFR, dl);
    
    /*设置串口的MCR*/
    VMD642_UART_rset(hUart, VMD642_UART_MCR, config -> regs[3]);
    
    /*设置串口的IER*/
    VMD642_UART_rset(hUart, VMD642_UART_IER, config -> regs[0]);
    /*设置串口的FCR*/
    VMD642_UART_rset(hUart, VMD642_UART_FCR, config -> regs[1]);	
    /*设置串口的LCR*/
    VMD642_UART_rset(hUart, VMD642_UART_LCR, config -> regs[2]);

    /* Set up baud divisor clock，设置波待率 */
    dl = VMD642_UART_rget(hUart, VMD642_UART_MCR);
    if((dl & 0x80)==0x80)
    {
    	baudrate = (baudrate/4);	
    }
    VMD642_UART_rset(hUart, VMD642_UART_DLL, baudrate & 0xff);
    dl = VMD642_UART_rget(hUart, VMD642_UART_DLL);
    VMD642_UART_rset(hUart, VMD642_UART_DLH, (baudrate >> 8) & 0xff);
    dl = VMD642_UART_rget(hUart, VMD642_UART_DLH);
    /* Clear any outstanding receive characters，清空接收寄存器 */
    VMD642_UART_rget(hUart, VMD642_UART_RBR);
    
    return hUart;
}

/*********************************************************************/
/*  ======== VMD642_UART_getChar ========							 */
/*  Get one character of data from the UART,读取数据			     */
/*********************************************************************/
Int16 VMD642_UART_getChar(VMD642_UART_Handle hUart)
{
    Int16 status;
    
    while(1)
    {
        status = VMD642_UART_rget(hUart, VMD642_UART_LSR);
        if ((status & 1) != 0)  // DR
            break;
    }
    
    return VMD642_UART_rget(hUart, VMD642_UART_RBR);
}

/*********************************************************************/
/*  ======== VMD642_UART_putChar ========				     		 */
/*  Send one character of data to the UART,发送一个数据			     */
/*********************************************************************/
void VMD642_UART_putChar(VMD642_UART_Handle hUart, Uint16 data)
{
    Int16 status;
    
    while(1)
   {
        status = VMD642_UART_rget(hUart, VMD642_UART_LSR);
        if ((status & 0x20) != 0)  // THRE
            break;
    }

    VMD642_UART_rset(hUart, VMD642_UART_THR, data);

}

