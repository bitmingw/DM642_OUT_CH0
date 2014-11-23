/*****************************************************************************
 * Author: bitmingw
 * Replace some redundant operations in main routine.
 *****************************************************************************/

#include <csl.h>
#include <csl_emifa.h>
#include <csl_dat.h>
#include "frame_operation.h"

 void send_frame(int numLines, int numPixels, int srcY, int srcCb, int srcCr, \
    int dstY, int dstCb, int dstCr)
{
    int i;

    for(i=0; i<numLines; i++)
    {
        DAT_copy((void *)(srcY + i * numPixels),
                 (void *)(dstY + i * numPixels),
                 numPixels);

        DAT_copy((void *)(srcCb + i * (numPixels >> 1)),
                 (void *)(dstCb + i * (numPixels >> 1)),
                 numPixels>>1);

        DAT_copy((void *)(srcCr + i * (numPixels >> 1)),
                 (void *)(dstCr + i * (numPixels >> 1)),
                 numPixels>>1);
    }
}

void gen_diff_frame(int numLines, int numPixels, int Y, int Cb, int Cr, \
    int subY, int subCb, int subCr, int dstY, int dstCb, int dstCr)
{
    int i, j;
    Uint8 *sub, *sub2, *diffbyte;

    for (i = 0; i < numLines; i++)
    {
        for (j = 0; j < numPixels; j++)
        {
            sub  = (Uint8 *)(Y + i * numPixels + j);
            sub2 = (Uint8 *)(subY + i * numPixels + j);
            diffbyte = (Uint8 *)(dstY + i * numPixels + j);
            if (*sub - *sub2 > *sub)
                *diffbyte = *sub2 - *sub;
            else
                *diffbyte = *sub - *sub2;
        }
    }
    for (i = 0; i < numLines; i++)
    {
        for (j = 0; j < (numPixels >> 1); j++)
        {
            sub  = (Uint8 *)(Cb + i * numPixels + j);
            sub2 = (Uint8 *)(subCb + i * numPixels + j);
            diffbyte = (Uint8 *)(dstCb + i * numPixels + j);
            if (*sub - *sub2 > *sub)
                *diffbyte = *sub2 - *sub;
            else
                *diffbyte = *sub - *sub2;
        }
    }
    for (i = 0; i < numLines; i++)
    {
        for (j = 0; j < (numPixels >> 1); j++)
        {
            sub  = (Uint8 *)(Cr + i * numPixels + j);
            sub2 = (Uint8 *)(subCr + i * numPixels + j);
            diffbyte = (Uint8 *)(dstCr + i * numPixels + j);
            if (*sub - *sub2 > *sub)
                *diffbyte = *sub2 - *sub;
            else
                *diffbyte = *sub - *sub2;
        }
    }
}

void merge_diff_frame(int numLines, int numPixels, int diff1Y, int diff1Cb, int diff1Cr, \
    int diff2Y, int diff2Cb, int diff2Cr, int dispY, int dispCb, int dispCr)
{
    int i, j;
    Uint8 *sub, *sub2, *disp;

    for (i = 0; i < numLines; i++)
    {
        for (j = 0; j < numPixels; j++)
        {
            sub  = (Uint8 *)(diff1Y + i * numPixels + j);
            sub2 = (Uint8 *)(diff2Y + i * numPixels + j);
            disp = (Uint8 *)(dispY + i * numPixels + j);
            if (*sub + *sub2 < *sub)
                *disp = 0xFF;
            else
                *disp = *sub + *sub2;
        }
    }
    for (i = 0; i < numLines; i++)
    {
        for (j = 0; j < (numPixels >> 1); j++)
        {
            sub  = (Uint8 *)(diff1Cb + i * numPixels + j);
            sub2 = (Uint8 *)(diff2Cb + i * numPixels + j);
            disp = (Uint8 *)(dispCb + i * numPixels + j);
            if (*sub + *sub2 < *sub)
                *disp = 0xFF;
            else
                *disp = *sub + *sub2;
        }
    }
    for (i = 0; i < numLines; i++)
    {
        for (j = 0; j < (numPixels >> 1); j++)
        {
            sub  = (Uint8 *)(diff1Cr + i * numPixels + j);
            sub2 = (Uint8 *)(diff2Cr + i * numPixels + j);
            disp = (Uint8 *)(dispCr + i * numPixels + j);
            if (*sub + *sub2 < *sub)
                *disp = 0xFF;
            else
                *disp = *sub + *sub2;
        }
    }
}


 void send_frame_gray(int numLines, int numPixels, int srcY, int dstY)
{
    int i;

    for(i=0; i<numLines; i++)
    {
        DAT_copy((void *)(srcY + i * numPixels),
                 (void *)(dstY + i * numPixels),
                 numPixels);
    }
}

 void send_fill_frame_gray(int numLines, int numPixels, int srcY, int dstY, int dstCb, int dstCr)
{
    int i;
    Uint32 fillVal = 0x80808080;    /*32bit value to cover Cb and Cr channel*/

    for(i=0; i<numLines; i++)
    {
        DAT_copy((void *)(srcY + i * numPixels),
                 (void *)(dstY + i * numPixels),
                 numPixels);
    }
    for (i = 0; i < numLines; i++)
    {
        DAT_fill((void *)(dstCb + i * numPixels), numPixels, &fillVal);

    }
    for (i = 0; i < numLines; i++)
    {
        DAT_fill((void *)(dstCr + i * numPixels), numPixels, &fillVal);
    }
}

