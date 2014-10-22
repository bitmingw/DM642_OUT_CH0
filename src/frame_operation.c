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

void gen_diff_frame_gray(int numLines, int numPixels, int Y, int subY, int dstY)
{
    int i, j;
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
            CACHE_S[j] = ((CACHE_A[j] ^ CACHE_B[j]) & 0x80) ? 0xFF : 0x00;
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
            CACHE_S[j] = (CACHE_A[j] & CACHE_B[j]) ? 0xFF : 0x00;
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

void centroid(int numLines, int numPixels, int srcY, int * positionX, int * positionY)
{
    int i, j, count, points, sum;
    extern Uint8 CACHE_S[720];    /*Iterate for each line*/
    extern Uint8 CACHE_A[720];    /*Store the count of each line. 588 used.*/
    extern Uint8 CACHE_B[720];    /*Store the mid white point of x-axis. 588 used.*/

    for (i = 0; i < numLines; i++)
    {
        DAT_copy((void *)(srcY + i * numPixels),
                 CACHE_S, numPixels);
        /*Count the number of white points in this line.*/
        count = 0;
        for (j = 0; j < numPixels; j++)
        {
            if (CACHE_S[j] == 0xFF)
                count++;
        }
        CACHE_A[i] = count;
        /*Find the mid of white points*/
        points = count / 2;
        count = 0;
        for (j = 0; j < numPixels; j++)
        {
            if (CACHE_S[j] == 0xFF)
                count++;
            if (count == points)
            {
                CACHE_B[i] = j;
                break;
            }
        }
    }

    /* Calculate the position */
    points = 0;    sum = 0;
    for (i = 0; i < numLines; i++)
    {
        points += CACHE_A[i];
    }
    *positionY = points / numLines;
    for (i = 0; i < numLines; i++)
    {
        sum += CACHE_B[i] * CACHE_A[i];
    }
    *positionX = sum / points;
}

/* DEFINE line_width = 20
 * DEFINE window_length = window_height = 100
 */
void draw_rectangle(int numLines, int numPixels, int dstY, int positionX, int positionY)
{
    int i, j;
    int width = 20;
    int UL_line, UL_pixel;    /* Upper left value, should be >= 20 */
    int BR_line, BR_pixel;    /* Button right value, should be <=567 && <=699 */
    extern Uint8 CACHE_S[720];
    
    UL_pixel = (positionX - 50 < 20) ? 20 : positionX - 50;
    UL_line  = (positionY - 50 < 20) ? 20 : positionY - 50;
    BR_pixel = (positionX + 50 > 567) ? 567 : positionY + 50;
    BR_line  = (positionY + 50 > 699) ? 699 : positionX + 50;
    
    /*Up horizontal*/
    /* odd lines */
    for(i = UL_line - width; i < UL_line; i++)
	{
        DAT_copy((void *)(dstY + i * numPixels), CACHE_S, numPixels);
        for(j = UL_pixel - width; j < BR_pixel + width; j++)
	    {
	    	 CACHE_S[j] = 0xFF;
	    }
        DAT_copy(CACHE_S, (void *)(dstY + i * numPixels), numPixels);
	}	
	/* even lines */
	for(i = numLines/2 + UL_line - width; i < numLines/2 + UL_line; i++)
	{
        DAT_copy((void *)(dstY + i * numPixels), CACHE_S, numPixels);
	    for(j = UL_pixel - width; j < BR_pixel + width; j++)
	    {
	    	 CACHE_S[j] = 0xFF;
	    }
        DAT_copy(CACHE_S, (void *)(dstY + i * numPixels), numPixels);
	}

	/*Button horizontal*/
	/* odd lines */
    for(i = BR_line; i < BR_line + width; i++)
	{
        DAT_copy((void *)(dstY + i * numPixels), CACHE_S, numPixels);	    
        for(j = UL_pixel - width; j < BR_pixel + width; j++)
	    {
	    	 CACHE_S[j] = 0xFF;
	    }
        DAT_copy(CACHE_S, (void *)(dstY + i * numPixels), numPixels);
	}
	/* even lines */
	for(i = numLines/2 + BR_line; i < numLines/2 + BR_line + width; i++)
	{
	    DAT_copy((void *)(dstY + i * numPixels), CACHE_S, numPixels);
        for(j = UL_pixel - width; j < BR_pixel + width; j++)
	    {
	    	 CACHE_S[j] = 0xFF;
	    }
        DAT_copy(CACHE_S, (void *)(dstY + i * numPixels), numPixels);
	}	
	
	/*Left vertical*/
	/* odd lines */
    for(i = UL_line; i < BR_line; i++)
	{
	    DAT_copy((void *)(dstY + i * numPixels), CACHE_S, numPixels);
        for(j = UL_pixel - width; j < UL_pixel; j++)
	    {
	    	 CACHE_S[j] = 0xFF;
	    }
        DAT_copy(CACHE_S, (void *)(dstY + i * numPixels), numPixels);
	}
	/* even lines */
	for(i = numLines/2 + UL_line; i < numLines/2 + BR_line; i++)
	{
	    DAT_copy((void *)(dstY + i * numPixels), CACHE_S, numPixels);
        for(j = UL_pixel - width; j < UL_pixel; j++)
	    {
	    	 CACHE_S[j] = 0xFF;
	    }
        DAT_copy(CACHE_S, (void *)(dstY + i * numPixels), numPixels);
	}
	
	/*Right vertical*/
	/* odd lines */
    for(i = UL_line; i < BR_line; i++)
	{
	    DAT_copy((void *)(dstY + i * numPixels), CACHE_S, numPixels);
        for(j = BR_pixel; j<BR_pixel + width; j++)
	    {
	    	 CACHE_S[j] = 0xFF;
	    }
        DAT_copy(CACHE_S, (void *)(dstY + i * numPixels), numPixels);
	}
	/* even lines */
    for(i = numLines/2 + UL_line; i < numLines/2 + BR_line; i++)
	{
	    DAT_copy((void *)(dstY + i * numPixels), CACHE_S, numPixels);
        for(j = BR_pixel; j < BR_pixel + width; j++)
	    {
	    	 CACHE_S[j] = 0xFF;
	    }
        DAT_copy(CACHE_S, (void *)(dstY + i * numPixels), numPixels);
	}    
}

void histograms(int numLines, int numPixels, int srcY)
{
    int i, j;
    Uint32 fillVal = 0x00000000;  /*32bit value to fill 0 to space*/
    extern Uint32 HIST_X[720];    /*Store the histogram on X axis*/
    extern Uint32 HIST_Y[588];    /*Store the histogram on Y axis*/
    extern Uint8 CACHE_S[720];    /*Iterate for each line*/
    
    /* Clear the former histogram info */
    DAT_fill(HIST_X, numPixels, &fillVal);
    DAT_fill(HIST_Y, numLines, &fillVal);
    
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
    for (i = 0; i < 588; i++)
    {
        if (HIST_Y[i] > peakValY)
        {
            peakValY = HIST_Y[i];
            peakPositionY = i;
        }
    }
    
    /* Check if thresholds are met - there is a moving object */
    
    /* No object detected */
    if (peakValX < thresholdX || peakValY < thresholdY)
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
        
        /* Search in X axis near the peak point */
        for (i = peakPositionX; i >= 0; i--)
        {
            if (HIST_X[i] < edgeValX)
            {
                x1 = i;
                break;
            }
            x1 = 0;             /* else */
        }
        for (i = peakPositionX; i < numPixels; i++)
        {
            if (HIST_X[i] < edgeValX)
            {
                x2 = i;
                break;
            }
            x2 = numPixels;     /* else */
        }
        /* Search in Y axis near the peak point */
        for (i = peakPositionY; i >= 0; i--)
        {
            if (HIST_Y[i] < edgeValY)
            {
                y1 = i;
                break;
            }
            y1 = 0;             /* else */
        }
        for (i = peakPositionY; i < numLines; i++)
        {
            if (HIST_Y[i] < edgeValY)
            {
                y2 = i;
                break;
            }
            y2 = numLines;     /* else */
        }
        
        /* Recalculate the peak position via this range */
        peakPositionX = (x1 + x2) >> 1;
        peakPositionY = (y1 + y2) >> 1;
        
        /* Return the values via reference */
        *positionX = peakPositionX;
        *positionY = peakPositionY;
        *rangeX = (x2 - x1) >> 1;
        *rangeY = (y2 - y1) >> 1;
        return;
    }
}
