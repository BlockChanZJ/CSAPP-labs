/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * Please fill in the following team struct 
 */
team_t team = {
        "QWQ",              /* Team name */

        "BlockChanZJ",     /* First member full name */
        "blockchanzj@163.com",  /* First member email address */

        "",                   /* Second member full name (leave blank if none) */
        ""                    /* Second member email addr (leave blank if none) */
};


/*********
 * HELPER
 *********/

void rotate_square(int s, int dim, pixel *src, pixel *dst) {
    for (int r = 0; r < dim / s; r++)
        for (int c = 0; c < dim / s; c++) {
            for (int i = 0; i < s; i++)
                for (int j = 0; j < s; j++)
                    dst[RIDX(dim - 1 - (r * s + i), c * s + j, dim)] = src[RIDX(c * s + j, r * s + i, dim)];
        }
}

void transpose(int s, int dim, pixel *src, pixel *dst) {
    for (int r = 0; r < dim / s; r++)
        for (int c = 0; c < dim / s; c++)
            for (int i = 0; i < s; i++)
                for (int j = 0; j < s; j++)
                    dst[RIDX(r * s + i, c * s + j, dim)] = src[RIDX(c * s + j, r * s + i, dim)];
}


void exchange_rows(int dim, pixel *src, pixel *dst) {
    for (int i = 0; i < dim / 2; i++)
        for (int j = 0; j < dim; j++) {
            pixel tmp = dst[RIDX(i, j, dim)];
            dst[RIDX(i, j, dim)] = dst[RIDX(dim - 1 - i, j, dim)];
            dst[RIDX(dim - 1 - i, j, dim)] = tmp;
        }
}

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) {
    int i, j;

    for (i = 0; i < dim; i++)
        for (j = 0; j < dim; j++)
            dst[RIDX(dim - 1 - j, i, dim)] = src[RIDX(i, j, dim)];
}

char move_ptr_rotate_descr[] = "move_ptr_rotate: move ptr implementation";
void move_ptr_rotate(int dim, pixel *src, pixel *dst) {
    int i, j;
    dst += (dim * dim - dim);
    for (i = 0; i < dim; i += 16) {
        for (j = 0; j < dim; j++) {
            dst[0] = src[0];
            dst[1] = src[dim];
            dst[2] = src[2 * dim];
            dst[3] = src[3 * dim];
            dst[4] = src[4 * dim];
            dst[5] = src[5 * dim];
            dst[6] = src[6 * dim];
            dst[7] = src[7 * dim];
            dst[8] = src[8 * dim];
            dst[9] = src[9 * dim];
            dst[10] = src[10 * dim];
            dst[11] = src[11 * dim];
            dst[12] = src[12 * dim];
            dst[13] = src[13 * dim];
            dst[14] = src[14 * dim];
            dst[15] = src[15 * dim];
            src++;      //j++ => src+=1
            dst -= dim;   //j++ => dim+=-dim
        }
        //i+=32 => src+=32*dim, then neutralize the effects of for(j)
        src += 15 * dim;
        //i+=32 => dst+=32, then neutralize the effects of for(j)
        dst += dim * dim + 16;
    }
}

char optimized_naive_rotate_descr[] = "optimized_naive_rotate: Optimized Naive implementation";
void optimized_naive_rotate(int dim, pixel *src, pixel *dst) {
    rotate_square(16, dim, src, dst);
}

char transpose_and_exchange_rows_descr[] = "transpose and exchange rows";
void transpose_and_exchange_rows(int dim, pixel *src, pixel *dst) {
    transpose(16, dim, src, dst);
    exchange_rows(dim, src, dst);
}

/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) {
    naive_rotate(dim, src, dst);
}

