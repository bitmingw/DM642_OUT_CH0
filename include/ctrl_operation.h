/*****************************************************************************
 * Author: bitmingw
 * Kalman filter, system modeling and moving control.
 *****************************************************************************/

 #ifndef _H_CTRL_OPERATION
 #define _H_CTRL_OPERATION
 
/* define some matrix type here */
typedef struct matrix41 {
    double array[4][1];
} Matrix41;

typedef struct matrix44 {
    double array[4][4];
} Matrix44;


 /* Kalman filter function.
  * Read the previous state vector, current measurement and noise.
  * Return the current state vector via reference.
  * The state vector is defined as stat = [positionX, positionY, velocityX, velocityY], unit = pix
  * The input / control vector is defined as input = [velocityX, 0, 0, 0], unit = pix / s
  * The output vector is defined as output = [positionX, positionY, velocityX, velocityY], unit = pix
  */
void kalman_filter();

/* Compute the matrix multiplication C = A * B
 * The size of A is 4*4 and the size of B is 4*4
 * The size of C is 4*4
 */
Matrix44 matrix_multiply_44(Matrix44 A, Matrix44 B);

/* Compute the matrix multiplication C = A * B
 * The size of A is 4*4 and the size of B is 4*1
 * The size of C is 4*1
 */
Matrix41 matrix_multiply_41(Matrix44 A, Matrix41 B);

/* Compute the multiplication of each element
 * The size of A is 4*4
 */
Matrix44 scalar_multiply_44(Matrix44 A, double n);

/* Compute the multiplication of each element
 * The size of B is 4*1
 */
Matrix41 scalar_multiply_41(Matrix41 B, double n);

/* Return the transpose of the matrix
 * The size of A is 4*4
 */
Matrix44 matrix_trans_44(Matrix44 A);

/* Compute the determinant of matrix A recursively.
 * Where k is the order of matrix.
 */
double determinant(Matrix44 A, int k);

/* Compute the adjugate of matrix A from determinant.
 * Where k is the order of matrix.
 */
Matrix44 adjugate(Matrix44 A, int k);

/* Compute the inverse of matrix A.
 */
Matrix44 inverse(Matrix44 A);

#endif /* _H_CTRL_OPERATION */
