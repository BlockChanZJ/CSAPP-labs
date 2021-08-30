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
void trans_normal(int s, int ii, int jj, int r, int c, int M, int N, int A[N][M], int B[M][N]) {
    int i, j;
    for (i = 0; i < ii; i++) {
        for (j = 0; j < jj; j++) {
            B[c * s + j][r * s + i] = A[r * s + i][c * s + j];
        }
    }
}

void trans_square2(int r, int c, int N, int A[N][N], int B[N][N]) {
    int t0, t1, t2, t3;
    t0 = A[r][c];
    t1 = A[r][c + 1];
    t2 = A[r + 1][c];
    t3 = A[r + 1][c + 1];
    B[c][r] = t0;
    B[c][r + 1] = t2;
    B[c + 1][r] = t1;
    B[c + 1][r + 1] = t3;
}

void trans_square4_v2(int r, int c, int N, int A[N][N], int B[N][N]) {
    int t0, t1, t2, t3, t4, t5, t6, t7;
    t0 = A[r + 0][c + 0];
    t1 = A[r + 0][c + 1];
    t2 = A[r + 1][c + 0];
    t3 = A[r + 1][c + 1];
    t4 = A[r + 2][c + 0];
    t5 = A[r + 2][c + 1];
    t6 = A[r + 3][c + 0];
    t7 = A[r + 3][c + 1];
    B[c + 0][r + 0] = t0;
    B[c + 0][r + 1] = t2;
    B[c + 0][r + 2] = t4;
    B[c + 0][r + 3] = t6;
    B[c + 1][r + 0] = t1;
    B[c + 1][r + 1] = t3;
    B[c + 1][r + 2] = t5;
    B[c + 1][r + 3] = t7;


    t0 = A[r + 0][c + 2];
    t1 = A[r + 0][c + 3];
    t2 = A[r + 1][c + 2];
    t3 = A[r + 1][c + 3];
    t4 = A[r + 2][c + 2];
    t5 = A[r + 2][c + 3];
    t6 = A[r + 3][c + 2];
    t7 = A[r + 3][c + 3];
    B[c + 2][r + 0] = t0;
    B[c + 2][r + 1] = t2;
    B[c + 2][r + 2] = t4;
    B[c + 2][r + 3] = t6;
    B[c + 3][r + 0] = t1;
    B[c + 3][r + 1] = t3;
    B[c + 3][r + 2] = t5;
    B[c + 3][r + 3] = t7;
}

void trans_square4(int r, int c, int N, int A[N][N], int B[N][N]) {
    int t0, t1, t2, t3, t4, t5, t6, t7;
    t0 = A[r][c];
    t1 = A[r][c + 1];
    t2 = A[r][c + 2];
    t3 = A[r][c + 3];
    t4 = A[r + 1][c];
    t5 = A[r + 1][c + 1];
    t6 = A[r + 1][c + 2];
    t7 = A[r + 1][c + 3];
    B[c][r] = t0;
    B[c + 1][r] = t1;
    B[c + 2][r] = t2;
    B[c + 3][r] = t3;
    B[c][r + 1] = t4;
    B[c + 1][r + 1] = t5;
    B[c + 2][r + 1] = t6;
    B[c + 3][r + 1] = t7;

    t0 = A[r + 2][c];
    t1 = A[r + 2][c + 1];
    t2 = A[r + 2][c + 2];
    t3 = A[r + 2][c + 3];
    t4 = A[r + 3][c];
    t5 = A[r + 3][c + 1];
    t6 = A[r + 3][c + 2];
    t7 = A[r + 3][c + 3];
    B[c][r + 2] = t0;
    B[c + 1][r + 2] = t1;
    B[c + 2][r + 2] = t2;
    B[c + 3][r + 2] = t3;
    B[c][r + 3] = t4;
    B[c + 1][r + 3] = t5;
    B[c + 2][r + 3] = t6;
    B[c + 3][r + 3] = t7;

}

