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


void kalman_filter(double * stat_pre[], double * input[], double * noise[], double * stat_esti)
{
    /* Define a series of variables */
    double dt = 0.1;     /* Minimal time unit */

    /* The state transition matrix F */
    Matrix44 F;

    /* The control input matrix B, give velocity rather than acceleration */
    Matrix41 B;

    /* The noise ratio of the state */
    double noise_mag;

    /* The noise matrix of control inputs */
    Matrix44 Qt;

    /*************************************************************************/
    /* Initialize these variables */

    F.array[0][0] = 1;  F.array[0][1] = 0;  F.array[0][2] = dt; F.array[0][3] = 0;
    F.array[1][0] = 0;  F.array[1][1] = 1;  F.array[1][2] = 0;  F.array[1][3] = dt;
    F.array[2][0] = 0;  F.array[2][1] = 0;  F.array[2][2] = 1;  F.array[2][3] = 0;
    F.array[3][0] = 0;  F.array[3][1] = 0;  F.array[3][2] = 0;  F.array[3][3] = 1;

    B.array[0][0] = dt; B.array[1][0] = 0;  B.array[2][0] = 1;  F.array[3][0] = 0;

    /* Predict the next state with the last state and predicted input */

}

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
