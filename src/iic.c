/***********************************************************/
/*  Copyright 2006 by Visioin Magic Ltd.				   */
/*  All rights reserved.  								   */ 
/*  											           */
/***********************************************************/
#include "iic.h"      

#define I2CDELAY(iterations)  {      \
    volatile Int j;                  \
    for(j = 0; j < iterations; j ++); \
}   
#define DELAY_TIME 0x4000
/*VMD642的IIC的设置结构*/
I2C_Config VMD642IIC_Configt = {
    0,  /* master mode,  i2coar;采用主模式   */
    0,  /* no interrupt, i2cimr;只写，不读，采用无中断方式*/
    (20-5), /* scl low time, i2cclkl;  */
    (20-5), /* scl high time,i2cclkh;  */
    2,  /* configure later, i2ccnt;*/
    0,  /* configure later, i2csar;*/
    0x4620, /* master tx mode,     */
            /* i2c runs free,      */
            /* 8-bit data + NACK   */
            /* no repeat mode      */
    (75-1), /* 2MHz clock, i2cpsc  */
};

I2C_Config VMD642IIC_Configr = {
    0,  /* master mode,  i2coar;采用主模式   */
    0,  /* no interrupt, i2cimr;只写，不读，采用无中断方式*/
    (20-5), /* scl low time, i2cclkl;  */
    (20-5), /* scl high time,i2cclkh;  */
    1,  /* configure later, i2ccnt;*/
    0,  /* configure later, i2csar;*/
    0x4420, /* master tx mode,     */
            /* i2c runs free,      */
            /* 8-bit data + NACK   */
            /* no repeat mode      */
    (75-1), /* 2MHz clock, i2cpsc  */
};


/*
 * ======== _IIC_write ========
 * This function performs write operation via I2C bus.
 */
/* Spin in a delay loop for delay iterations */
void EVMDM642_wait(Uint32 delay)
{
    volatile Uint32 i, n;
    
    n = 0;
    for (i = 0; i < delay; i++)
    {
        n = n + 1;
    }
}

/* Spin in a delay loop for delay microseconds */
void EVMDM642_waitusec(Uint32 delay)
{
    EVMDM642_wait(delay * 21);
}

void _IIC_write(I2C_Handle hI2C,
              Uint8 devAddress,
              Uint32  subAddress,
              Uint8 data
              )
{             
    I2C_Config prevI2CCfg;
    
    /* Wait until bus is free */
    while (I2C_bb(hI2C));
    
    /* Save old settings */
    I2C_getConfig(hI2C, &prevI2CCfg);
    
    /* Restore settings for AIC23 */
    VMD642IIC_Configt.i2csar = devAddress;
    I2C_config(hI2C, &VMD642IIC_Configt);

    /* Submit the MSB for transmit */
    I2C_RSETH(hI2C, I2CDXR, (subAddress) & 0xff);
    
    /* Generate start condition, starts transmission */
    I2C_start(hI2C);
    
    /* Wait until MSB transmit is done */
    while(!I2C_xrdy(hI2C));

    /* Submit the LSB for transmit */ 
    I2C_RSETH(hI2C, I2CDXR,data);    
 //   EVMDM642_waitusec(350);
    
    /* Generate stop condition */
    I2C_sendStop(hI2C);  

    /* Wait until bus is free */
    while (I2C_bb(hI2C));
            
    EVMDM642_waitusec(350);

    /* Reconfigure I2C with old settings */
    I2C_config(hI2C, &prevI2CCfg);              
}     

/*
 * ======== _IIC_read ========
 * This function performs read from operation via I2C bus.
 */

void _IIC_read(I2C_Handle hI2C,
              Uint8 devAddress,
              Uint32  subAddress,
              Uint8 *data
              )
{
    I2C_Config prevI2CCfg;
    
    /* Wait until bus is free */
    while (I2C_bb(hI2C));
    
    /* Save old settings */
    I2C_getConfig(hI2C, &prevI2CCfg);
    
    /* Restore settings for AIC23 */
    VMD642IIC_Configt.i2csar = devAddress;
    I2C_config(hI2C, &VMD642IIC_Configt);

    /* Submit the MSB for transmit */
    I2C_RSETH(hI2C, I2CDXR, (subAddress) & 0xff);
    
    /* Generate start condition, starts transmission */
    I2C_start(hI2C);
    
    /* Wait until MSB transmit is done */
    while(!I2C_xrdy(hI2C));
        
    /* Generate stop condition */
    I2C_sendStop(hI2C);  
	EVMDM642_waitusec(20);

    /* Reconfigure I2C with old settings */
    I2C_config(hI2C, &prevI2CCfg);
  	I2C_getConfig(hI2C, &prevI2CCfg);
    /*从发送到接收需一段时间转换*/
    EVMDM642_waitusec(0x200);
    /* Restore settings for AIC23 */
    VMD642IIC_Configr.i2csar = devAddress;
    I2C_config(hI2C, &VMD642IIC_Configr);
    /* Generate start condition, starts transmission */
    I2C_start(hI2C);
    while(I2C_FGETH(hI2C,I2CSTR,ARDY));
    /* Wait until MSB transmit is done */
    while(!I2C_rrdy(hI2C));

    /* Submit the MSB for transmit */
    *data = I2C_RGETH(hI2C, I2CDRR);
        
    /* Generate stop condition */
    I2C_sendStop(hI2C);  
    
    /* Wait until bus is free */
    while (I2C_bb(hI2C)); 

    /* Short delay for AIC23 to accept command */        
    EVMDM642_waitusec(20);

    /* Reconfigure I2C with old settings */
    I2C_config(hI2C, &prevI2CCfg);          
}