void trans_square8(int r, int c, int N, int A[N][N], int B[N][N]) {
    for (int i = 0; i < 8; i++) {
        int t0 = A[r * 8 + i][c * 8 + 0];
        int t1 = A[r * 8 + i][c * 8 + 1];
        int t2 = A[r * 8 + i][c * 8 + 2];
        int t3 = A[r * 8 + i][c * 8 + 3];
        int t4 = A[r * 8 + i][c * 8 + 4];
        int t5 = A[r * 8 + i][c * 8 + 5];
        int t6 = A[r * 8 + i][c * 8 + 6];
        int t7 = A[r * 8 + i][c * 8 + 7];
        B[c * 8 + 0][r * 8 + i] = t0;
        B[c * 8 + 1][r * 8 + i] = t1;
        B[c * 8 + 2][r * 8 + i] = t2;
        B[c * 8 + 3][r * 8 + i] = t3;
        B[c * 8 + 4][r * 8 + i] = t4;
        B[c * 8 + 5][r * 8 + i] = t5;
        B[c * 8 + 6][r * 8 + i] = t6;
        B[c * 8 + 7][r * 8 + i] = t7;
    }
}

void trans_square8_v2(int r, int c, int N, int A[N][N], int B[N][N]) {
    for (int i = 0; i < 4; i++) {
        int t1, t2, t3, t4, t5, t6, t7, t0;
        t0 = A[r * 8 + i * 2][c * 8 + 0];
        t1 = A[r * 8 + i * 2][c * 8 + 1];
        t2 = A[r * 8 + i * 2][c * 8 + 2];
        t3 = A[r * 8 + i * 2][c * 8 + 3];
        t4 = A[r * 8 + i * 2 + 1][c * 8 + 0];
        t5 = A[r * 8 + i * 2 + 1][c * 8 + 1];
        t6 = A[r * 8 + i * 2 + 1][c * 8 + 2];
        t7 = A[r * 8 + i * 2 + 1][c * 8 + 3];
        B[c * 8 + 0][r * 8 + i * 2] = t0;
        B[c * 8 + 1][r * 8 + i * 2] = t1;
        B[c * 8 + 2][r * 8 + i * 2] = t2;
        B[c * 8 + 3][r * 8 + i * 2] = t3;
        B[c * 8 + 0][r * 8 + i * 2 + 1] = t4;
        B[c * 8 + 1][r * 8 + i * 2 + 1] = t5;
        B[c * 8 + 2][r * 8 + i * 2 + 1] = t6;
        B[c * 8 + 3][r * 8 + i * 2 + 1] = t7;

        t0 = A[r * 8 + i * 2][c * 8 + 4];
        t1 = A[r * 8 + i * 2][c * 8 + 5];
        t2 = A[r * 8 + i * 2][c * 8 + 6];
        t3 = A[r * 8 + i * 2][c * 8 + 7];
        t4 = A[r * 8 + i * 2 + 1][c * 8 + 4];
        t5 = A[r * 8 + i * 2 + 1][c * 8 + 5];
        t6 = A[r * 8 + i * 2 + 1][c * 8 + 6];
        t7 = A[r * 8 + i * 2 + 1][c * 8 + 7];
        B[c * 8 + 4][r * 8 + i * 2] = t0;
        B[c * 8 + 4][r * 8 + i * 2 + 1] = t4;
        B[c * 8 + 5][r * 8 + i * 2] = t1;
        B[c * 8 + 5][r * 8 + i * 2 + 1] = t5;
        B[c * 8 + 6][r * 8 + i * 2] = t2;
        B[c * 8 + 6][r * 8 + i * 2 + 1] = t6;
        B[c * 8 + 7][r * 8 + i * 2] = t3;
        B[c * 8 + 7][r * 8 + i * 2 + 1] = t7;
    }
}

