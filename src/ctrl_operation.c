/*****************************************************************************
 * Author: bitmingw
 * Kalman filter, system modeling and calculation.
 *****************************************************************************/

#include <stdlib.h>
#include <csl.h>

#include "ctrl_operation.h"

/* GLOBAL VARIABLES */
/*****************************************************************************/
    
/* Minimal time unit */
double dt = 0.1;

/* The former state vector */
Matrix21 X_pre;

/* The predicted state vector */
Matrix21 X_predict;

/* The state transition matrix F */
Matrix22 F;

/* The control input matrix B, give velocity (not acceleration) */
Matrix21 B;

/* The input value u */
double u;

/* The former estimate covariance */
Matrix22 P_pre;

/* The noise ratio of control input */
double sigma_u;

/* The noise matrix of control input Q */
Matrix22 Q;

/* The predicted covariance */
Matrix22 P_predict;

/* Measurement */
/* The output transformation matrix H */
Matrix22 H;

/* The noise ratio of measurement */
double sigma_z;

/* The measurement error v */
Matrix21 v;

/* The measurement of state vector */
Matrix21 X_measure;

/* The estimate output z */
Matrix21 z;

/* The noise matrix of measurement R */
Matrix22 R;

/* Update Stage */
/* The measurement residual y */
Matrix21 y;

/* The residual covariance matrix S */
Matrix22 S;

/* The optimal Kalman gain K */
Matrix22 K;

/* The estimate state vector */
Matrix21 X_post;

/* The unit matrix I */
Matrix22 I;

/* The estimate covariance */
Matrix22 P_post;

/*****************************************************************************/

double sig_rand()
{
    return ((double)rand() / RAND_MAX - 0.5) * 2;
}

void init_kalman_filter()
{
    /* variables in filter */
    extern Matrix21 X_pre, X_post;
    extern Matrix22 F;
    extern Matrix21 B;
    extern double u;
    extern double sigma_u, sigma_z;
    extern Matrix22 Q;
    extern Matrix22 P_pre, P_post;
    extern Matrix21 v;
    extern Matrix22 H;
    extern Matrix22 I;
    
    /* variables in main routine */
    extern int numPixels, numLines;
    
    X_post.array[0][0] = numPixels / 2;  /* For later iteration */
    X_post.array[1][0] = numLines / 2;
    
    F.array[0][0] = 1;  F.array[0][1] = 0;
    F.array[1][0] = 0;  F.array[1][1] = 1;
    
    B.array[0][0] = dt; B.array[1][0] = 0;
    
    u = 0;  /* Determined by moving object */
    sigma_u = 0;
    Q = matrix_construct_22(B, B);
    Q = scalar_multiply_22(Q, sigma_u);
    
    P_post.array[0][0] = 1;  P_post.array[0][1] = 0;    /* For later iteration */
    P_post.array[1][0] = 0;  P_post.array[1][1] = 1;
    
    sigma_z = 0;
    v.array[0][0] = dt;     v.array[1][0] = dt;
    R = matrix_construct_22(v, v);
    R = scalar_multiply_22(R, sigma_z);
    
    H.array[0][0] = 1;  H.array[0][1] = 0;
    H.array[1][0] = 0;  H.array[1][1] = 1;
    
    I.array[0][0] = 1;  I.array[0][1] = 0;
    I.array[1][0] = 0;  I.array[1][1] = 1;
}