void gen_diff_frame_gray(int numLines, int numPixels, int Y, int subY, int dstY)
{
    int i, j;
    Uint8 diff_val;
    extern Uint8 CACHE_A[720];
    extern Uint8 CACHE_B[720];
    extern Uint8 CACHE_S[720];

    /* For Y channel, set threshold to 0.5
     * i.e. if the MSB is different, then generate 0xFF
     * Otherwise generate 0x00
     */
    for (i = 0; i < numLines; i++)
    {
        DAT_copy((void *)(Y + i * numPixels),
                 CACHE_A, numPixels);
        DAT_copy((void *)(subY + i * numPixels),
                 CACHE_B, numPixels);
        for (j = 0; j < numPixels; j++)
        {
            diff_val = CACHE_A[j] > CACHE_B[j] ? (CACHE_A[j] - CACHE_B[j]) : (CACHE_B[j] - CACHE_A[j]);
            CACHE_S[j] = diff_val > 0x40 ? 0xFF : 0x00;     /* 0x40 is a proper value to use */
        }
        DAT_copy(CACHE_S,
                 (void *)(dstY + i * numPixels), numPixels);
    }
}

void merge_diff_frame_gray(int numLines, int numPixels, int diff1Y, int diff1Cb, int diff1Cr, \
    int diff2Y, int diff2Cb, int diff2Cr, int dispY, int dispCb, int dispCr)
{
    int i, j;
    Uint32 fillVal = 0x80808080;    /*32bit value to cover Cb and Cr channel*/
    extern Uint8 CACHE_A[720];
    extern Uint8 CACHE_B[720];
    extern Uint8 CACHE_S[720];

    /* For Y output, if both frame is non-negative (both 0xFF) then display 0xFF
     * Otherwise display 0x00
     */
    for (i = 0; i < numLines; i++)
    {
        DAT_copy((void *)(diff1Y + i * numPixels),
                 CACHE_A, numPixels);
        DAT_copy((void *)(diff2Y + i * numPixels),
                 CACHE_B, numPixels);
        for (j = 0; j < numPixels; j++)
        {
            CACHE_S[j] = (CACHE_A[j] | CACHE_B[j]) ? 0xFF : 0x00;
        }
        DAT_copy(CACHE_S,
                 (void *)(dispY + i * numPixels), numPixels);
    }
    for (i = 0; i < numLines; i++)
    {
        DAT_fill((void *)(dispCb + i * numPixels), numPixels, &fillVal);

    }
    for (i = 0; i < numLines; i++)
    {
        DAT_fill((void *)(dispCr + i * numPixels), numPixels, &fillVal);
    }
}


