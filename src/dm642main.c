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
#include <csl_cache.h>
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
#include "frame_operation.h"

/********************************************************************/

/*VMD642的emifa的设置结构*/
EMIFA_Config VMDEMIFConfig ={
	   0x00052078,/*gblctl EMIFA(B)global control register value */
	   			  /*将CLK6、4、1使能；将MRMODE置1；使能EK2EN,EK2RATE*/
	   0xffffffd3,/*cectl0 CE0 space control register value*/
	   			  /*将CE0空间设为SDRAM*/
	   0x73a28e01,/*cectl1 CE1 space control register value*/
	   			  /*Read hold: 1 clock;
	   			    MTYPE : 0000,选择8位的异步接口
	   			    Read strobe ：001110；14个clock宽度
	   			    TA：2 clock; Read setup 2 clock;
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

/*VMD642的IIC的设置结构*/
I2C_Config VMD642IIC_Config = {
    0,  /* master mode,  i2coar;采用主模式   */
    0,  /* no interrupt, i2cimr;只写，不读，采用无中断方式*/
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
// 地址为0 for cvbs port1,选择复合信号做为输入
Uint8 input_sel = 0x00;
/*地址为0xf，将Pin27设置成为CAPEN功能*/
Uint8 pin_cfg = 0x02;
/*地址为1B*/
Uint8 chro_ctrl_2 = 0x14;
/*图像句柄的声明*/
VP_Handle vpHchannel0;
VP_Handle vpHchannel1;
VP_Handle vpHchannel2;

/********************************************************************/

/*确定图像的参数*/
int numPixels = 720;//每行720个像素
int numLines  = 576;//每帧576行（PAL）

/*声明缓存空间*/
#pragma DATA_ALIGN(CACHE_A, CACHE_L2_LINESIZE)
#pragma DATA_ALIGN(CACHE_B, CACHE_L2_LINESIZE)
#pragma DATA_ALIGN(CACHE_S, CACHE_L2_LINESIZE)
#pragma DATA_SECTION(CACHE_A, ".cache")
#pragma DATA_SECTION(CACHE_B, ".cache")
#pragma DATA_SECTION(CACHE_S, ".cache")
Uint8 CACHE_A[720];
Uint8 CACHE_B[720];
Uint8 CACHE_S[720];

/*缓存大小的计算 Y通道 720 * 588
Cb 与 Cr 通道 720 * 294 */

/*SDRAM 地址 0x80000000 - 0x81FFFFFF*/
/*注意给定的地址与cmd文件中描述的地址一致*/

/*当前图像首地址，空间已在 vportcap.c 中分配*/
Uint32 capYbuffer  = 0x80000000;
Uint32 capCbbuffer = 0x800675c0;
Uint32 capCrbuffer = 0x8009b0a0;

/*分配空间，记录第一帧图像首地址*/
#pragma DATA_SECTION(ChaAYSpace1, ".ChaAYSpace1")
Uint8 ChaAYSpace1[720*588];
#pragma DATA_SECTION(ChaACbSpace1, ".ChaACbSpace1")
Uint8 ChaACbSpace1[360*588];
#pragma DATA_SECTION(ChaACrSpace1, ".ChaACrSpace1")
Uint8 ChaACrSpace1[360*588];

Uint32 Ybuffer1  = 0x80100000;
Uint32 Cbbuffer1 = 0x801675c0;
Uint32 Crbuffer1 = 0x8019b0a0;

/*分配空间，记录第二帧图像首地址*/
#pragma DATA_SECTION(ChaAYSpace2, ".ChaAYSpace2")
Uint8 ChaAYSpace2[720*588];
#pragma DATA_SECTION(ChaACbSpace2, ".ChaACbSpace2")
Uint8 ChaACbSpace2[360*588];
#pragma DATA_SECTION(ChaACrSpace2, ".ChaACrSpace2")
Uint8 ChaACrSpace2[360*588];

Uint32 Ybuffer2  = 0x80200000;
Uint32 Cbbuffer2 = 0x802675c0;
Uint32 Crbuffer2 = 0x8029b0a0;

/*分配空间，记录第三帧图像首地址*/
#pragma DATA_SECTION(ChaAYSpace3, ".ChaAYSpace3")
Uint8 ChaAYSpace3[720*588];
#pragma DATA_SECTION(ChaACbSpace3, ".ChaACbSpace3")
Uint8 ChaACbSpace3[360*588];
#pragma DATA_SECTION(ChaACrSpace3, ".ChaACrSpace3")
Uint8 ChaACrSpace3[360*588];

Uint32 Ybuffer3  = 0x80300000;
Uint32 Cbbuffer3 = 0x803675c0;
Uint32 Crbuffer3 = 0x8039b0a0;

/*分配空间，记录第一二帧差图像首地址*/
#pragma DATA_SECTION(ChaAYSpaceDiff12, ".ChaAYSpaceDiff12")
Uint8 ChaAYSpaceDiff12[720*588];
#pragma DATA_SECTION(ChaACbSpaceDiff12, ".ChaACbSpaceDiff12")
Uint8 ChaACbSpaceDiff12[360*588];
#pragma DATA_SECTION(ChaACrSpaceDiff12, ".ChaACrSpaceDiff12")
Uint8 ChaACrSpaceDiff12[360*588];

Uint32 YbufferDiff12  = 0x80400000;
Uint32 CbbufferDiff12 = 0x804675c0;
Uint32 CrbufferDiff12 = 0x8049b0a0;

/*分配空间，记录第二三帧差图像首地址*/
#pragma DATA_SECTION(ChaAYSpaceDiff23, ".ChaAYSpaceDiff23")
Uint8 ChaAYSpaceDiff23[720*588];
#pragma DATA_SECTION(ChaACbSpaceDiff23, ".ChaACbSpaceDiff23")
Uint8 ChaACbSpaceDiff23[360*588];
#pragma DATA_SECTION(ChaACrSpaceDiff23, ".ChaACrSpaceDiff23")
Uint8 ChaACrSpaceDiff23[360*588];

Uint32 YbufferDiff23  = 0x80500000;
Uint32 CbbufferDiff23 = 0x805675c0;
Uint32 CrbufferDiff23 = 0x8059b0a0;

/*分配空间，后处理图像首地址*/
#pragma DATA_SECTION(ChaAYSpacePost, ".ChaAYSpacePost")
Uint8 ChaAYSpacePost[720*588];
#pragma DATA_SECTION(ChaACbSpacePost, ".ChaACbSpacePost")
Uint8 ChaACbSpacePost[360*588];
#pragma DATA_SECTION(ChaACrSpacePost, ".ChaACrSpacePost")
Uint8 ChaACrSpacePost[360*588];

Uint32 YbufferPost  = 0x80600000;
Uint32 CbbufferPost = 0x806675c0;
Uint32 CrbufferPost = 0x8069b0a0;

/*显示图像首地址，空间已在 vportdis.c 中分配*/
Uint32 disYbuffer  = 0x81000000;
Uint32 disCbbuffer = 0x810675c0;
Uint32 disCrbuffer = 0x8109b0a0;

/*图像格式标志*/
Uint8 NTSCorPAL = 0;
extern far void vectors();
extern volatile Uint32 capNewFrame;
extern volatile Uint32 disNewFrame;


/********************************************************************/

/*此程序可将视频采集口1 CH1(第二个通道)的数据经过Video Port0送出*/
void main()
{
	Uint8 addrI2C;
	int i;

	/* The next position to store a frame */
	Uint8 nextFrame;
    Uint8 nextDiff;

	/* Current Buf addr */
	Uint32 YBuf, CbBuf, CrBuf;
    /* Buf addr been subtract */
    Uint32 YSubBuf, CbSubBuf, CrSubBuf;
    /* Buf addr to be add */
    Uint32 YAddBuf, CbAddBuf, CrAddBuf;
    /* Buf addr to store results */
    Uint32 YAnsBuf, CbAnsBuf, CrAnsBuf;
    
    /* Position of centroid */
    int positionX, positionY;

/*-------------------------------------------------------*/
/* perform all initializations                           */
/*-------------------------------------------------------*/
	/*Initialise CSL，初始化CSL库*/
	CSL_init();
	CHIP_config(&VMD642percfg);
/*----------------------------------------------------------*/
	/*EMIFA的初始化，将CE0设为SDRAM空间，CE1设为异步空间
	 注，DM642支持的是EMIFA，而非EMIF*/
	EMIFA_config(&VMDEMIFConfig);
/*----------------------------------------------------------*/
	/*中断向量表的初始化*/
	//Point to the IRQ vector table
    IRQ_setVecs(vectors);
    IRQ_nmiEnable();
    IRQ_globalEnable();
    IRQ_map(IRQ_EVT_VINT1, 11);
    IRQ_map(IRQ_EVT_VINT0, 12);
    IRQ_reset(IRQ_EVT_VINT1);
    IRQ_reset(IRQ_EVT_VINT0);
    /*打开一个数据拷贝的数据通路*/
    DAT_open(DAT_CHAANY, DAT_PRI_LOW, DAT_OPEN_2D);

/*----------------------------------------------------------*/
	/*进行IIC的初始化*/
	hVMD642I2C = I2C_open(I2C_PORT0, I2C_OPEN_RESET);
	I2C_config(hVMD642I2C, &VMD642IIC_Config);

/*----------------------------------------------------------*/
	/*进行TVP5150pbs的初始化*/
	/*选择TVP5150，设置视频采集第一通路ch0, 即U12*/

	/*将GPIO0不做为GPINT使用*/
	GPIO_RSET(GPGC, 0x0);

	/*将GPIO0做为输出*/
	GPIO_RSET(GPDIR, 0x1);

	/*GPIO0输出为高，选择IIC0总线*/
	GPIO_RSET(GPVAL, 0x0);

	/*选择第二个5150，U12*/
	addrI2C = 0xBA >> 1;
    _IIC_write(hVMD642I2C, addrI2C, 0x00, input_sel);
    _IIC_write(hVMD642I2C, addrI2C, 0x03, misc_ctrl);
    _IIC_write(hVMD642I2C, addrI2C, 0x0D, output_format);
    _IIC_write(hVMD642I2C, addrI2C, 0x0F, pin_cfg);
    _IIC_write(hVMD642I2C, addrI2C, 0x1B, chro_ctrl_2);
    /*读取视频格式*/
    _IIC_read(hVMD642I2C, addrI2C, 0x8c, &vFromat);
    vFromat = vFromat & 0xff;
	switch (vFromat)
	{
		case TVP51XX_NTSCM:
		case TVP51XX_NTSC443:
			NTSCorPAL = 1;/*系统为NTSC的模式*/
			break;
		case TVP51XX_PALBGHIN:
		case TVP51XX_PALM:
		case TVP5150_FORCED_PAL:
			NTSCorPAL = 0;/*系统为PAL的模式*/
			break;
		default:
			NTSCorPAL = 2;/*系统为不支持的模式*/
			break;
	}
	if(NTSCorPAL == 2)
	{
		/*系统不支持的模式，重新配置*/
		for(;;)
		{}
	}

/*----------------------------------------------------------*/
	/*进行SAA7121H的初始化*/

	/*GPIO0输出为低，选择IIC0总线，配置图像输出*/
	GPIO_RSET(GPVAL, 0x0);
	/*选择第一个5150，即U10*/
	addrI2C = 0xB8 >> 1;
	/*将Video Port0的视频输入口的数据口设为高阻状态，
	  使能SCLK，将第27脚设为输入*/
	_IIC_write(hVMD642I2C, addrI2C, 0x03, 0x1);

	/*配置SAA7121H*/
	/*GPIO0输出为低，选择IIC1总线，配置图像输出*/
	GPIO_RSET(GPVAL, 0x1);

	/*初始化Video Port0*/
	/*将Video Port0设为encoder输出*/
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
	/*初始化Video Port1*/
	/*将Video Port1设为采集输入*/
	portNumber = 1;
	vpHchannel1 = bt656_8bit_ncfc(portNumber);

/*----------------------------------------------------------*/
	/*启动采集模块*/
	bt656_capture_start(vpHchannel1);

	/*第一次运行，采集三帧数据*/
	for (nextFrame = 1; nextFrame <= 3; nextFrame++)
	{
		/*等待一帧数据采集完成*/
		while(capNewFrame == 0){}
		/*清除采集完成的标志，开始下一帧采集*/
		capNewFrame = 0;

		switch (nextFrame)
		{
			case 1:
				YBuf = Ybuffer1; CbBuf = Cbbuffer1; CrBuf = Crbuffer1;
				break;
			case 2:
				YBuf = Ybuffer2; CbBuf = Cbbuffer2; CrBuf = Crbuffer2;
				break;
			case 3:
				YBuf = Ybuffer3; CbBuf = Cbbuffer3; CrBuf = Crbuffer3;
				break;
		}
        send_frame_gray(numLines, numPixels, capYbuffer, YBuf);
	}
    nextFrame = 1;  /*给出下一帧的位置*/

    /*生成两个帧差图像*/
    for (nextDiff = 1; nextDiff <= 2; nextDiff++)
    {
        switch (nextDiff)
        {
            case 1:
                YBuf = Ybuffer2;   YSubBuf = Ybuffer1;   YAnsBuf = YbufferDiff12;
                CbBuf = Cbbuffer2; CbSubBuf = Cbbuffer1; CbAnsBuf = CbbufferDiff12;
                CrBuf = Crbuffer2; CrSubBuf = Crbuffer1; CrAnsBuf = CrbufferDiff12;
                break;
            case 2:
                YBuf = Ybuffer3;   YSubBuf = Ybuffer2;   YAnsBuf = YbufferDiff23;
                CbBuf = Cbbuffer3; CbSubBuf = Cbbuffer2; CbAnsBuf = CbbufferDiff23;
                CrBuf = Crbuffer3; CrSubBuf = Crbuffer2; CrAnsBuf = CrbufferDiff23;
                break;
        }
        gen_diff_frame_gray(numLines, numPixels, YBuf, YSubBuf, YAnsBuf);
    }

    /*拼合帧差图像，并传送至显示区*/
    YBuf = YbufferDiff12;   YAddBuf = YbufferDiff23;
    CbBuf = CbbufferDiff12; CbAddBuf = CbbufferDiff23;
    CrBuf = CrbufferDiff12; CrAddBuf = CrbufferDiff23;
    merge_diff_frame_gray(numLines, numPixels, YBuf, CbBuf, CrBuf, YAddBuf, CbAddBuf, CrAddBuf,
        YbufferPost, disCbbuffer, disCrbuffer);
    centroid(numLines, numPixels, YbufferPost, &positionX, &positionY);
    draw_rectangle(numLines, numPixels, YbufferPost, positionX, positionY);
    send_frame_gray(numLines, numPixels, YbufferPost, disYbuffer);

	/*启动显示模块*/
	bt656_display_start(vpHchannel0);
	/*建立实时计算循环*/
	for(;;)
	{
		/*当采集区的数据已经采集好，而显示缓冲区的数据已空*/
		if((capNewFrame == 1)&&(disNewFrame == 1))
		{
			/*清除采集完成的标志，提示可以显示缓冲区图像*/
			capNewFrame =0;  disNewFrame =0;

            /*传送一帧图像*/
            switch (nextFrame)
            {
                case 1:
                    YBuf = Ybuffer1; CbBuf = Cbbuffer1; CrBuf = Crbuffer1;
                    break;
                case 2:
                    YBuf = Ybuffer2; CbBuf = Cbbuffer2; CrBuf = Crbuffer2;
                    break;
                case 3:
                    YBuf = Ybuffer3; CbBuf = Cbbuffer3; CrBuf = Crbuffer3;
                    break;
            }
            send_frame_gray(numLines, numPixels, capYbuffer, YBuf);

            /*空间指针更新至下个位置*/
            if (nextFrame >= 3)
                nextFrame = 1;
            else
                nextFrame ++;

            /*计算帧差图像*/
            for (nextDiff = 1; nextDiff <= 2; nextDiff++)
            {
                switch (nextDiff)
                {
                    case 1:
                        YAnsBuf = YbufferDiff12; CbAnsBuf = CbbufferDiff12; CrAnsBuf = CrbufferDiff12;
                        switch (nextFrame)
                        {
                            case 1:
                                YBuf = Ybuffer2;   YSubBuf = Ybuffer1;
                                CbBuf = Cbbuffer2; CbSubBuf = Cbbuffer1;
                                CrBuf = Crbuffer2; CrSubBuf = Crbuffer1;
                                break;
                            case 2:
                                YBuf = Ybuffer3;   YSubBuf = Ybuffer2;
                                CbBuf = Cbbuffer3; CbSubBuf = Cbbuffer2;
                                CrBuf = Crbuffer3; CrSubBuf = Crbuffer2;
                                break;
                            case 3:
                                YBuf = Ybuffer1;   YSubBuf = Ybuffer3;
                                CbBuf = Cbbuffer1; CbSubBuf = Cbbuffer3;
                                CrBuf = Crbuffer1; CrSubBuf = Crbuffer3;
                                break;
                        }
                        break;
                    case 2:
                        YAnsBuf = YbufferDiff23; CbAnsBuf = CbbufferDiff23; CrAnsBuf = CrbufferDiff23;
                        switch (nextFrame)
                        {
                            case 1:
                                YBuf = Ybuffer3;   YSubBuf = Ybuffer2;
                                CbBuf = Cbbuffer3; CbSubBuf = Cbbuffer2;
                                CrBuf = Crbuffer3; CrSubBuf = Crbuffer2;
                                break;
                            case 2:
                                YBuf = Ybuffer1;   YSubBuf = Ybuffer3;
                                CbBuf = Cbbuffer1; CbSubBuf = Cbbuffer3;
                                CrBuf = Crbuffer1; CrSubBuf = Crbuffer3;
                                break;
                            case 3:
                                YBuf = Ybuffer2;   YSubBuf = Ybuffer1;
                                CbBuf = Cbbuffer2; CbSubBuf = Cbbuffer1;
                                CrBuf = Crbuffer2; CrSubBuf = Crbuffer1;
                        }
                        break;
                }
                gen_diff_frame_gray(numLines, numPixels, YBuf, YSubBuf, YAnsBuf);
            }

            /*拼合帧差图像，并传送至显示区*/
            YBuf = YbufferDiff12;   YAddBuf = YbufferDiff23;
            CbBuf = CbbufferDiff12; CbAddBuf = CbbufferDiff23;
            CrBuf = CrbufferDiff12; CrAddBuf = CrbufferDiff23;
            merge_diff_frame_gray(numLines, numPixels, YBuf, CbBuf, CrBuf, YAddBuf, CbAddBuf, CrAddBuf,
                YbufferPost, disCbbuffer, disCrbuffer);
            centroid(numLines, numPixels, YbufferPost, &positionX, &positionY);
            draw_rectangle(numLines, numPixels, YbufferPost, positionX, positionY);
            send_frame_gray(numLines, numPixels, YbufferPost, disYbuffer);
		}
	}
}
