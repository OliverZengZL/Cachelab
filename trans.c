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
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void trans_block_32(int M, int N, int A[N][M], int B[M][N]);
void trans_block_64(int M, int N, int A[N][M], int B[M][N]);
void trans_block_rectangular(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    switch (M)
    {
    case 32:
        trans_block_32(M, N, A, B);
        break;
    case 64:
        trans_block_64(M, N, A, B);
        break;
    case 61:
        trans_block_rectangular(M, N, A, B);
    default:
        break;
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

char trans_block_32_desc[] = "Blocked transpose of size 32 * 32";
void trans_block_32(int M, int N, int A[N][M], int B[M][N])
{
    int bsize = 8;
    int diagonal, diaind, diaval = 0;
    for (int ii = 0; ii < N; ii += bsize)
    {
        for (int jj = 0; jj < M; jj += bsize)
        {
            for (int i = ii; i < ii + bsize; ++i)
            {
                for (int j = jj; j < jj + bsize; ++j)
                {
                    if (i != j)
                    {
                        B[j][i] = A[i][j];
                        continue;
                    }

                    //diagonal
                    diagonal = 1;
                    diaval = A[i][j];
                    diaind = i;
                }
                if (diagonal)
                {
                    B[diaind][diaind] = diaval;
                    diagonal = 0;
                }
            }
        }
    }
}

char trans_block_64_desc[] = "Blocked transpose of size 64 * 64";
void trans_block_64(int M, int N, int A[N][M], int B[M][N])
{
    int bsize = 8;
    int ii, jj, i, j;
    int diagonal, diaind, diaval = 0;
    int ent0, ent1, ent2, ent3, ent4, ent5, ent6, ent7;
    for (ii = 0; ii < M; ii += bsize)
    {
        for (jj = 0; jj < N; jj += bsize)
        {
            for (i = ii; i < ii + bsize / 2; ++i) //left top corner
            {
                for (j = jj; j < jj + bsize / 2; ++j)
                {
                    if (i != j)
                    {
                        B[j][i] = A[i][j];
                        continue;
                    }

                    //diagonal
                    diagonal = 1;
                    diaval = A[i][j];
                    diaind = i;
                }
                if (diagonal)
                {
                    B[diaind][diaind] = diaval;
                    diagonal = 0;
                }
            }

            for (i = ii; i < ii + bsize / 2; ++i) // A right top corner -> B top right corner 
            {
                ent4 = A[i][jj + bsize / 2];
                ent5 = A[i][jj + bsize / 2 + 1];
                ent6 = A[i][jj + bsize / 2 + 2];
                ent7 = A[i][jj + bsize / 2 + 3];

                B[jj][i + bsize / 2] = ent4;
                B[jj + 1][i + bsize / 2] = ent5;
                B[jj + 2][i + bsize / 2] = ent6;
                B[jj + 3][i + bsize / 2] = ent7;
            }

            for (j = jj; j < jj + bsize / 2; ++j) // left bottom corner
            {
                // B right top row
                ent0 = B[j][ii + bsize / 2];
                ent1 = B[j][ii + bsize / 2 + 1];
                ent2 = B[j][ii + bsize / 2 + 2];
                ent3 = B[j][ii + bsize / 2 + 3];

                // A left bottom col
                ent4 = A[ii + bsize/2][j];
                ent5 = A[ii + bsize/2 + 1][j];
                ent6 = A[ii + bsize/2 + 2][j];
                ent7 = A[ii + bsize/2 + 3][j];

                //A left bottom col -> B right top row
                B[j][ii + bsize / 2] = ent4;
                B[j][ii + bsize / 2 + 1] = ent5;
                B[j][ii + bsize / 2 + 2] = ent6;
                B[j][ii + bsize / 2 + 3] = ent7;

                //B right top row -> B left bottom row
                B[j + bsize/2][ii] = ent0;
                B[j + bsize/2][ii + 1] = ent1;
                B[j + bsize/2][ii + 2] = ent2;
                B[j + bsize/2][ii + 3] = ent3;
            }


            for (i = ii + bsize / 2; i < ii + bsize; ++i) //right bottom corner
            {
                for (int j = jj + bsize / 2; j < jj + bsize; ++j)
                {
                    if (i != j)
                    {
                        B[j][i] = A[i][j];
                        continue;
                    }

                    //diagonal
                    diagonal = 1;
                    diaval = A[i][j];
                    diaind = i;
                }
                if (diagonal)
                {
                    B[diaind][diaind] = diaval;
                    diagonal = 0;
                }
            }
        }
    }
}

char trans_block_rectangular_desc[] = "Blocked transpose of rectangular";
void trans_block_rectangular(int M, int N, int A[N][M], int B[M][N])
{
    int ir_bsize = 17;
    int diagonal, di, dj, diaval = 0;
    for (int ii = 0; ii < N; ii += ir_bsize)
    {
        for (int jj = 0; jj < M; jj += ir_bsize)
        {
            for (int i = ii; i < ((ii + ir_bsize > N) ? N : (ii + ir_bsize)); ++i)
            {
                for (int j = jj; j < ((jj + ir_bsize > M) ? M : (jj + ir_bsize)); ++j)
                {
                    if (i != j)
                    {
                        B[j][i] = A[i][j];
                        continue;
                    }

                    //diagonal
                    diagonal = 1;
                    diaval = A[i][j];
                    di = i;
                    dj = j;
                }
                if (diagonal)
                {
                    B[di][dj] = diaval;
                    diagonal = 0;
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
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    // registerTransFunction(trans, trans_desc);
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}