/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() {
//    add_rotate_function(&naive_rotate, naive_rotate_descr);
//    add_rotate_function(&rotate, rotate_descr);
    add_rotate_function(&optimized_naive_rotate, optimized_naive_rotate_descr);
//    add_rotate_function(&transpose_and_exchange_rows, transpose_and_exchange_rows_descr);
    add_rotate_function(&move_ptr_rotate, move_ptr_rotate_descr);
    /* ... Register additional test functions here */
}


/***************
 * SMOOTH KERNEL
 **************/

/***************************************************************
 * Various typedefs and helper functions for the smooth function
 * You may modify these any way you like.
 **************************************************************/

/* A struct used to compute averaged pixel value */
typedef struct {
    int red;
    int green;
    int blue;
    int num;
} pixel_sum;

/* Compute min and max of two integers, respectively */
static int min(int a, int b) { return (a < b ? a : b); }
static int max(int a, int b) { return (a > b ? a : b); }

/* 
 * initialize_pixel_sum - Initializes all fields of sum to 0 
 */
static void initialize_pixel_sum(pixel_sum *sum) {
    sum->red = sum->green = sum->blue = 0;
    sum->num = 0;
    return;
}

/* 
 * accumulate_sum - Accumulates field values of p in corresponding 
 * fields of sum 
 */
inline void accumulate_sum(pixel_sum *sum, pixel p) {
    sum->red += (int) p.red;
    sum->green += (int) p.green;
    sum->blue += (int) p.blue;
    sum->num++;
    return;
}

inline void subtract_sum(pixel_sum *sum, pixel p) {
    sum->red -= (int) p.red;
    sum->green -= (int) p.green;
    sum->blue -= (int) p.blue;
    sum->num--;
    return;
}

inline void add(pixel_sum *sum1, pixel_sum *sum2) {
    sum1->red += sum2->red;
    sum1->green += sum2->green;
    sum1->blue += sum2->blue;
    sum1->num += sum2->num;
}

inline void sub(pixel_sum *sum1, pixel_sum *sum2) {
    sum1->red -= sum2->red;
    sum1->green -= sum2->green;
    sum1->blue -= sum2->blue;
    sum1->num -= sum2->num;
}

/* 
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel 
 */
inline void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum) {
    current_pixel->red = (unsigned short) (sum.red / sum.num);
    current_pixel->green = (unsigned short) (sum.green / sum.num);
    current_pixel->blue = (unsigned short) (sum.blue / sum.num);
    return;
}

/* 
 * avg - Returns averaged pixel value at (i,j) 
 */
static pixel avg(int dim, int i, int j, pixel *src) {
    int ii, jj;
    pixel_sum sum;
    pixel current_pixel;

    initialize_pixel_sum(&sum);
    for (ii = max(i - 1, 0); ii <= min(i + 1, dim - 1); ii++)
        for (jj = max(j - 1, 0); jj <= min(j + 1, dim - 1); jj++)
            accumulate_sum(&sum, src[RIDX(ii, jj, dim)]);

    assign_sum_to_pixel(&current_pixel, sum);
    return current_pixel;
}

/******************************************************
 * Your different versions of the smooth kernel go here
 ******************************************************/

/*
 * naive_smooth - The naive baseline version of smooth 
 */
char naive_smooth_descr[] = "naive_smooth: Naive baseline implementation";
void naive_smooth(int dim, pixel *src, pixel *dst) {
    int i, j;
    for (i = 0; i < dim; i++)
        for (j = 0; j < dim; j++)
            dst[RIDX(i, j, dim)] = avg(dim, i, j, src);


}

/*
 * smooth - Your current working version of smooth. 
 * IMPORTANT: This is the version you will be graded on
 */

#define N 520
pixel_sum sum[N][N];