void trans_square8_v3(int r, int c, int N, int A[N][N], int B[N][N]) {
    int t1, t2, t3, t4, t5, t6, t7, t0;
    for (int i = 0; i < 2; i++) {
        t0 = A[r * 8 + i * 4 + 0][c * 8 + 0];
        t1 = A[r * 8 + i * 4 + 0][c * 8 + 1];
        t2 = A[r * 8 + i * 4 + 1][c * 8 + 0];
        t3 = A[r * 8 + i * 4 + 1][c * 8 + 1];
        t4 = A[r * 8 + i * 4 + 2][c * 8 + 0];
        t5 = A[r * 8 + i * 4 + 2][c * 8 + 1];
        t6 = A[r * 8 + i * 4 + 3][c * 8 + 0];
        t7 = A[r * 8 + i * 4 + 3][c * 8 + 1];
        B[c * 8 + 0][r * 8 + i * 4 + 0] = t0;
        B[c * 8 + 0][r * 8 + i * 4 + 1] = t2;
        B[c * 8 + 0][r * 8 + i * 4 + 2] = t4;
        B[c * 8 + 0][r * 8 + i * 4 + 3] = t6;
        B[c * 8 + 1][r * 8 + i * 4 + 0] = t1;
        B[c * 8 + 1][r * 8 + i * 4 + 1] = t3;
        B[c * 8 + 1][r * 8 + i * 4 + 2] = t5;
        B[c * 8 + 1][r * 8 + i * 4 + 3] = t7;

        t0 = A[r * 8 + i * 4 + 0][c * 8 + 2];
        t1 = A[r * 8 + i * 4 + 0][c * 8 + 3];
        t2 = A[r * 8 + i * 4 + 1][c * 8 + 2];
        t3 = A[r * 8 + i * 4 + 1][c * 8 + 3];
        t4 = A[r * 8 + i * 4 + 2][c * 8 + 2];
        t5 = A[r * 8 + i * 4 + 2][c * 8 + 3];
        t6 = A[r * 8 + i * 4 + 3][c * 8 + 2];
        t7 = A[r * 8 + i * 4 + 3][c * 8 + 3];
        B[c * 8 + 2][r * 8 + i * 4 + 0] = t0;
        B[c * 8 + 2][r * 8 + i * 4 + 1] = t2;
        B[c * 8 + 2][r * 8 + i * 4 + 2] = t4;
        B[c * 8 + 2][r * 8 + i * 4 + 3] = t6;
        B[c * 8 + 3][r * 8 + i * 4 + 0] = t1;
        B[c * 8 + 3][r * 8 + i * 4 + 1] = t3;
        B[c * 8 + 3][r * 8 + i * 4 + 2] = t5;
        B[c * 8 + 3][r * 8 + i * 4 + 3] = t7;

        t0 = A[r * 8 + i * 4 + 0][c * 8 + 2];
        t1 = A[r * 8 + i * 4 + 0][c * 8 + 3];
        t2 = A[r * 8 + i * 4 + 1][c * 8 + 2];
        t3 = A[r * 8 + i * 4 + 1][c * 8 + 3];
        t4 = A[r * 8 + i * 4 + 2][c * 8 + 2];
        t5 = A[r * 8 + i * 4 + 2][c * 8 + 3];
        t6 = A[r * 8 + i * 4 + 3][c * 8 + 2];
        t7 = A[r * 8 + i * 4 + 3][c * 8 + 3];
        B[c * 8 + 2][r * 8 + i * 4 + 0] = t0;
        B[c * 8 + 2][r * 8 + i * 4 + 1] = t2;
        B[c * 8 + 2][r * 8 + i * 4 + 2] = t4;
        B[c * 8 + 2][r * 8 + i * 4 + 3] = t6;
        B[c * 8 + 3][r * 8 + i * 4 + 0] = t1;
        B[c * 8 + 3][r * 8 + i * 4 + 1] = t3;
        B[c * 8 + 3][r * 8 + i * 4 + 2] = t5;
        B[c * 8 + 3][r * 8 + i * 4 + 3] = t7;

        t0 = A[r * 8 + i * 4 + 0][c * 8 + 4];
        t1 = A[r * 8 + i * 4 + 0][c * 8 + 5];
        t2 = A[r * 8 + i * 4 + 1][c * 8 + 4];
        t3 = A[r * 8 + i * 4 + 1][c * 8 + 5];
        t4 = A[r * 8 + i * 4 + 2][c * 8 + 4];
        t5 = A[r * 8 + i * 4 + 2][c * 8 + 5];
        t6 = A[r * 8 + i * 4 + 3][c * 8 + 4];
        t7 = A[r * 8 + i * 4 + 3][c * 8 + 5];
        B[c * 8 + 4][r * 8 + i * 4 + 0] = t0;
        B[c * 8 + 4][r * 8 + i * 4 + 1] = t2;
        B[c * 8 + 4][r * 8 + i * 4 + 2] = t4;
        B[c * 8 + 4][r * 8 + i * 4 + 3] = t6;
        B[c * 8 + 5][r * 8 + i * 4 + 0] = t1;
        B[c * 8 + 5][r * 8 + i * 4 + 1] = t3;
        B[c * 8 + 5][r * 8 + i * 4 + 2] = t5;
        B[c * 8 + 5][r * 8 + i * 4 + 3] = t7;

        t0 = A[r * 8 + i * 4 + 0][c * 8 + 6];
        t1 = A[r * 8 + i * 4 + 0][c * 8 + 7];
        t2 = A[r * 8 + i * 4 + 1][c * 8 + 6];
        t3 = A[r * 8 + i * 4 + 1][c * 8 + 7];
        t4 = A[r * 8 + i * 4 + 2][c * 8 + 6];
        t5 = A[r * 8 + i * 4 + 2][c * 8 + 7];
        t6 = A[r * 8 + i * 4 + 3][c * 8 + 6];
        t7 = A[r * 8 + i * 4 + 3][c * 8 + 7];
        B[c * 8 + 6][r * 8 + i * 4 + 0] = t0;
        B[c * 8 + 6][r * 8 + i * 4 + 1] = t2;
        B[c * 8 + 6][r * 8 + i * 4 + 2] = t4;
        B[c * 8 + 6][r * 8 + i * 4 + 3] = t6;
        B[c * 8 + 7][r * 8 + i * 4 + 0] = t1;
        B[c * 8 + 7][r * 8 + i * 4 + 1] = t3;
        B[c * 8 + 7][r * 8 + i * 4 + 2] = t5;
        B[c * 8 + 7][r * 8 + i * 4 + 3] = t7;
    }
}

