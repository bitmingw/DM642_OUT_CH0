/********************************************************************/
/* Output video of Three-frame difference method                    */
/* Author: Ming Wen                                                 */
/********************************************************************/

/********************************************************************/
/*  Copyright 2006 by Vision Magic Ltd.								*/
/*  Restricted rights to use, duplicate or disclose this code are	*/
/*  granted through contract.									    */
/*  															    */
/********************************************************************/

#include <csl.h>
#include <csl_emifa.h>
#include <csl_i2c.h>
#include <csl_gpio.h>
#include <csl_irq.h>
#include <csl_chip.h>
#include <csl_dat.h>
#include "iic.h"
#include "vportcap.h"
#include "vportdis.h"
#include "sa7121h.h"
#include "TVP51xx.h"

/********************************************************************/

/*VMD642µÄemifaµÄÉèÖÃ½á¹¹*/
EMIFA_Config VMDEMIFConfig ={
	   0x00052078,/*gblctl EMIFA(B)global control register value */
	   			  /*½«CLK6¡¢4¡¢1Ê¹ÄÜ£»½«MRMODEÖÃ1£»Ê¹ÄÜEK2EN,EK2RATE*/
	   0xffffffd3,/*cectl0 CE0 space control register value*/
	   			  /*½«CE0¿Õ¼äÉèÎªSDRAM*/
	   0x73a28e01,/*cectl1 CE1 space control register value*/
	   			  /*Read hold: 1 clock;
	   			    MTYPE : 0000,Ñ¡Ôñ8Î»µÄÒì²½½Ó¿Ú
	   			    Read strobe £º001110£»14¸öclock¿í¶È
	   			    TA£º2 clock; Read setup 2 clock;
	   			    Write hold :2 clock; Write strobe: 14 clock
	   			    Write setup :7 clock
	   			    --					 ---------------
	   			  	  \		 14c		/1c
	   			 	   \----------------/ */
	   0x22a28a22, /*cectl2 CE2 space control register value*/
       0x22a28a42, /*cectl3 CE3 space control register value*/
	   0x57226000, /*sdctl SDRAM control register value*/
	   0x0000081b, /*sdtim SDRAM timing register value*/
	   0x001faf4d, /*sdext SDRAM extension register value*/
	   0x00000002, /*cesec0 CE0 space secondary control register value*/
	   0x00000002, /*cesec1 CE1 space secondary control register value*/
	   0x00000002, /*cesec2 CE2 space secondary control register value*/
	   0x00000073 /*cesec3 CE3 space secondary control register value*/	
};

/*VMD642µÄIICµÄÉèÖÃ½á¹¹*/
I2C_Config VMD642IIC_Config = {
    0,  /* master mode,  i2coar;²ÉÓÃÖ÷Ä£Ê½   */
    0,  /* no interrupt, i2cimr;Ö»Ð´£¬²»¶Á£¬²ÉÓÃÎÞÖÐ¶Ï·½Ê½*/
    (20-5), /* scl low time, i2cclkl;  */
    (20-5), /* scl high time,i2cclkh;  */
    1,  /* configure later, i2ccnt;*/
    0,  /* configure later, i2csar;*/
    0x4ea0, /* master tx mode,     */
            /* i2c runs free,      */
            /* 8-bit data + NACK   */
            /* no repeat mode      */
    (75-1), /* 4MHz clock, i2cpsc  */
};

CHIP_Config VMD642percfg = {
	CHIP_VP2+\
	CHIP_VP1+\
	CHIP_VP0+\
	CHIP_I2C
};

I2C_Handle hVMD642I2C;
int portNumber;
extern SA7121H_ConfParams sa7121hPAL[45];
extern SA7121H_ConfParams sa7121hNTSC[45];
Uint8 vFromat = 0;
Uint8 misc_ctrl = 0x6D;
Uint8 output_format = 0x47;
// µØÖ·Îª0 for cvbs port1,Ñ¡Ôñ¸´ºÏÐÅºÅ×öÎªÊäÈë
Uint8 input_sel = 0x00;
/*µØÖ·Îª0xf£¬½«Pin27ÉèÖÃ³ÉÎªCAPEN¹¦ÄÜ*/	
Uint8 pin_cfg = 0x02;
/*µØÖ·Îª1B*/
Uint8 chro_ctrl_2 = 0x14;
/*Í¼Ïñ¾ä±úµÄÉùÃ÷*/
VP_Handle vpHchannel0;
VP_Handle vpHchannel1;
VP_Handle vpHchannel2;