void kalman_filter()
{
    /*************************************************************************/
    /* Parameters in Kalman Filter */
    
    /* Prediction Stage */
    /* X_predict = F * X_pre + B * u */ /* Predict the state estimate */
    /* P_predict = F * P_pre * F.transpose + Q */ /* Predict estimate covariance */
    
    /* Update Stage */
    /* y = z - H * X_predict */ /* The measurement residual */
    /* S = H * P_predict * H.transpose + R */ /* The residual covariance */
    /* K = P_predict * H.transpose * S^(-1) */ /* Optimal Kalman Gain */
    /* X_post = X_predict + K * y */ /* Update state estimate */
    /* P_post = (I - K * H) * P_predict */ /* Update estimate covariance */
    
    /* Where */
    /* Q = B * B.transpose * var_of_u */
    /* z = H * X_measure + v */
    /* R = v * v.transpose * var_of_m */
    
    /*************************************************************************/
    /* Prediction Stage */
    /* Minimal time unit */
    extern double dt;
    
    /* The former state vector */
    extern Matrix21 X_pre;
    
    /* The predicted state vector */
    extern Matrix21 X_predict;
    
    /* The state transition matrix F */
    extern Matrix22 F;

    /* The control input matrix B, give velocity (not acceleration) */
    extern Matrix21 B;
    
    /* The input value u */
    extern double u;
    
    /* The former estimate covariance */
    extern Matrix22 P_pre;

    /* The noise ratio of control input */
    extern double sigma_u;

    /* The noise matrix of control input Q */
    extern Matrix22 Q;
    
    /* The predicted covariance */
    extern Matrix22 P_predict;

    /*************************************************************************/
    /* Measurement */
    /* The output transformation matrix H */
    extern Matrix22 H;
    
    /* The noise ratio of measurement */
    extern double sigma_z;   
    
    /* The measurement error v */
    extern Matrix21 v;
    
    /* The measurement of state vector */
    extern Matrix21 X_measure;
    
    /* The estimate output z */
    extern Matrix21 z;
    
    /* The noise matrix of measurement R */
    extern Matrix22 R;
    
    /*************************************************************************/
    /* Update Stage */
    /* The measurement residual y */
    extern Matrix21 y;
    
    /* The residual covariance matrix S */
    extern Matrix22 S;
    
    /* The optimal Kalman gain K */
    extern Matrix22 K;
    
    /* The estimate state vector */
    extern Matrix21 X_post;
    
    /* The unit matrix I */
    extern Matrix22 I;
    
    /* The estimate covariance */
    extern Matrix22 P_post;
    
    /*************************************************************************/
    Matrix21 temp21_1;
    Matrix21 temp21_2;
    Matrix22 temp22_1;
    Matrix22 temp22_2;
    
    /* Add noise into process and measurement matrix */
    Q.array[0][0] = sigma_u * sig_rand();
    Q.array[1][1] = sigma_u * sig_rand();
    v.array[0][0] = sigma_z * sig_rand();
    v.array[1][0] = sigma_z * sig_rand();
    R = matrix_construct_22(v, v);
    
    /* Predict Stage */
    temp21_1 = matrix_multiply_21(F, X_pre);
    temp21_2 = scalar_multiply_21(B, u);
    X_predict = matrix_add_21(temp21_1, temp21_2);
    
    temp22_1 = matrix_multiply_22(F, P_pre);
    temp22_2 = matrix_trans_22(F);
    temp22_1 = matrix_multiply_22(temp22_1, temp22_2);
    P_predict = matrix_add_22(temp22_1, Q);
    
    /* Do Measurement */
    temp21_1 = matrix_multiply_21(H, X_measure);
    z = matrix_add_21(temp21_1, v);
    
    /* Update Stage */
    temp21_1 = matrix_multiply_21(H, X_predict);
    temp21_1 = scalar_multiply_21(temp21_1, -1);
    y = matrix_add_21(z, temp21_1);
    
    temp22_1 = matrix_multiply_22(H, P_predict);
    temp22_2 = matrix_trans_22(H);
    temp22_1 = matrix_multiply_22(temp22_1, temp22_2);
    S = matrix_add_22(temp22_1, R);
    
    temp22_1 = matrix_trans_22(H);
    temp22_1 = matrix_multiply_22(P_predict, temp22_1);
    temp22_2 = inverse2(S);
    K = matrix_multiply_22(temp22_1, temp22_2);
    
    temp21_1 = matrix_multiply_21(K, y);
    X_post = matrix_add_21(X_predict, temp21_1);
    
    temp22_1 = matrix_multiply_22(K, H);
    temp22_1 = scalar_multiply_22(temp22_1, -1);
    temp22_1 = matrix_add_22(I, temp22_1);
    P_post = matrix_multiply_22(temp22_1, P_predict);
}

/*****************************************************************************/

Matrix44 matrix_multiply_44(Matrix44 A, Matrix44 B)
{
    Matrix44 C;
    int i, j, k;
    /* Clear the values in C */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            C.array[i][j] = 0.0;
            for (k = 0; k < 4; k++) {
                C.array[i][j] += A.array[i][k] * B.array[k][j];
            }
        }
    }
    return C;
}

Matrix41 matrix_multiply_41(Matrix44 A, Matrix41 B)
{
    Matrix41 C;
    int i, j;

    /* Clear the values in C */
    for (i = 0; i < 4; i++) {
        C.array[i][0] = 0.0;
    }
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            C.array[i][0] += A.array[i][j] * B.array[j][0];
        }
    }
    return C;
}

Matrix44 scalar_multiply_44(Matrix44 A, double n)
{
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            A.array[i][j] *= n;
        }
    }
    return A;
}

Matrix41 scalar_multiply_41(Matrix41 B, double n)
{
    int i;
    for (i = 0; i < 4; i++) {
        B.array[i][0] *= n;
    }
    return B;
}

Matrix44 matrix_trans_44(Matrix44 A)
{
    Matrix44 Ap;
    int i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            Ap.array[i][j] = A.array[j][i];
        }
    }
    return Ap;
}

/*****************************************************************************/

Matrix22 matrix_multiply_22(Matrix22 A, Matrix22 B)
{
    Matrix22 C;
    int i, j, k;
    /* Clear the values in C */
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            C.array[i][j] = 0.0;
            for (k = 0; k < 2; k++) {
                C.array[i][j] += A.array[i][k] * B.array[k][j];
            }
        }
    }
    return C;
}