void draw_rectangle(int numLines, int numPixels, int dstY, int positionX, int positionY, int rangeX, int rangeY)
{
    int i, j;
    int width = 2;
    int x1, x2, y1, y2;
    extern Uint8 CACHE_S[720];

    x1 = (positionX - rangeX >= 2) ? (positionX - rangeX) : 2;
    x2 = (positionX + rangeX < numPixels - 2) ? (positionX + rangeX) : numPixels - 2;
    /* Y storage for odd and even are separated */
    y1 = (positionY - rangeY >= 2) ? (positionY - rangeY)/2 + 1 : 2;
    y2 = (positionY + rangeY < numLines - 2) ? (positionY + rangeY)/2 - 1 : (numLines)/2 - 2;

    /* Up horizontal */
    /* odd lines */
    for(i = y1 - width; i < y1; i++)
	{
        DAT_copy((void *)(dstY + i * numPixels), CACHE_S, numPixels);
        for(j = x1 - width; j < x2 + width; j++)
	    {
	    	 CACHE_S[j] = 0xFF;
	    }
        DAT_copy(CACHE_S, (void *)(dstY + i * numPixels), numPixels);
	}
	/* even lines */
	for(i = numLines/2 + y1 - width; i < numLines/2 + y1; i++)
	{
        DAT_copy((void *)(dstY + i * numPixels), CACHE_S, numPixels);
	    for(j = x1 - width; j < x2 + width; j++)
	    {
	    	 CACHE_S[j] = 0xFF;
	    }
        DAT_copy(CACHE_S, (void *)(dstY + i * numPixels), numPixels);
	}

	/* Button horizontal */
	/* odd lines */
    for(i = y2; i < y2 + width; i++)
	{
        DAT_copy((void *)(dstY + i * numPixels), CACHE_S, numPixels);
        for(j = x1 - width; j < x2 + width; j++)
	    {
	    	 CACHE_S[j] = 0xFF;
	    }
        DAT_copy(CACHE_S, (void *)(dstY + i * numPixels), numPixels);
	}
	/* even lines */
	for(i = numLines/2 + y2; i < numLines/2 + y2 + width; i++)
	{
	    DAT_copy((void *)(dstY + i * numPixels), CACHE_S, numPixels);
        for(j = x1 - width; j < x2 + width; j++)
	    {
	    	 CACHE_S[j] = 0xFF;
	    }
        DAT_copy(CACHE_S, (void *)(dstY + i * numPixels), numPixels);
	}

	/* Left vertical */
	/* odd lines */
    for(i = y1; i < y2; i++)
	{
        for(j = x1 - width; j < x1; j++)
	    {
	    	 *(Uint8 *)(dstY + i*numPixels + j) = 0xFF;
	    }
	}
	/* even lines */
	for(i = numLines/2 + y1; i < numLines/2 + y2; i++)
	{
        for(j = x1 - width; j < x1; j++)
	    {
	    	 *(Uint8 *)(dstY + i*numPixels + j) = 0xFF;
	    }
	}

	/* Right vertical */
	/* odd lines */
    for(i = y1; i < y2; i++)
	{
        for(j = x2; j < x2 + width; j++)
	    {
	    	 *(Uint8 *)(dstY + i*numPixels + j) = 0xFF;
	    }
	}
	/* even lines */
    for(i = numLines/2 + y1; i < numLines/2 + y2; i++)
	{
        for(j = x2; j < x2 + width; j++)
	    {
	    	 *(Uint8 *)(dstY + i*numPixels + j) = 0xFF;
	    }
	}
}

void draw_arrow(int numLines, int numPixels, int dstY, int direction)
{
    int i, j;
    
    if (direction != LEFT_ARROW && direction != RIGHT_ARROW) {
        return;
    }
    /* NOTE: left part is truncated, it starts from 30 pix */
    if (direction == LEFT_ARROW) {
        /* odd lines, upper part */
        for (i = 130; i <= 140; i++) {
            for (j = 320 - 2*i; j <= 60; j++) {
                *(Uint8 *)(dstY + i*numPixels + j) = 0xFF;
            }
        }
        /* even lines, upper part */
        for (i = 130 + numLines/2; i <= 140 + numLines/2; i++) {
            for (j = 896 - 2*i; j <= 60; j++) {
                *(Uint8 *)(dstY + i*numPixels + j) = 0xFF;
            }
        }
        /* odd lines, lower part */
        for (i = 141; i <= 150; i++) {
            for (j = 2*i - 240; j <= 60; j++) {
                *(Uint8 *)(dstY + i*numPixels + j) = 0xFF;
            }
        }
        /* even lines, lower part */
        for (i = 141 + numLines/2; i <= 150 + numLines/2; i++) {
            for (j = 2*i - 816; j <= 60; j++) {
                *(Uint8 *)(dstY + i*numPixels + j) = 0xFF;
            }
        }
    }
    /* Right part is normal, ends at 720 pix */
    else if (direction == RIGHT_ARROW) {
        /* odd lines, upper part */
        for (i = 130; i <= 140; i++) {
            for (j = 690; j <= 430 + 2*i; j++) {
                *(Uint8 *)(dstY + i*numPixels + j) = 0xFF;
            }
        }
        /* even lines, upper part */
        for (i = 130 + numLines/2; i <= 140 + numLines/2; i++) {
            for (j = 690; j <= 2*i - 146; j++) {
                *(Uint8 *)(dstY + i*numPixels + j) = 0xFF;
            }
        }
        /* odd lines, lower part */
        for (i = 141; i <= 150; i++) {
            for (j = 690; j <= 990 - 2*i; j++) {
                *(Uint8 *)(dstY + i*numPixels + j) = 0xFF;
            }
        }
        /* even lines, lower part */
        for (i = 141 + numLines/2; i <= 150 + numLines/2; i++) {
            for (j = 690; j <= 1466 - 2*i; j++) {
                *(Uint8 *)(dstY + i*numPixels + j) = 0xFF;
            }
        }
    }
}