/********************************************************************/

/*È·¶¨Í¼ÏñµÄ²ÎÊý*/
int numPixels = 720;//Ã¿ÐÐ720¸öÏñËØ
int numLines  = 576;//Ã¿Ö¡576ÐÐ£¨PAL£©

/*»º´æ´óÐ¡µÄ¼ÆËã YÍ¨µÀ 720 * 588
Cb Óë Cr Í¨µÀ 720 * 294 */

/*SDRAM µØÖ· 0x80000000 - 0x81FFFFFF*/
/*×¢Òâ¸ø¶¨µÄµØÖ·ÓëcmdÎÄ¼þÖÐÃèÊöµÄµØÖ·Ò»ÖÂ*/

/*µ±Ç°Í¼ÏñÊ×µØÖ·£¬¿Õ¼äÒÑÔÚ vportcap.c ÖÐ·ÖÅä*/
Uint32 capYbuffer  = 0x80000000;
Uint32 capCbbuffer = 0x800675c0;
Uint32 capCrbuffer = 0x8009b0a0;

/*·ÖÅä¿Õ¼ä£¬¼ÇÂ¼µÚÒ»Ö¡Í¼ÏñÊ×µØÖ·*/
#pragma DATA_SECTION(ChaAYSpace1, ".ChaAYSpace1") 
Uint8 ChaAYSpace1[720*588]; 
#pragma DATA_SECTION(ChaACbSpace1, ".ChaACbSpace1")
Uint8 ChaACbSpace1[360*588]; 
#pragma DATA_SECTION(ChaACrSpace1, ".ChaACrSpace1")
Uint8 ChaACrSpace1[360*588];

Uint32 Ybuffer1  = 0x80100000;
Uint32 Cbbuffer1 = 0x801675c0;
Uint32 Crbuffer1 = 0x8019b0a0;

/*·ÖÅä¿Õ¼ä£¬¼ÇÂ¼µÚ¶þÖ¡Í¼ÏñÊ×µØÖ·*/
#pragma DATA_SECTION(ChaAYSpace2, ".ChaAYSpace2") 
Uint8 ChaAYSpace2[720*588]; 
#pragma DATA_SECTION(ChaACbSpace2, ".ChaACbSpace2")
Uint8 ChaACbSpace2[360*588]; 
#pragma DATA_SECTION(ChaACrSpace2, ".ChaACrSpace2")
Uint8 ChaACrSpace2[360*588];

Uint32 Ybuffer2  = 0x80200000;
Uint32 Cbbuffer2 = 0x802675c0;
Uint32 Crbuffer2 = 0x8029b0a0;

/*·ÖÅä¿Õ¼ä£¬¼ÇÂ¼µÚÈýÖ¡Í¼ÏñÊ×µØÖ·*/
#pragma DATA_SECTION(ChaAYSpace3, ".ChaAYSpace3") 
Uint8 ChaAYSpace3[720*588]; 
#pragma DATA_SECTION(ChaACbSpace3, ".ChaACbSpace3")
Uint8 ChaACbSpace3[360*588]; 
#pragma DATA_SECTION(ChaACrSpace3, ".ChaACrSpace3")
Uint8 ChaACrSpace3[360*588];

Uint32 Ybuffer3  = 0x80300000;
Uint32 Cbbuffer3 = 0x803675c0;
Uint32 Crbuffer3 = 0x8039b0a0;

/*·ÖÅä¿Õ¼ä£¬¼ÇÂ¼µÚÒ»¶þÖ¡²îÍ¼ÏñÊ×µØÖ·*/
#pragma DATA_SECTION(ChaAYSpaceDiff12, ".ChaAYSpaceDiff12") 
Uint8 ChaAYSpaceDiff12[720*588]; 
#pragma DATA_SECTION(ChaACbSpaceDiff12, ".ChaACbSpaceDiff12")
Uint8 ChaACbSpaceDiff12[360*588]; 
#pragma DATA_SECTION(ChaACrSpaceDiff12, ".ChaACrSpaceDiff12")
Uint8 ChaACrSpaceDiff12[360*588];

