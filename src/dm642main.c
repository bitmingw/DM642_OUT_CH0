/********************************************************************/
/* Video tracking system via Three-frame difference method          */
/* Author: Ming Wen                                                 */
/********************************************************************/

/********************************************************************/
/*  Copyright 2006 by Vision Magic Ltd.								*/
/*  Restricted rights to use, duplicate or disclose this code are	*/
/*  granted through contract.									    */
/*  															    */
/********************************************************************/

#include <stdlib.h>
#include <csl.h>
#include <csl_cache.h>
#include <csl_emifa.h>
#include <csl_i2c.h>
#include <csl_gpio.h>
#include <csl_irq.h>
#include <csl_chip.h>
#include <csl_dat.h>
#include <csl_timer.h>

#include "iic.h"
#include "vportcap.h"
#include "vportdis.h"
#include "sa7121h.h"
#include "TVP51xx.h"
#include "vmd642.h"
#include "vmd642_uart.h"
#include "frame_operation.h"
#include "ctrl_operation.h"
#include "g_config.h"

interrupt void MovingCtrl(void);
void do_analysis();

/********************************************************************/

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
    /*TIMER初始化，设置TIMER0*/
    hTimer = TIMER_open(TIMER_DEV0, 0);
    TimerEventId = TIMER_getEventId(hTimer);
    TIMER_config(hTimer, &timerConfig);
    
/*----------------------------------------------------------*/
	/*中断向量表的初始化*/
	//Point to the IRQ vector table
    IRQ_setVecs(vectors);
    IRQ_nmiEnable();
    IRQ_globalEnable();
    IRQ_map(IRQ_EVT_VINT1, 11);
    IRQ_map(IRQ_EVT_VINT0, 12);
    IRQ_map(TimerEventId, 14);
    IRQ_reset(IRQ_EVT_VINT1);
    IRQ_reset(IRQ_EVT_VINT0);
    IRQ_reset(TimerEventId);
    IRQ_enable(TimerEventId);   /* Enable timer interrupt */
    
    /*打开一个数据拷贝的数据通路*/
    DAT_open(DAT_CHAANY, DAT_PRI_LOW, DAT_OPEN_2D);

/*----------------------------------------------------------*/
/*打开RS485输出信号通路*/
    /* Open UART */
    g_uartHandleA = VMD642_UART_open(VMD642_UARTB,
    									UARTHW_VMD642_BAUD_9600,
    									&g_uartConfig);
    
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
    
    /*开启定时器*/
    TIMER_start(hTimer);

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
        disYbuffer, disCbbuffer, disCrbuffer);
    
    /*采集随机数种子*/
    srand(TIMER_getCount(hTimer));
    /*初始化Kalman滤波器*/
    init_kalman_filter();
    
    /*
    histograms(numLines, numPixels, YbufferPost);
    send_frame_gray(numLines, numPixels, YbufferPost, disYbuffer);
    */

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
                disYbuffer, disCbbuffer, disCrbuffer);
            
            /*
            histograms(numLines, numPixels, YbufferPost);
            send_frame_gray(numLines, numPixels, YbufferPost, disYbuffer);
            */
		}
	}
}

interrupt void MovingCtrl(void)
{
    extern Matrix21 X_post;
    extern Matrix21 z;
    Uint8 i;
    
    /* Calculate pre-configure parameters of filter */
    do_analysis();
    
    /* Iterate the filter process */
    kalman_filter();
    
    /* if no object found, stop the holder if it is running */
    if (nextMove == HOLDER_MOV_STAY && curMove != HOLDER_MOV_STAY) {
        for (i = 0; i < 7; i++) {
            VMD642_UART_putChar(g_uartHandleA, stay[i]);
        }
    }
    /* if we are tracking an object, decide the direction to move */
    if (nextMove == HOLDER_MOV_UNDEF) {
        if (z.array[0][0] < numPixels/2) {
            nextMove = HOLDER_MOV_LEFT;
        }
        else {
            nextMove = HOLDER_MOV_RIGHT;
        }
    }

    /* Set moving command if next != current */
    if (nextMove == HOLDER_MOV_LEFT && curMove != HOLDER_MOV_LEFT) {
        for (i = 0; i < 7; i++) {
            VMD642_UART_putChar(g_uartHandleA, turnLeft[i]);
        }
    }
    else if (nextMove == HOLDER_MOV_RIGHT && curMove != HOLDER_MOV_RIGHT) {
        for (i = 0; i < 7; i++) {
            VMD642_UART_putChar(g_uartHandleA, turnRight[i]);
        }
    }
    
    /* Finally update moving command */
    curMove = nextMove;
}

void do_analysis(void)
{
    extern Uint32 disYbuffer;
    extern int numPixels, numLines;
    extern int positionX, positionY, rangeX, rangeY;
    
    extern Matrix21 X_pre, X_post, X_measure, B, v;
    extern Matrix22 P_pre, P_post;
    extern double u, sigma_u, sigma_z;
    
    histograms(numLines, numPixels, disYbuffer);
    
    hist_analysis(numLines, numPixels, &positionX, &positionY, &rangeX, &rangeY);
    
    /* No object is found, don't change measurement position, but update input value */
    /* Stop movement of the camera */
    if ((rangeX == 0 || rangeY == 0) && (curMove != HOLDER_MOV_UNDEF)) {
        u = curMove * angular_speed;
        nextMove = HOLDER_MOV_STAY;
    }
    /* One object is found, update variables */
    /* Move camera to track the object */
    else {
        X_measure.array[0][0] = positionX;
        X_measure.array[1][0] = positionY;
        u = curMove * angular_speed;  /* input value */
        sigma_u = rangeX / numPixels;   /* process error */
        sigma_z = 0.01;             /* measurement error, estimated as constant */
        nextMove = HOLDER_MOV_UNDEF;
    }
    /* Do iteration for state vector and covariance */
    X_pre = X_post;
    P_pre = P_post;
}
