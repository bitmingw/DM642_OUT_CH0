/*
 *  Copyright 2006 by VisionMagic Ltd.
 *  All rights reserved. Property of VisionMagic Ltd.
 */
 
/*
 *  ======== vmd642_uart.h ========
 *
 *  UART interface on the VMD642-A board
 */
#ifndef __VMD642_UART_INCLUDED__
#define __VMD642_UART_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#include <csl.h>

#define VMD642_UART_CONFIGREGS      4
#define VMD642_UART_BASEADDR        0x9c00
#define VMD642_UART_RBR             0x00  // Read
#define VMD642_UART_THR             0x00  // Write
#define VMD642_UART_IER             0x01
#define VMD642_UART_IIR             0x02  // Read
#define VMD642_UART_FCR             0x02  // Write
#define VMD642_UART_LCR             0x03
#define VMD642_UART_MCR             0x04
#define VMD642_UART_LSR             0x05
#define VMD642_UART_SCR             0x07

#define VMD642_UART_DLL             0x08
#define VMD642_UART_DLH             0x09
#define VMD642_UART_EFR             0x0A
#define VMD642_UART_XON1            0x0C
#define VMD642_UART_XON2            0x0D
#define VMD642_UART_XOFF1           0x0E
#define VMD642_UART_XOFF2           0x0F

#define VMD642_UART_TCR             0x16
#define VMD642_UART_TLR             0x17

#define VMD642_UART_FIFORDY         0x1f

#define VMD642_UARTA                0
#define VMD642_UARTB                1

//#define CLOCK_RATE_30M72
#define CLOCK_RATE_20M
//#define CLOCK_RATE_3M68

#if 0

/* TL16C550C UART baud rates - 29.4912MHz */
typedef enum {
    UARTHW_VMD642_BAUD_50             = 36864,
    UARTHW_VMD642_BAUD_75             = 24576,
    UARTHW_VMD642_BAUD_110            = 16756,
    UARTHW_VMD642_BAUD_134_5          = 13704,
    UARTHW_VMD642_BAUD_150            = 12288,
    UARTHW_VMD642_BAUD_300            = 6144,
    UARTHW_VMD642_BAUD_600            = 3072,
    UARTHW_VMD642_BAUD_1200           = 1536,
    UARTHW_VMD642_BAUD_1800           = 1024,
    UARTHW_VMD642_BAUD_2000           = 922,
    UARTHW_VMD642_BAUD_2400           = 768,
    UARTHW_VMD642_BAUD_3600           = 512,
    UARTHW_VMD642_BAUD_4800           = 384,
    UARTHW_VMD642_BAUD_7200           = 256,
    UARTHW_VMD642_BAUD_9600           = 192,
    UARTHW_VMD642_BAUD_19200          = 96,
    UARTHW_VMD642_BAUD_38400          = 48,
    UARTHW_VMD642_BAUD_57600          = 32,
    UARTHW_VMD642_BAUD_115200         = 16,
    UARTHW_VMD642_BAUD_230400         = 8
} UARTHW_VMD642_Baud;

#else

#if defined(CLOCK_RATE_30M72)
/* clock: - 30.72MHz */
typedef enum {
    UARTHW_VMD642_BAUD_50             = 38400,
    UARTHW_VMD642_BAUD_75             = 25600,
    UARTHW_VMD642_BAUD_110            = 17450,
    UARTHW_VMD642_BAUD_134_5          = 14280,
    UARTHW_VMD642_BAUD_150            = 12800,
    UARTHW_VMD642_BAUD_300            = 6400,
    UARTHW_VMD642_BAUD_600            = 3200,
    UARTHW_VMD642_BAUD_1200           = 1600,
    UARTHW_VMD642_BAUD_1800           = 1070,
    UARTHW_VMD642_BAUD_2000           = 960,
    UARTHW_VMD642_BAUD_2400           = 800,
    UARTHW_VMD642_BAUD_3600           = 530,
    UARTHW_VMD642_BAUD_4800           = 400,
    UARTHW_VMD642_BAUD_7200           = 270,
    UARTHW_VMD642_BAUD_9600           = 200,
    UARTHW_VMD642_BAUD_19200          = 100,
    UARTHW_VMD642_BAUD_38400          = 50,
    UARTHW_VMD642_BAUD_57600          = 33,
    UARTHW_VMD642_BAUD_115200         = 17,
    UARTHW_VMD642_BAUD_230400         = 8
} UARTHW_VMD642_Baud;

#elif defined(CLOCK_RATE_20M)
/* clock: - 20MHz */
typedef enum 
{
    UARTHW_VMD642_BAUD_50             = 24960,
    UARTHW_VMD642_BAUD_75             = 16640,
    UARTHW_VMD642_BAUD_110            = 11345,
    UARTHW_VMD642_BAUD_150            = 8320,
    UARTHW_VMD642_BAUD_300            = 4160,
    UARTHW_VMD642_BAUD_600            = 2080,
    UARTHW_VMD642_BAUD_1200           = 1040,
    UARTHW_VMD642_BAUD_1800           = 694,
    UARTHW_VMD642_BAUD_2000           = 624,
    UARTHW_VMD642_BAUD_2400           = 520,
    UARTHW_VMD642_BAUD_3600           = 390,
    UARTHW_VMD642_BAUD_4800           = 260,
    UARTHW_VMD642_BAUD_7200           = 195,
    UARTHW_VMD642_BAUD_9600           = 130,
    UARTHW_VMD642_BAUD_19200          = 65,
    UARTHW_VMD642_BAUD_38400          = 32,
    UARTHW_VMD642_BAUD_57600          = 22,
    UARTHW_VMD642_BAUD_115200         = 11,
    UARTHW_VMD642_BAUD_230400         = 5
} UARTHW_VMD642_Baud;

#endif

#endif


/* UART handle */
typedef Int16 VMD642_UART_Handle;

/* Config structure for the VMDM642 UART */
typedef struct VMD642_UART_Config {
    int regs[VMD642_UART_CONFIGREGS];
} VMD642_UART_Config;

/* Set a UART register */
extern void VMD642_UART_rset(VMD642_UART_Handle hUart, Int16 regnum, Int16 regval);

/* Get the value of a UART register */
extern Int16 VMD642_UART_rget(VMD642_UART_Handle hUart, Int16 regnum);

/* Initialize UART and return handle */
extern Int16 VMD642_UART_open(Int16 devid, Int16 baudrate, VMD642_UART_Config *Config);

/* Get one character of data from the UART */
extern Int16 VMD642_UART_getChar(VMD642_UART_Handle hUart);

/* Send one character of data to the UART */
extern void VMD642_UART_putChar(VMD642_UART_Handle hUart, Uint16 data);

#ifdef __cplusplus
}
#endif

#endif