Uint32 YbufferDiff12  = 0x80400000;
Uint32 CbbufferDiff12 = 0x804675c0;
Uint32 CrbufferDiff12 = 0x8049b0a0;

/*·ÖÅä¿Õ¼ä£¬¼ÇÂ¼µÚ¶þÈýÖ¡²îÍ¼ÏñÊ×µØÖ·*/
#pragma DATA_SECTION(ChaAYSpaceDiff23, ".ChaAYSpaceDiff23") 
Uint8 ChaAYSpaceDiff23[720*588]; 
#pragma DATA_SECTION(ChaACbSpaceDiff23, ".ChaACbSpaceDiff23")
Uint8 ChaACbSpaceDiff23[360*588]; 
#pragma DATA_SECTION(ChaACrSpaceDiff23, ".ChaACrSpaceDiff23")
Uint8 ChaACrSpaceDiff23[360*588];

Uint32 YbufferDiff23  = 0x80500000;
Uint32 CbbufferDiff23 = 0x805675c0;
Uint32 CrbufferDiff23 = 0x8059b0a0;

/*ÏÔÊ¾Í¼ÏñÊ×µØÖ·£¬¿Õ¼äÒÑÔÚ vportdis.c ÖÐ·ÖÅä*/
Uint32 disYbuffer  = 0x81000000;
Uint32 disCbbuffer = 0x810675c0; 
Uint32 disCrbuffer = 0x8109b0a0;

/*Í¼Ïñ¸ñÊ½±êÖ¾*/
Uint8 NTSCorPAL = 0;
extern far void vectors();
extern volatile Uint32 capNewFrame;
extern volatile Uint32 disNewFrame;


/********************************************************************/