char brute_force_smooth_descr[] = "Brute Force";
void brute_force_smooth(int dim, pixel *src, pixel *dst) {
    int i, j;

    pixel current_pixel;
    pixel *pcurrent_pixel = &current_pixel;

    i = 0;
    j = 0;
    pcurrent_pixel->red =
            (unsigned short) (((int) (src[RIDX(0, 0, dim)].red + src[RIDX(0, 1, dim)].red +
                                      src[RIDX(1, 0, dim)].red + src[RIDX(1, 1, dim)].red)) /
                              4);
    pcurrent_pixel->green =
            (unsigned short) (((int) (src[RIDX(0, 0, dim)].green + src[RIDX(0, 1, dim)].green +
                                      src[RIDX(1, 0, dim)].green + src[RIDX(1, 1, dim)].green)) /
                              4);
    pcurrent_pixel->blue =
            (unsigned short) (((int) (src[RIDX(0, 0, dim)].blue + src[RIDX(0, 1, dim)].blue +
                                      src[RIDX(1, 0, dim)].blue + src[RIDX(1, 1, dim)].blue)) /
                              4);
    dst[RIDX(0, 0, dim)] = current_pixel;

    i = 0;
    j = dim - 1;
    pcurrent_pixel->red =
            (unsigned short) (((int) (src[RIDX(i, j, dim)].red + src[RIDX(i + 1, j, dim)].red +
                                      src[RIDX(i, j - 1, dim)].red + src[RIDX(i + 1, j - 1, dim)].red)) /
                              4);
    pcurrent_pixel->green =
            (unsigned short) (((int) (src[RIDX(i, j, dim)].green + src[RIDX(i + 1, j, dim)].green +
                                      src[RIDX(i, j - 1, dim)].green + src[RIDX(i + 1, j - 1, dim)].green)) /
                              4);
    pcurrent_pixel->blue =
            (unsigned short) (((int) (src[RIDX(i, j, dim)].blue + src[RIDX(i + 1, j, dim)].blue +
                                      src[RIDX(i, j - 1, dim)].blue + src[RIDX(i + 1, j - 1, dim)].blue)) /
                              4);
    dst[RIDX(i, j, dim)] = current_pixel;

    i = dim - 1;
    j = 0;
    pcurrent_pixel->red =
            (unsigned short) (((int) (src[RIDX(i, j, dim)].red + src[RIDX(i - 1, j, dim)].red +
                                      src[RIDX(i, j + 1, dim)].red + src[RIDX(i - 1, j + 1, dim)].red)) /
                              4);
    pcurrent_pixel->green =
            (unsigned short) (((int) (src[RIDX(i, j, dim)].green + src[RIDX(i - 1, j, dim)].green +
                                      src[RIDX(i, j + 1, dim)].green + src[RIDX(i - 1, j + 1, dim)].green)) /
                              4);
    pcurrent_pixel->blue =
            (unsigned short) (((int) (src[RIDX(i, j, dim)].blue + src[RIDX(i - 1, j, dim)].blue +
                                      src[RIDX(i, j + 1, dim)].blue + src[RIDX(i - 1, j + 1, dim)].blue)) /
                              4);
    dst[RIDX(i, j, dim)] = current_pixel;

    i = dim - 1;
    j = dim - 1;
    pcurrent_pixel->red =
            (unsigned short) (((int) (src[RIDX(i, j, dim)].red + src[RIDX(i - 1, j, dim)].red +
                                      src[RIDX(i, j - 1, dim)].red + src[RIDX(i - 1, j - 1, dim)].red)) /
                              4);
    pcurrent_pixel->green =
            (unsigned short) (((int) (src[RIDX(i, j, dim)].green + src[RIDX(i - 1, j, dim)].green +
                                      src[RIDX(i, j - 1, dim)].green + src[RIDX(i - 1, j - 1, dim)].green)) /
                              4);
    pcurrent_pixel->blue =
            (unsigned short) (((int) (src[RIDX(i, j, dim)].blue + src[RIDX(i - 1, j, dim)].blue +
                                      src[RIDX(i, j - 1, dim)].blue + src[RIDX(i - 1, j - 1, dim)].blue)) /
                              4);
    dst[RIDX(i, j, dim)] = current_pixel;

    j = 0;
    for (i = 1; i < dim - 1; i++) {
        pcurrent_pixel->red =
                (unsigned short) (((int) (src[RIDX(i - 1, j, dim)].red + src[RIDX(i - 1, j + 1, dim)].red +
                                          src[RIDX(i, j, dim)].red + src[RIDX(i, j + 1, dim)].red +
                                          src[RIDX(i + 1, j, dim)].red + src[RIDX(i + 1, j + 1, dim)].red)) /
                                  6);
        pcurrent_pixel->green =
                (unsigned short) (((int) (src[RIDX(i - 1, j, dim)].green + src[RIDX(i - 1, j + 1, dim)].green +
                                          src[RIDX(i, j, dim)].green + src[RIDX(i, j + 1, dim)].green +
                                          src[RIDX(i + 1, j, dim)].green + src[RIDX(i + 1, j + 1, dim)].green)) /
                                  6);
        pcurrent_pixel->blue =
                (unsigned short) (((int) (src[RIDX(i - 1, j, dim)].blue + src[RIDX(i - 1, j + 1, dim)].blue +
                                          src[RIDX(i, j, dim)].blue + src[RIDX(i, j + 1, dim)].blue +
                                          src[RIDX(i + 1, j, dim)].blue + src[RIDX(i + 1, j + 1, dim)].blue)) /
                                  6);
        dst[RIDX(i, j, dim)] = current_pixel;
    }

    i = dim - 1;
    for (j = 1; j < dim - 1; j++) {
        pcurrent_pixel->red =
                (unsigned short) (((int) (src[RIDX(i, j, dim)].red + src[RIDX(i - 1, j, dim)].red +
                                          src[RIDX(i, j - 1, dim)].red + src[RIDX(i - 1, j - 1, dim)].red +
                                          src[RIDX(i, j + 1, dim)].red + src[RIDX(i - 1, j + 1, dim)].red)) /
                                  6);
        pcurrent_pixel->green =
                (unsigned short) (((int) (src[RIDX(i, j, dim)].green + src[RIDX(i - 1, j, dim)].green +
                                          src[RIDX(i, j - 1, dim)].green + src[RIDX(i - 1, j - 1, dim)].green +
                                          src[RIDX(i, j + 1, dim)].green + src[RIDX(i - 1, j + 1, dim)].green)) /
                                  6);
        pcurrent_pixel->blue =
                (unsigned short) (((int) (src[RIDX(i, j, dim)].blue + src[RIDX(i - 1, j, dim)].blue +
                                          src[RIDX(i, j - 1, dim)].blue + src[RIDX(i - 1, j - 1, dim)].blue +
                                          src[RIDX(i, j + 1, dim)].blue + src[RIDX(i - 1, j + 1, dim)].blue)) /
                                  6);
        dst[RIDX(i, j, dim)] = current_pixel;
    }

    j = dim - 1;
    for (i = 1; i < dim - 1; i++) {
        pcurrent_pixel->red =
                (unsigned short) (((int) (src[RIDX(i - 1, j, dim)].red + src[RIDX(i - 1, j - 1, dim)].red +
                                          src[RIDX(i, j, dim)].red + src[RIDX(i, j - 1, dim)].red +
                                          src[RIDX(i + 1, j, dim)].red + src[RIDX(i + 1, j - 1, dim)].red)) /
                                  6);
        pcurrent_pixel->green =
                (unsigned short) (((int) (src[RIDX(i - 1, j, dim)].green + src[RIDX(i - 1, j - 1, dim)].green +
                                          src[RIDX(i, j, dim)].green + src[RIDX(i, j - 1, dim)].green +
                                          src[RIDX(i + 1, j, dim)].green + src[RIDX(i + 1, j - 1, dim)].green)) /
                                  6);
        pcurrent_pixel->blue =
                (unsigned short) (((int) (src[RIDX(i - 1, j, dim)].blue + src[RIDX(i - 1, j - 1, dim)].blue +
                                          src[RIDX(i, j, dim)].blue + src[RIDX(i, j - 1, dim)].blue +
                                          src[RIDX(i + 1, j, dim)].blue + src[RIDX(i + 1, j - 1, dim)].blue)) /
                                  6);
        dst[RIDX(i, j, dim)] = current_pixel;
    }

    i = 0;
    for (j = 1; j < dim - 1; j++) {
        pcurrent_pixel->red =
                (unsigned short) (((int) (src[RIDX(i, j, dim)].red + src[RIDX(i + 1, j, dim)].red +
                                          src[RIDX(i, j - 1, dim)].red + src[RIDX(i + 1, j - 1, dim)].red +
                                          src[RIDX(i, j + 1, dim)].red + src[RIDX(i + 1, j + 1, dim)].red)) /
                                  6);
        pcurrent_pixel->green =
                (unsigned short) (((int) (src[RIDX(i, j, dim)].green + src[RIDX(i + 1, j, dim)].green +
                                          src[RIDX(i, j - 1, dim)].green + src[RIDX(i + 1, j - 1, dim)].green +
                                          src[RIDX(i, j + 1, dim)].green + src[RIDX(i + 1, j + 1, dim)].green)) /
                                  6);
        pcurrent_pixel->blue =
                (unsigned short) (((int) (src[RIDX(i, j, dim)].blue + src[RIDX(i + 1, j, dim)].blue +
                                          src[RIDX(i, j - 1, dim)].blue + src[RIDX(i + 1, j - 1, dim)].blue +
                                          src[RIDX(i, j + 1, dim)].blue + src[RIDX(i + 1, j + 1, dim)].blue)) /
                                  6);
        dst[RIDX(i, j, dim)] = current_pixel;
    }


    for (i = 1; i < dim - 1; i++) {
        for (j = 1; j < dim - 1; j++) {
            pcurrent_pixel->red =
                    (unsigned short) (((int) (src[RIDX(i + 1, j, dim)].red + src[RIDX(i + 1, j - 1, dim)].red +
                                              src[RIDX(i, j, dim)].red + src[RIDX(i - 1, j, dim)].red +
                                              src[RIDX(i, j - 1, dim)].red + src[RIDX(i - 1, j - 1, dim)].red +
                                              src[RIDX(i, j + 1, dim)].red + src[RIDX(i - 1, j + 1, dim)].red +
                                              src[RIDX(i + 1, j + 1, dim)].red)) /
                                      9);
            pcurrent_pixel->green =
                    (unsigned short) (((int) (src[RIDX(i + 1, j, dim)].green + src[RIDX(i + 1, j - 1, dim)].green +
                                              src[RIDX(i, j, dim)].green + src[RIDX(i - 1, j, dim)].green +
                                              src[RIDX(i, j - 1, dim)].green + src[RIDX(i - 1, j - 1, dim)].green +
                                              src[RIDX(i, j + 1, dim)].green + src[RIDX(i - 1, j + 1, dim)].green +
                                              src[RIDX(i + 1, j + 1, dim)].green)) /
                                      9);
            pcurrent_pixel->blue =
                    (unsigned short) (((int) (src[RIDX(i + 1, j, dim)].blue + src[RIDX(i + 1, j - 1, dim)].blue +
                                              src[RIDX(i, j, dim)].blue + src[RIDX(i - 1, j, dim)].blue +
                                              src[RIDX(i, j - 1, dim)].blue + src[RIDX(i - 1, j - 1, dim)].blue +
                                              src[RIDX(i, j + 1, dim)].blue + src[RIDX(i - 1, j + 1, dim)].blue +
                                              src[RIDX(i + 1, j + 1, dim)].blue)) /
                                      9);
            dst[RIDX(i, j, dim)] = current_pixel;
        }
    }
}


