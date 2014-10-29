/*****************************************************************************
 * Author: bitmingw
 * Kalman filter, system modeling and moving control.
 *****************************************************************************/

#include <csl.h>
#include <csl_emifa.h>
#include <csl_irq.h>
#include <csl_chip.h>
#include <csl_timer.h>

#include "ctrl_operation.h"

#if 0
void kalman_filter(double * stat_pre[], double * input[], double * noise[], double * stat_esti)
{
    /*************************************************************************/
    /* Introduction of parameters in Kalman Filter */
    
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
    /* Define a series of variables */
    double dt = 0.1;     /* Minimal time unit */
    
    /* The former state vector */
    extern Matrix41 X_pre;
    
    /* Predict state vector */
    extern Matrix41 X_predict;
    
    /* The state transition matrix F */
    Matrix44 F;

    /* The control input matrix B, give velocity (not acceleration) */
    Matrix41 B;
    
    /* The input value */
    extern double u;
    
    /* The former estimate covariance */
    extern Matrix44 P_pre;

    /* The noise ratio of control input */
    double noise_mag;

    /* The noise matrix of control input */
    Matrix44 Q;

    /*************************************************************************/
    
    
    
    /*************************************************************************/
    /* Initialize these variables */

    F.array[0][0] = 1;  F.array[0][1] = 0;  F.array[0][2] = dt; F.array[0][3] = 0;
    F.array[1][0] = 0;  F.array[1][1] = 1;  F.array[1][2] = 0;  F.array[1][3] = dt;
    F.array[2][0] = 0;  F.array[2][1] = 0;  F.array[2][2] = 1;  F.array[2][3] = 0;
    F.array[3][0] = 0;  F.array[3][1] = 0;  F.array[3][2] = 0;  F.array[3][3] = 1;

    B.array[0][0] = dt; B.array[1][0] = 0;  B.array[2][0] = 1;  F.array[3][0] = 0;

    Q.array[0][0] = 1;  Q.array[0][1] = 0;  Q.array[0][2] = dt; Q.array[0][3] = 0;
    Q.array[1][0] = 0;  Q.array[1][1] = 1;  Q.array[1][2] = 0;  Q.array[1][3] = dt;
    Q.array[2][0] = 0;  Q.array[2][1] = 0;  Q.array[2][2] = 1;  Q.array[2][3] = 0;
    Q.array[3][0] = 0;  Q.array[3][1] = 0;  Q.array[3][2] = 0;  Q.array[3][3] = 1;
    /* Predict the next state with the last state and predicted input */

}
#endif

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
