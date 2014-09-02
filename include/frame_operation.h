/*****************************************************************************
 * Author: bitmingw
 * Replace some redundant operations in main routine.
 *****************************************************************************/

#ifndef _H_FRAME_OPERATION
#define _H_FRAME_OPERATION

/* Copy one frame from src to dst.
 * src and dst are memory addr.
 * EMDA are used to complete the process.
 */
void send_frame(int numLines, int numPixels, int srcY, int srcCb, int srcCr, \
    int dstY, int dstCb, int dstCr);

/* Gray version of send_frame()
 */
void send_frame_gray(int numLines, int numPixels, int srcY, int dstY);


/* Generate a difference frame from Y/Cb/Cr and subY/subCb/subCr.
 * The results are placed in dstY/dstCb/dstCr.
 */
void gen_diff_frame(int numLines, int numPixels, int Y, int Cb, int Cr, \
    int subY, int subCb, int subCr, int dstY, int dstCb, int dstCr);

/* Gray version of gen_diff_frame()
 */
void gen_diff_frame_gray(int numLines, int numPixels, int Y, int subY, int dstY);


/* Generate the display image from two difference frames.
 */
void merge_diff_frame(int numLines, int numPixels, int diff1Y, int diff1Cb, int diff1Cr, \
    int diff2Y, int diff2Cb, int diff2Cr, int dispY, int dispCb, int dispCr);

/* Gray version of merge_diff_frame()
 */
void merge_diff_frame_gray(int numLines, int numPixels, int diff1Y, int diff1Cb, int diff1Cr, \
    int diff2Y, int diff2Cb, int diff2Cr, int dispY, int dispCb, int dispCr);

#endif /*_H_FRAME_OPERATION*/