void histograms(int numLines, int numPixels, int srcY)
{
    int i, j;
    extern Uint32 HIST_X[720];    /*Store the histogram on X axis*/
    extern Uint32 HIST_Y[588];    /*Store the histogram on Y axis*/
    extern Uint8 CACHE_S[720];    /*Iterate for each line*/

    /* Clear the former histogram info */
    for (i = 0; i < numPixels; i++)
    {
        HIST_X[i] = 0;
    }
    for (i = 0; i < numLines; i++)
    {
        HIST_Y[i] = 0;
    }

    /* Calculate two sides histogram at the same time */
    for (i = 0; i < numLines; i++)
    {
        DAT_copy((void *)(srcY + i * numPixels),
                 CACHE_S, numPixels);
        /* Contribute to X and Y axis when a white point is found. */
        for (j = 0; j < numPixels; j++)
        {
            if (CACHE_S[j] == 0xFF)
            {
                HIST_X[j]++;
                HIST_Y[i]++;
            }
        }
    }
}

void hist_analysis(int numLines, int numPixels, int * positionX, int * positionY, int * rangeX, int * rangeY)
{
    int i;
    Uint32 peakValX, peakValY;              /* The max values on histogram */
    int peakPositionX, peakPositionY;       /* The index of point with highest density of histogram */
    Uint32 edgeValX, edgeValY;                 /* The edge (half) value on histogram */
    Uint32 x1, x2, y1, y2;                     /* The range of object */
    extern Uint32 thresholdX, thresholdY;   /* The external threshold value of histogram to detect a object */
    extern Uint32 HIST_X[720];              /* The histogram on X axis */
    extern Uint32 HIST_Y[588];              /* The histogram on Y axis */

    /* Calculate on X axis */
    peakValX = 0;
    peakPositionX = 0;
    for (i = 0; i < 720; i++)
    {
        if (HIST_X[i] > peakValX)
        {
            peakValX = HIST_X[i];
            peakPositionX = i;
        }
    }

    /* Calculate on Y axis */
    peakValY = 0;
    peakPositionY = 0;
    for (i = 0; i < 294; i++)
    {
        if (HIST_Y[i] > peakValY)
        {
            peakValY = HIST_Y[i];
            peakPositionY = i;
        }
    }

    /* Check if thresholds are met - there is a moving object */

    /* No object detected */
    if (peakValX < thresholdX || peakValY < thresholdY/2)
    {
        *positionX = peakPositionX;
        *positionY = peakPositionY;
        *rangeX = 0;
        *rangeY = 0;
        return;
    }
    /* Find the range of the object */
    else
    {
        edgeValX = peakValX >> 1;
        edgeValY = peakValY >> 1;
        x1 = 0;
        x2 = numPixels - 1;
        y1 = 0;
        y2 = numLines/2 - 1;

        /* Search in X axis near the peak point */
        for (i = peakPositionX; i >= 0; i--)
        {
            if (HIST_X[i] < edgeValX)
            {
                x1 = i;
                break;
            }
        }
        for (i = peakPositionX; i < numPixels; i++)
        {
            if (HIST_X[i] < edgeValX)
            {
                x2 = i;
                break;
            }
        }
        /* Search in Y axis near the peak point */
        for (i = peakPositionY; i >= 0; i--)
        {
            if (HIST_Y[i] < edgeValY)
            {
                y1 = i;
                break;
            }
        }
        for (i = peakPositionY; i < numLines/2; i++)
        {
            if (HIST_Y[i] < edgeValY)
            {
                y2 = i;
                break;
            }
        }

        /* Recalculate the peak position via this range */
        peakPositionX = (x1 + x2) >> 1;
        peakPositionY = (y1 + y2) >> 1;

        /* Return the values via reference */
        *positionX = peakPositionX;
        *positionY = peakPositionY * 2;
        *rangeX = (x2 - x1) >> 1;
        *rangeY = (y2 - y1);
        return;
    }
}