Matrix21 matrix_multiply_21(Matrix22 A, Matrix21 B)
{
    Matrix21 C;
    int i, j;

    /* Clear the values in C */
    for (i = 0; i < 2; i++) {
        C.array[i][0] = 0.0;
    }
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            C.array[i][0] += A.array[i][j] * B.array[j][0];
        }
    }
    return C;
}

Matrix22 scalar_multiply_22(Matrix22 A, double n)
{
    int i, j;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            A.array[i][j] *= n;
        }
    }
    return A;
}

Matrix21 scalar_multiply_21(Matrix21 B, double n)
{
    int i;
    for (i = 0; i < 2; i++) {
        B.array[i][0] *= n;
    }
    return B;
}

Matrix22 matrix_add_22(Matrix22 A, Matrix22 B)
{
    Matrix22 sum;
    int i, j;
    
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            sum.array[i][j] = A.array[i][j] + B.array[i][j];
        }
    }
    return sum;
}

Matrix21 matrix_add_21(Matrix21 A, Matrix21 B)
{
    Matrix21 sum;
    int i;
    
    for (i = 0; i < 2; i++) {
        sum.array[i][0] = A.array[i][0] + B.array[i][0];
    }
    return sum;
}

Matrix22 matrix_construct_22(Matrix21 A, Matrix21 B)
{
    Matrix22 C;
    C.array[0][0] = A.array[0][0] * B.array[0][0];
    C.array[0][1] = A.array[0][0] * B.array[1][0];
    C.array[1][0] = A.array[1][0] * B.array[0][0];
    C.array[1][1] = A.array[1][0] * B.array[1][0];
    return C;
}

Matrix22 matrix_trans_22(Matrix22 A)
{
    Matrix22 Ap;
    int i, j;

    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            Ap.array[i][j] = A.array[j][i];
        }
    }
    return Ap;
}

/*****************************************************************************/

double determinant(Matrix44 A, int k)
{
    int sign = 1;
    double det = 0;
    Matrix44 minor;
    int i, j, m, n, c;
    
    /* Base case */
    if (k == 1) {
        return A.array[0][0];
    }
    /* Do recursion */
    else {
        det = 0;
        for (c = 0; c < k; c++) {   /* c -> the chosen column to be exclusive */
            m = 0;  /* the row in submatrix, index start from 0 */
            n = 0;  /* the column in submatrix, index start from 0 */
            for (i = 0; i < k; i++) {       /* i -> iterate over the original matrix */
                for (j = 0; j < k; j++) {   /* j -> iterate over the original matrix */
                    minor.array[i][j] = 0;
                    if (i != 0 && j != c) {
                        minor.array[m][n] = A.array[i][j];
                        if (n < (k-2)) {
                            n++;
                        }
                        else {
                            n = 0;
                            m++;
                        }
                    }
                }
            }
            det = det + sign * (A.array[0][c] * determinant(minor, k-1));
            sign *= -1; /* odd column positive, even column negative */
        }
        return det;
    }
}

Matrix44 adjugate(Matrix44 A, int k)
{
    Matrix44 B;
    Matrix44 fac;
    int sign = 1;
    int p, q, m, n, i, j;
    for (q = 0; q < k; q++) {       /* q -> the chosen of row to be exclusive */
        for (p = 0; p < k; p++) {   /* p -> the chosen of column to be exclusive */
            m = 0;  /* the row in submatrix, index start from 0 */
            n = 0;  /* the column in submatrix, index start from 0 */
            for (i = 0; i < k; i++) {       /* i -> iterate over the original matrix */
                for (j = 0; j < k; j++) {   /* j -> iterate over the original matrix */
                    if (i != q && j != p) {
                        B.array[m][n] = A.array[i][j];
                        if (n < (k-2)) {
                            n++;
                        }
                        else {
                            n = 0;
                            m++;
                        }
                    }
                }
            }
            sign = (q+p)%2 ? -1 : 1;
            fac.array[q][p] = sign * determinant(B, k-1);
        }
    }
    
    /* Compute the transpose */
    return matrix_trans_44(fac);
}

Matrix44 inverse4(Matrix44 A)
{
    double det;
    Matrix44 adj;
    
    det = determinant(A, 4);
    adj = adjugate(A, 4);
    return scalar_multiply_44(adj, 1 / det);
}

Matrix22 inverse2(Matrix22 A)
{
    Matrix22 inv;
    inv.array[0][0] = A.array[1][1];
    inv.array[0][1] = -A.array[0][1];
    inv.array[1][0] = -A.array[1][0];
    inv.array[1][1] = A.array[0][0];
    return scalar_multiply_22(inv, (A.array[0][0] * A.array[1][1] - A.array[0][1] * A.array[1][0]));
}