/*´Ë³ÌÐò¿É½«ÊÓÆµ²É¼¯¿Ú1 CH1(µÚ¶þ¸öÍ¨µÀ)µÄÊý¾Ý¾­¹ýVideo Port0ËÍ³ö*/
void main()
{
	Uint8 addrI2C;
	int i;

	/*The next position to store a frame*/
	Uint8 nextFrame;
	/* Current Buf addr */
	Uint32 YBuf;
	Uint32 CbBuf;
	Uint32 CrBuf;

/*-------------------------------------------------------*/
/* perform all initializations                           */
/*-------------------------------------------------------*/
	/*Initialise CSL£¬³õÊ¼»¯CSL¿â*/
	CSL_init();
	CHIP_config(&VMD642percfg);
/*----------------------------------------------------------*/
	/*EMIFAµÄ³õÊ¼»¯£¬½«CE0ÉèÎªSDRAM¿Õ¼ä£¬CE1ÉèÎªÒì²½¿Õ¼ä
	 ×¢£¬DM642Ö§³ÖµÄÊÇEMIFA£¬¶ø·ÇEMIF*/
	EMIFA_config(&VMDEMIFConfig);
/*----------------------------------------------------------*/
	/*ÖÐ¶ÏÏòÁ¿±íµÄ³õÊ¼»¯*/
	//Point to the IRQ vector table
    IRQ_setVecs(vectors);
    IRQ_nmiEnable();
    IRQ_globalEnable();
    IRQ_map(IRQ_EVT_VINT1, 11);
    IRQ_map(IRQ_EVT_VINT0, 12);
    IRQ_reset(IRQ_EVT_VINT1);
    IRQ_reset(IRQ_EVT_VINT0);
    /*´ò¿ªÒ»¸öÊý¾Ý¿½±´µÄÊý¾ÝÍ¨Â·*/
    DAT_open(DAT_CHAANY, DAT_PRI_LOW, DAT_OPEN_2D);	

/*----------------------------------------------------------*/	
	/*½øÐÐIICµÄ³õÊ¼»¯*/
	hVMD642I2C = I2C_open(I2C_PORT0, I2C_OPEN_RESET);
	I2C_config(hVMD642I2C, &VMD642IIC_Config);

/*----------------------------------------------------------*/
	/*½øÐÐTVP5150pbsµÄ³õÊ¼»¯*/
	/*Ñ¡ÔñTVP5150£¬ÉèÖÃÊÓÆµ²É¼¯µÚÒ»Í¨Â·ch0, ¼´U12*/

	/*½«GPIO0²»×öÎªGPINTÊ¹ÓÃ*/
	GPIO_RSET(GPGC, 0x0);

	/*½«GPIO0×öÎªÊä³ö*/
	GPIO_RSET(GPDIR, 0x1);

	/*GPIO0Êä³öÎª¸ß£¬Ñ¡ÔñIIC0×ÜÏß*/
	GPIO_RSET(GPVAL, 0x0);

	/*Ñ¡ÔñµÚ¶þ¸ö5150£¬U12*/
	addrI2C = 0xBA >> 1;
    _IIC_write(hVMD642I2C, addrI2C, 0x00, input_sel);
    _IIC_write(hVMD642I2C, addrI2C, 0x03, misc_ctrl);
    _IIC_write(hVMD642I2C, addrI2C, 0x0D, output_format);
    _IIC_write(hVMD642I2C, addrI2C, 0x0F, pin_cfg);
    _IIC_write(hVMD642I2C, addrI2C, 0x1B, chro_ctrl_2);
    /*»ØÁµ±Ç°ÉãÏñÉè±¸µÄ¸ñÊ½*/
    _IIC_read(hVMD642I2C, addrI2C, 0x8c, &vFromat);
    vFromat = vFromat & 0xff;
	switch (vFromat)
	{
		case TVP51XX_NTSCM:
		case TVP51XX_NTSC443:
			NTSCorPAL = 1;/*ÏµÍ³ÎªNTSCµÄÄ£Ê½*/
			break;
		case TVP51XX_PALBGHIN:
		case TVP51XX_PALM:
		case TVP5150_FORCED_PAL:
			NTSCorPAL = 0;/*ÏµÍ³ÎªPALµÄÄ£Ê½*/
			break;
		default:
			NTSCorPAL = 2;/*ÏµÍ³Îª²»Ö§³ÖµÄÄ£Ê½*/
			break;
	}
	if(NTSCorPAL == 2)
	{
		/*ÏµÍ³²»Ö§³ÖµÄÄ£Ê½£¬ÖØÐÂÅäÖÃ*/
		for(;;)
		{}
	}  
			  
/*----------------------------------------------------------*/	
	/*½øÐÐSAA7121HµÄ³õÊ¼»¯*/

	/*GPIO0Êä³öÎªµÍ£¬Ñ¡ÔñIIC0×ÜÏß£¬ÅäÖÃÍ¼ÏñÊä³ö*/				
	GPIO_RSET(GPVAL, 0x0);
	/*Ñ¡ÔñµÚÒ»¸ö5150£¬¼´U10*/
	addrI2C = 0xB8 >> 1; 
	/*½«Video Port0µÄÊÓÆµÊäÈë¿ÚµÄÊý¾Ý¿ÚÉèÎª¸ß×è×´Ì¬£¬
	  Ê¹ÄÜSCLK£¬½«µÚ27½ÅÉèÎªÊäÈë*/
	_IIC_write(hVMD642I2C, addrI2C, 0x03, 0x1);

	/*ÅäÖÃSAA7121H*/
	/*GPIO0Êä³öÎªµÍ£¬Ñ¡ÔñIIC1×ÜÏß£¬ÅäÖÃÍ¼ÏñÊä³ö*/	
	GPIO_RSET(GPVAL, 0x1);

	/*³õÊ¼»¯Video Port0*/
	/*½«Video Port0ÉèÎªencoderÊä³ö*/
	portNumber = 0;
	vpHchannel0 = bt656_8bit_ncfd(portNumber);

	addrI2C = 0x88 >> 1;					      
	for(i=0; i<43; i++)
	{
		if(NTSCorPAL == 1)
		{
			_IIC_write(hVMD642I2C, 
					   addrI2C,
					   (sa7121hNTSC[i].regsubaddr), 
					   (sa7121hNTSC[i].regvule));
		}
		else
		{
			_IIC_write(hVMD642I2C, 
					   addrI2C,
					   (sa7121hPAL[i].regsubaddr), 
					   (sa7121hPAL[i].regvule));	
		}		
	}
	
/*----------------------------------------------------------*/
	/*³õÊ¼»¯Video Port1*/
	/*½«Video Port1ÉèÎª²É¼¯ÊäÈë*/
	portNumber = 1;
	vpHchannel1 = bt656_8bit_ncfc(portNumber);

/*----------------------------------------------------------*/
	/*Æô¶¯²É¼¯Ä£¿é*/
	bt656_capture_start(vpHchannel1);

	/*µÚÒ»´ÎÔËÐÐ£¬²É¼¯ÈýÖ¡Êý¾Ý*/
	for (nextFrame = 1; nextFrame <= 3; nextFrame++)
	{
		/*µÈ´ýÒ»Ö¡Êý¾Ý²É¼¯Íê³É*/
		while(capNewFrame == 0){}
		/*½«Êý¾Ý´æÈëÏÔÊ¾»º³åÇø£¬²¢Çå²É¼¯Íê³ÉµÄ±êÖ¾*/
		capNewFrame = 0;

		switch (nextFrame)
		{
			case 1: 
				YBuf = Ybuffer1;
				CbBuf = Cbbuffer1;
				CrBuf = Crbuffer1;
				break;
			case 2: 
				YBuf = Ybuffer2;
				CbBuf = Cbbuffer2;
				CrBuf = Crbuffer2;
				break;
			case 3: 
				YBuf = Ybuffer3;
				CbBuf = Cbbuffer3;
				CrBuf = Crbuffer3;
				break;		
		}
		for(i=0; i<numLines; i++)
		{
			/*´«ËÍY»º³åÇø*/
			DAT_copy((void *)(capYbuffer + i * numPixels), 
		             (void *)(YBuf + i * numPixels),
		             numPixels);
		    /*´«ËÍCb»º³åÇø*/
		    DAT_copy((void *)(capCbbuffer + i * (numPixels >> 1)), 
		             (void *)(CbBuf + i * (numPixels >> 1)),
		             numPixels>>1);
			/*´«ËÍCr»º³åÇø*/
		    DAT_copy((void *)(capCrbuffer + i * (numPixels >> 1)), 
		             (void *)(CrBuf + i * (numPixels >> 1)),
		             numPixels>>1);
		}
	}


	/*Æô¶¯ÏÔÊ¾Ä£¿é*/
	bt656_display_start(vpHchannel0);
	/*½¨Á¢ÊµÊ±¼ÆËãÑ­»·*/
	for(;;)
	{
		/*µ±²É¼¯ÇøµÄÊý¾ÝÒÑ¾­²É¼¯ºÃ£¬¶øÏÔÊ¾»º³åÇøµÄÊý¾ÝÒÑ¿Õ*/
		if((capNewFrame == 1)&&(disNewFrame == 1))
		{
			/*½«Êý¾Ý×°ÈëÏÔÊ¾»º³åÇø£¬²¢Çå²É¼¯Íê³ÉµÄ±êÖ¾*/
			capNewFrame =0;
			disNewFrame =0;
			for(i=0;i<numLines;i++)
			{
				/*´«ËÍY»º³åÇø*/
				DAT_copy((void *)(capYbuffer + i * numPixels), 
			             (void *)(disYbuffer + i * numPixels),
			             numPixels);
			    /*´«ËÍCb»º³åÇø*/ 
			    DAT_copy((void *)(capCbbuffer + i * (numPixels >> 1)), 
			             (void *)(disCrbuffer + i * (numPixels >> 1)),
			             numPixels>>1);
				/*´«ËÍCr»º³åÇø*/
			    DAT_copy((void *)(capCrbuffer + i * (numPixels >> 1)), 
			             (void *)(disCrbuffer + i * (numPixels >> 1)),
			             numPixels>>1);
			 }
		}
	}
}     
