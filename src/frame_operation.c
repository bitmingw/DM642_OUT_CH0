/*****************************************************************************
 * Author: bitmingw
 * Replace some redundant operations in main routine.
 *****************************************************************************/

#include <csl.h>
#include <csl_emifa.h>
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

        /*
        DAT_copy((void *)(srcCb + i * (numPixels >> 1)),
                 (void *)(dstCb + i * (numPixels >> 1)),
                 numPixels>>1);

        DAT_copy((void *)(srcCr + i * (numPixels >> 1)),
                 (void *)(dstCr + i * (numPixels >> 1)),
                 numPixels>>1);
        */
    }
}

void gen_diff_frame(int numLines, int numPixels, int Y, int Cb, int Cr, \
    int subY, int subCb, int subCr, int dstY, int dstCb, int dstCr)
{
    int i, j;
    Uint8 *sub, *sub2, *diffbyte;
    Uint8 byte1, byte2;
    
    /* For Y channel, set threshold to 0.5
     * i.e. if the MSB is different, then generate 0xFF
     * Otherwise generate 0x00
     */
    for (i = 0; i < numLines; i++)
    {
        for (j = 0; j < numPixels; j++)
        {
            sub  = (Uint8 *)(Y + i * numLines + j);
            sub2 = (Uint8 *)(subY + i * numLines + j);
            diffbyte = (Uint8 *)(dstY + i * numLines + j);
            byte1 = (*sub) & 0x80;
            byte2 = (*sub2) & 0x80;
            if (byte1 ^ byte2)
                *diffbyte = 0xFF;
            else
                *diffbyte = 0x00;
        }
    }
    /*
    for (i = 0; i < numLines; i++)
    {
        for (j = 0; j < (numPixels >> 1); j++)
        {
            sub  = (Uint8 *)(Cb + i * numLines + j);
            sub2 = (Uint8 *)(subCb + i * numLines + j);
            diffbyte = (Uint8 *)(dstCb + i * numLines + j);
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
            sub  = (Uint8 *)(Cr + i * numLines + j);
            sub2 = (Uint8 *)(subCr + i * numLines + j);
            diffbyte = (Uint8 *)(dstCr + i * numLines + j);
            if (*sub - *sub2 > *sub)
                *diffbyte = *sub2 - *sub;
            else
                *diffbyte = *sub - *sub2;
        }
    }
    */
}

void merge_diff_frame(int numLines, int numPixels, int diff1Y, int diff1Cb, int diff1Cr, \
    int diff2Y, int diff2Cb, int diff2Cr, int dispY, int dispCb, int dispCr)
{
    int i, j;
    Uint8 *sub, *sub2, *disp;
    
    /* For Y output, if both frame is non-negative (both 0xFF) then display 0xFF
     * Otherwise display 0x00
     */
    for (i = 0; i < numLines; i++)
    {
        for (j = 0; j < numPixels; j++)
        {
            sub  = (Uint8 *)(diff1Y + i * numLines + j);
            sub2 = (Uint8 *)(diff2Y + i * numLines + j);
            disp = (Uint8 *)(dispY + i * numLines + j);
            if (*sub & *sub2)
                *disp = 0xFF;
            else
                *disp = 0x00;
        }
    }
    for (i = 0; i < numLines; i++)
    {
        for (j = 0; j < (numPixels >> 1); j++)
        {
            /*
            sub  = (Uint8 *)(diff1Cb + i * numLines + j);
            sub2 = (Uint8 *)(diff2Cb + i * numLines + j);
            disp = (Uint8 *)(dispCb + i * numLines + j);
            if (*sub + *sub2 < *sub)
                *disp = 0xFF;
            else
                *disp = *sub + *sub2;
            */
            disp = (Uint8 *)(dispCb + i * numLines + j);
            *disp = 0x80;
        }
    }
    for (i = 0; i < numLines; i++)
    {
        for (j = 0; j < (numPixels >> 1); j++)
        {
            /*
            sub  = (Uint8 *)(diff1Cr + i * numLines + j);
            sub2 = (Uint8 *)(diff2Cr + i * numLines + j);
            disp = (Uint8 *)(dispCr + i * numLines + j);
            if (*sub + *sub2 < *sub)
                *disp = 0xFF;
            else
                *disp = *sub + *sub2;
            */
            disp = (Uint8 *)(dispCr + i * numLines + j);
            *disp = 0x80;
        }
    }
}