void trans_square8_v4(int r, int c, int N, int A[N][N], int B[N][N]) {
    int t1, t2, t3, t4, t5, t6, t7, t0;
    for (int i = 0; i < 8; i++) {
        t0 = A[r * 8 + 0][c * 8 + i];
        t1 = A[r * 8 + 1][c * 8 + i];
        t2 = A[r * 8 + 2][c * 8 + i];
        t3 = A[r * 8 + 3][c * 8 + i];
        t4 = A[r * 8 + 4][c * 8 + i];
        t5 = A[r * 8 + 5][c * 8 + i];
        t6 = A[r * 8 + 6][c * 8 + i];
        t7 = A[r * 8 + 7][c * 8 + i];

        B[c * 8 + i][r * 8 + 0] = t0;
        B[c * 8 + i][r * 8 + 1] = t1;
        B[c * 8 + i][r * 8 + 2] = t2;
        B[c * 8 + i][r * 8 + 3] = t3;
        B[c * 8 + i][r * 8 + 4] = t4;
        B[c * 8 + i][r * 8 + 5] = t5;
        B[c * 8 + i][r * 8 + 6] = t6;
        B[c * 8 + i][r * 8 + 7] = t7;
    }
}

void trans_square8_v5(int r, int c, int N, int A[N][N], int B[N][N]) {
    int t0, t1, t2, t3, t4, t5, t6, t7;
    for (int i = 0; i < 4; i++) {
        t0 = A[r + i][c + 0];
        t1 = A[r + i][c + 1];
        t2 = A[r + i][c + 2];
        t3 = A[r + i][c + 3];
        t4 = A[r + i][c + 4];
        t5 = A[r + i][c + 5];
        t6 = A[r + i][c + 6];
        t7 = A[r + i][c + 7];
        B[c + 0][r + i] = t0;
        B[c + 1][r + i] = t1;
        B[c + 2][r + i] = t2;
        B[c + 3][r + i] = t3;
        B[c + 4][r + i] = t4;
        B[c + 5][r + i] = t5;
        B[c + 6][r + i] = t6;
        B[c + 7][r + i] = t7;
    }
    trans_square4_v2(r + 4, c, N, A, B);
    trans_square4_v2(r + 4, c + 4, N, A, B);
}

void trans_square16(int r, int c, int N, int A[N][N], int B[N][N]) {
    trans_square8(r * 2, c * 2, N, A, B);
    trans_square8(r * 2, c * 2 + 1, N, A, B);
    trans_square8(r * 2 + 1, c * 2, N, A, B);
    trans_square8(r * 2 + 1, c * 2 + 1, N, A, B);
}
void trans_square16_v2(int r, int c, int N, int A[N][N], int B[N][N]) {
    trans_square8_v2(r * 2, c * 2, N, A, B);
    trans_square8_v2(r * 2, c * 2 + 1, N, A, B);
    trans_square8_v2(r * 2 + 1, c * 2, N, A, B);
    trans_square8_v2(r * 2 + 1, c * 2 + 1, N, A, B);
}

