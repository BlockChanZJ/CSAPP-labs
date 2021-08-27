/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include <assert.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void trans_row_first(int s, int ii, int jj, int r, int c, int M, int N, int A[N][M], int B[M][N]) {
    int i, j;
    for (i = 0; i < ii; i++) {
        for (j = 0; j < jj; j++) {
            B[c * s + j][r * s + i] = A[r * s + i][c * s + j];
        }
    }
//    for (i = 0; i < ii; i++) {
//        for (j = 0; j < jj / 2; j++) {
//            B[c * s + j][r * s + i] = A[r * s + i][c * s + j];
//        }
//    }
//    for (i = 0; i < ii; i++) {
//        for (j = jj / 2; j < jj; j++) {
//            B[c * s + j][r * s + i] = A[r * s + i][c * s + j];
//        }
//    }
}
void trans_col_first(int s, int ii, int jj, int r, int c, int M, int N, int A[N][M], int B[M][N]) {
    int i, j;
    for (j = 0; j < jj; j++) {
        for (i = 0; i < ii; i++) {
            B[c * s + j][r * s + i] = A[r * s + i][c * s + j];
        }
    }
//    for (j = 0; j < jj; j++) {
//        for (i = 0; i < ii / 2; i++) {
//            B[c * s + j][r * s + i] = A[r * s + i][c * s + j];
//        }
//    }
//    for (j = 0; j < jj; j++) {
//        for (i = ii / 2; i < ii; i++) {
//            B[c * s + j][r * s + i] = A[r * s + i][c * s + j];
//        }
//    }
}
void trans_diag(int s, int ii, int jj, int r, int c, int M, int N, int A[N][M], int B[M][N]) {
    int i;
//    assert(ii == jj);
    for (i = 0; i < ii; i++)
        B[c * s + i][r * s + i] = A[r * s + i][c * s + i];
    for (int d = 1; d < ii; d++) {
        for (i = 0; i < ii - d; i++) {
            B[c * s + i + d][r * s + i] = A[r * s + i][c * s + i + d];
        }
    }
    for (int d = 1; d < ii; d++) {
        for (i = 0; i < ii - d; i++) {
            B[c * s + i][r * s + i + d] = A[r * s + i + d][c * s + i];
        }
    }
}

void trans_diag_first(int s, int ii, int jj, int r, int c, int M, int N, int A[N][M], int B[M][N]) {
    int i, j;

    // diag
    for (i = 0; i < ii; i++)
        B[c * s + i][r * s + i] = A[r * s + i][c * s + i];
    for (i = 0; i < ii; i++)
        for (j = 0; j < jj; j++)
            if (i == j) continue;
            else B[c * s + j][r * s + i] = A[r * s + i][c * s + j];
}

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    int r, c, ii, jj, square = 8;
    if (M != N) square = 14;
    for (r = 0; r < ((N / square) + (N % square != 0)); r++) {
        for (c = 0; c < ((N / square + (N % square != 0))); c++) {
            ii = (r + 1) * square <= N ? square : (N - r * square);
            jj = (c + 1) * square <= N ? square : (M - c * square);
            if (M != N)
                trans_row_first(square, ii, jj, r, c, M, N, A, B);
            else
                trans_diag_first(square, ii, jj, r, c, M, N, A, B);
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_normal_desc[] = "normal transpose";
void trans_normal(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

char trans_square16_desc[] = "square 16 transpose";
void trans_square16(int M, int N, int A[N][M], int B[M][N]) {
    int tmp, r, c, i, j, ii, jj, square = 16;
    for (r = 0; r < ((N / square) + (N % square != 0)); r++) {
        for (c = 0; c < ((N / square + (N % square != 0))); c++) {
            ii = (r + 1) * square <= N ? square : (N - r * square);
            jj = (c + 1) * square <= N ? square : (M - c * square);
            for (i = 0; i < ii; i++) {
                for (j = 0; j < jj; j++) {
                    tmp = A[r * square + i][c * square + j];
                    B[c * square + j][r * square + i] = tmp;
                }
            }
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans_normal, trans_normal_desc);
    registerTransFunction(trans_square16, trans_square16_desc);
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

