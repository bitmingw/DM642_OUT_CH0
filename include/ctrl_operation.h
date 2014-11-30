/*****************************************************************************
 * Author: Ming Wen
 * Kalman filter is implemented here to estimate the states of
 * moving object.
 * Calculation of matrix is also implemented to support the filter.
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

typedef struct matrix21 {
    double array[2][1];
} Matrix21;

typedef struct matrix22 {
    double array[2][2];
} Matrix22;

/* Macros of moving functions */
#define HOLDER_MOV_STAY   0
#define HOLDER_MOV_LEFT  -1
#define HOLDER_MOV_RIGHT  1
#define HOLDER_MOV_UNDEF  2

/*****************************************************************************/

/* Return a random number in (-1, 1)
 */
double sig_rand();


/* Set initial values to the global variables in Kalman filter
 */
void init_kalman_filter();


/* Kalman filter function.
 * Read the previous state vector, current measurement and noise.
 * Return the current state vector via reference.
 * The state vector is defined as [positionX, positionY]
 * The input is defined as pix / s
 * The output is defined as [positionX, positionY]
 */
void kalman_filter();

/*****************************************************************************/

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

/*****************************************************************************/

Matrix22 matrix_multiply_22(Matrix22 A, Matrix22 B);

Matrix21 matrix_multiply_21(Matrix22 A, Matrix21 B);

Matrix22 scalar_multiply_22(Matrix22 A, double n);

Matrix21 scalar_multiply_21(Matrix21 B, double n);

Matrix22 matrix_add_22(Matrix22 A, Matrix22 B);

Matrix21 matrix_add_21(Matrix21 A, Matrix21 B);

Matrix22 matrix_construct_22(Matrix21 A, Matrix21 B);

Matrix22 matrix_trans_22(Matrix22 A);

/*****************************************************************************/

/* Compute the determinant of matrix A recursively.
 * Where k is the order of matrix.
 */
double determinant(Matrix44 A, int k);


/* Compute the adjugate of matrix A from determinant.
 * Where k is the order of matrix.
 */
Matrix44 adjugate(Matrix44 A, int k);


/* Compute the inverse of matrix A.
 * The size of A is 4*4
 */
Matrix44 inverse4(Matrix44 A);


/* Compute the inverse of matrix A.
 * The size of A is 2*2
 */
Matrix22 inverse2(Matrix22 A);

#endif /* _H_CTRL_OPERATION */