char dynamic_programming_smooth_descr[] = "dynamic programming smooth";
void dynamic_programming_smooth(int dim, pixel *src, pixel *dst) {
    int i, j;
    for (i = 0; i < dim; i++)
        for (j = 0; j < dim; j++)
            initialize_pixel_sum(&sum[i][j]);

    // top-left [0, 0]
    accumulate_sum(&sum[0][0], src[RIDX(0, 0, dim)]);
    accumulate_sum(&sum[0][0], src[RIDX(0, 1, dim)]);
    accumulate_sum(&sum[0][0], src[RIDX(1, 0, dim)]);
    accumulate_sum(&sum[0][0], src[RIDX(1, 1, dim)]);
    assign_sum_to_pixel(&dst[RIDX(0, 0, dim)], sum[0][0]);

    // right-down [dim-1, dim-1]
    accumulate_sum(&sum[dim - 1][dim - 1], src[RIDX(dim - 2, dim - 2, dim)]);
    accumulate_sum(&sum[dim - 1][dim - 1], src[RIDX(dim - 1, dim - 2, dim)]);
    accumulate_sum(&sum[dim - 1][dim - 1], src[RIDX(dim - 2, dim - 1, dim)]);
    accumulate_sum(&sum[dim - 1][dim - 1], src[RIDX(dim - 1, dim - 1, dim)]);
    assign_sum_to_pixel(&dst[RIDX(dim - 1, dim - 1, dim)], sum[dim - 1][dim - 1]);

    // [0, 1]
    sum[0][1] = sum[0][0];
    accumulate_sum(&sum[0][1], src[RIDX(0, 2, dim)]);
    accumulate_sum(&sum[0][1], src[RIDX(1, 2, dim)]);
    assign_sum_to_pixel(&dst[RIDX(0, 1, dim)], sum[0][1]);

    // [1, 0]
    sum[1][0] = sum[0][0];
    accumulate_sum(&sum[1][0], src[RIDX(2, 0, dim)]);
    accumulate_sum(&sum[1][0], src[RIDX(2, 1, dim)]);
    assign_sum_to_pixel(&dst[RIDX(1, 0, dim)], sum[1][0]);

    // top [0, 2] ==> [0, dim-2]
    for (i = 2; i < dim - 1; i++) {
        sum[0][i] = sum[0][i - 1];
        accumulate_sum(&sum[0][i], src[RIDX(0, i + 1, dim)]);
        accumulate_sum(&sum[0][i], src[RIDX(1, i + 1, dim)]);
        subtract_sum(&sum[0][i], src[RIDX(0, i - 2, dim)]);
        subtract_sum(&sum[0][i], src[RIDX(1, i - 2, dim)]);
        assign_sum_to_pixel(&dst[RIDX(0, i, dim)], sum[0][i]);
    }

    // top-right [0, dim-1]
    sum[0][dim - 1] = sum[0][dim - 2];
    subtract_sum(&sum[0][dim - 1], src[RIDX(0, dim - 3, dim)]);
    subtract_sum(&sum[0][dim - 1], src[RIDX(1, dim - 3, dim)]);
    assign_sum_to_pixel(&dst[RIDX(0, dim - 1, dim)], sum[0][dim - 1]);

    // left [2, 0] ==> [dim-2, 0]
    for (i = 2; i < dim - 1; i++) {
        sum[i][0] = sum[i - 1][0];
        accumulate_sum(&sum[i][0], src[RIDX(i + 1, 0, dim)]);
        accumulate_sum(&sum[i][0], src[RIDX(i + 1, 1, dim)]);
        subtract_sum(&sum[i][0], src[RIDX(i - 2, 0, dim)]);
        subtract_sum(&sum[i][0], src[RIDX(i - 2, 1, dim)]);
        assign_sum_to_pixel(&dst[RIDX(i, 0, dim)], sum[i][0]);
    }

    // left-down [dim-1, 0]
    sum[dim - 1][0] = sum[dim - 2][0];
    subtract_sum(&sum[dim - 1][0], src[RIDX(dim - 3, 0, dim)]);
    subtract_sum(&sum[dim - 1][0], src[RIDX(dim - 3, 1, dim)]);
    assign_sum_to_pixel(&dst[RIDX(dim - 1, 0, dim)], sum[dim - 1][0]);

    // middle [1, 1] ==> [dim-2, dim-2]
    for (i = 1; i < dim - 1; i++) {
        for (j = 1; j < dim - 1; j++) {
            sum[i][j] = sum[i - 1][j];
            accumulate_sum(&sum[i][j], src[RIDX(i + 1, j - 1, dim)]);
            accumulate_sum(&sum[i][j], src[RIDX(i + 1, j, dim)]);
            accumulate_sum(&sum[i][j], src[RIDX(i + 1, j + 1, dim)]);
            subtract_sum(&sum[i][j], src[RIDX(i - 2, j - 1, dim)]);
            subtract_sum(&sum[i][j], src[RIDX(i - 2, j, dim)]);
            subtract_sum(&sum[i][j], src[RIDX(i - 2, j + 1, dim)]);
            assign_sum_to_pixel(&dst[RIDX(i, j, dim)], sum[i][j]);
        }
    }

    // [dim-1, 1]
    sum[dim - 1][1] = sum[dim][0];
    accumulate_sum(&sum[dim - 1][1], src[RIDX(dim - 2, 2, dim)]);
    accumulate_sum(&sum[dim - 1][1], src[RIDX(dim - 1, 2, dim)]);

    // down [dim-1, 2] ==> [dim-1, dim-2]
    for (i = 2; i < dim - 1; i++) {
        sum[dim - 1][i] = sum[dim - 1][i - 1];
        subtract_sum(&sum[dim - 1][i], src[RIDX(dim - 3, i - 1, dim)]);
        subtract_sum(&sum[dim - 1][i], src[RIDX(dim - 3, i, dim)]);
        subtract_sum(&sum[dim - 1][i], src[RIDX(dim - 3, i + 1, dim)]);
        assign_sum_to_pixel(&dst[RIDX(dim - 1, i, dim)], sum[dim - 1][i]);
    }

    // [dim-1, 1]
    sum[1][dim - 1] = sum[0][dim - 1];
    accumulate_sum(&sum[1][dim - 1], src[RIDX(2, dim - 2, dim)]);
    accumulate_sum(&sum[1][dim - 1], src[RIDX(2, dim - 1, dim)]);

    // right [2, dim-1] ==> [dim-2, dim-1]
    for (i = 2; i < dim - 1; i++) {
        sum[i][dim - 1] = sum[i][dim - 2];
        subtract_sum(&sum[i][dim - 1], src[RIDX(i - 1, dim - 3, dim)]);
        subtract_sum(&sum[i][dim - 1], src[RIDX(i, dim - 3, dim)]);
        subtract_sum(&sum[i][dim - 1], src[RIDX(i + 1, dim - 3, dim)]);
        assign_sum_to_pixel(&dst[RIDX(i, dim - 1, dim)], sum[i][dim - 1]);
    }


}

char smooth_descr[] = "smooth: Current working version";
void smooth(int dim, pixel *src, pixel *dst) {
    naive_smooth(dim, src, dst);
}


/********************************************************************* 
 * register_smooth_functions - Register all of your different versions
 *     of the smooth kernel with the driver by calling the
 *     add_smooth_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_smooth_functions() {
    add_smooth_function(&naive_smooth, naive_smooth_descr);
    add_smooth_function(&dynamic_programming_smooth, dynamic_programming_smooth_descr);
    add_smooth_function(&brute_force_smooth, brute_force_smooth_descr);
    /* ... Register additional test functions here */
}

