/*****************************************************************************
 * Author: bitmingw
 * Replace some redundant operations in main routine.
 *****************************************************************************/

#ifndef _H_FRAME_OPERATION
#define _H_FRAME_OPERATION

#define NO_ARROW    0
#define LEFT_ARROW -1
#define RIGHT_ARROW 1

/* Copy one frame from src to dst.
 * src and dst are memory addr.
 * EMDA are used to complete the process.
 */
void send_frame(int numLines, int numPixels, int srcY, int srcCb, int srcCr, \
    int dstY, int dstCb, int dstCr);

/* Generate a difference frame from Y/Cb/Cr and subY/subCb/subCr.
 * The results are placed in dstY/dstCb/dstCr.
 */
void gen_diff_frame(int numLines, int numPixels, int Y, int Cb, int Cr, \
    int subY, int subCb, int subCr, int dstY, int dstCb, int dstCr);

/* Generate the display image from two difference frames.
 */
void merge_diff_frame(int numLines, int numPixels, int diff1Y, int diff1Cb, int diff1Cr, \
    int diff2Y, int diff2Cb, int diff2Cr, int dispY, int dispCb, int dispCr);


/* Gray version of send_frame()
 */
void send_frame_gray(int numLines, int numPixels, int srcY, int dstY);

/* Gray version of gen_diff_frame()
 */
void gen_diff_frame_gray(int numLines, int numPixels, int Y, int subY, int dstY);

/* Gray version of merge_diff_frame()
 */
void merge_diff_frame_gray(int numLines, int numPixels, int diff1Y, int diff1Cb, int diff1Cr, \
    int diff2Y, int diff2Cb, int diff2Cr, int dispY, int dispCb, int dispCr);


/* Draw a white rectangle in the current frame.
 * The location of rectangle is determined by the location of moving object, namely positionX and positionY.
 * Make sure positionX and positionY are in the range of screen.
 * The size of rectangle is determined by the size of moving object, namely rangeX and rangeY.
 * Make sure rangeX and rangeY are not zero.
 * The width of rectangle is a constant value 1.
 */
void draw_rectangle(int numLines, int numPixels, int dstY, int positionX, int positionY, int rangeX, int rangeY);

/* Draw a white arrow pointing to left or to right on the screen.
 * The position of arrow is determined by the movement of holder.
 * i.e. if the holder turn left, the left arrow will be displayed.
 */
void draw_arrow(int numLines, int numPixels, int dstY, int direction);

/* Calculate histogram of two axes of image.
 * Update two preallocated arrays to store the values.
 */
void histograms(int numLines, int numPixels, int srcY);

 /* Local the moving object in the screen based on two sided external histogram of the image.
  * Update three numbers: the position and range of the object
  * External threshold is checked to determine if there is no object in the screen.
  * The boundary of object is defined as the half of the peak value.
  */
void hist_analysis(int numLines, int numPixels, int * positionX, int * positionY, int * rangeX, int * rangeY);

#endif /*_H_FRAME_OPERATION*/