void trans_square32(int r, int c, int N, int A[N][N], int B[N][N]) {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 4; j++) {
            int t0, t1, t2, t3, t4, t5, t6, t7;
            t0 = A[r * 32 + i][c * 32 + j * 8 + 0];
            t1 = A[r * 32 + i][c * 32 + j * 8 + 1];
            t2 = A[r * 32 + i][c * 32 + j * 8 + 2];
            t3 = A[r * 32 + i][c * 32 + j * 8 + 3];
            t4 = A[r * 32 + i][c * 32 + j * 8 + 4];
            t5 = A[r * 32 + i][c * 32 + j * 8 + 5];
            t6 = A[r * 32 + i][c * 32 + j * 8 + 6];
            t7 = A[r * 32 + i][c * 32 + j * 8 + 7];
            B[c * 32 + j * 8 + 0][r * 32 + i] = t0;
            B[c * 32 + j * 8 + 1][r * 32 + i] = t1;
            B[c * 32 + j * 8 + 2][r * 32 + i] = t2;
            B[c * 32 + j * 8 + 3][r * 32 + i] = t3;
            B[c * 32 + j * 8 + 4][r * 32 + i] = t4;
            B[c * 32 + j * 8 + 5][r * 32 + i] = t5;
            B[c * 32 + j * 8 + 6][r * 32 + i] = t6;
            B[c * 32 + j * 8 + 7][r * 32 + i] = t7;
        }
    }
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
    int r, c, ii, jj, square;
    if (M == 64) {
        int i, j, x, y;
        int x1, x2, x3, x4, x5, x6, x7, x8;
        for (i = 0; i < N; i += 8)
            for (j = 0; j < M; j += 8) {
                for (x = i; x < i + 4; ++x) {
                    x1 = A[x][j];
                    x2 = A[x][j + 1];
                    x3 = A[x][j + 2];
                    x4 = A[x][j + 3];
                    x5 = A[x][j + 4];
                    x6 = A[x][j + 5];
                    x7 = A[x][j + 6];
                    x8 = A[x][j + 7];

                    B[j][x] = x1;
                    B[j + 1][x] = x2;
                    B[j + 2][x] = x3;
                    B[j + 3][x] = x4;
                    B[j][x + 4] = x5;
                    B[j + 1][x + 4] = x6;
                    B[j + 2][x + 4] = x7;
                    B[j + 3][x + 4] = x8;
                }

                // cover
                for (y = j; y < j + 4; ++y) {
                    x1 = A[i + 4][y];
                    x2 = A[i + 5][y];
                    x3 = A[i + 6][y];
                    x4 = A[i + 7][y];
                    x5 = B[y][i + 4];
                    x6 = B[y][i + 5];
                    x7 = B[y][i + 6];
                    x8 = B[y][i + 7];

                    B[y][i + 4] = x1;
                    B[y][i + 5] = x2;
                    B[y][i + 6] = x3;
                    B[y][i + 7] = x4;
                    B[y + 4][i] = x5;
                    B[y + 4][i + 1] = x6;
                    B[y + 4][i + 2] = x7;
                    B[y + 4][i + 3] = x8;
                }
                for (x = i + 4; x < i + 8; ++x) {
                    x1 = A[x][j + 4];
                    x2 = A[x][j + 5];
                    x3 = A[x][j + 6];
                    x4 = A[x][j + 7];
                    B[j + 4][x] = x1;
                    B[j + 5][x] = x2;
                    B[j + 6][x] = x3;
                    B[j + 7][x] = x4;
                }
            }
//        for (int i = 0; i < 8; i++) {
//            for (int j = 0; j < 8; j++) {
//                trans_square4_v2(i * 4, j * 4, N, A, B);
//            }
//            for (int j = 0; j < 8; j++) {
//                trans_square4_v2(i * 4, j * 4 + 32, N, A, B);
//            }
//            for (int j = 0; j < 8; j++) {
//                trans_square4_v2(i * 4 + 32, j * 4, N, A, B);
//            }
//            for (int j = 0; j < 8; j++) {
//                trans_square4_v2(i * 4 + 32, j * 4 + 32, N, A, B);
//            }
//        }

    } else if (M == 32) {
        for (r = 0; r < 4; r++)
            for (c = 0; c < 4; c++)
                trans_square8(r, c, N, A, B);
    } else {
        square = 14;
        for (r = 0; r < ((N / square) + (N % square != 0)); r++) {
            for (c = 0; c < ((N / square + (N % square != 0))); c++) {
                ii = (r + 1) * square <= N ? square : (N - r * square);
                jj = (c + 1) * square <= N ? square : (M - c * square);
                trans_normal(square, ii, jj, r, c, M, N, A, B);
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */


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

