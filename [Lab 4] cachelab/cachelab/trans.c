// 20150756 Choi Young-Il 

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
void transpose_32x32(int M, int N, int A[N][M], int B[M][N]);
void transpose_64x64(int M, int N, int A[N][M], int B[M][N]);
void transpose_61x67(int M, int N, int A[N][M], int B[M][N]);
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
    if(N==32)
	transpose_32x32(M, N, A, B);
    else if(N==64)
	transpose_64x64(M, N, A, B);
    else
	transpose_61x67(M, N, A, B);
}

void transpose_32x32(int M, int N, int A[N][M], int B[M][N]){
    int i,j,k1,k2;   
    for(i=0; i<N; i+=8){
	for(j=0; j<M; j+=8){
	    for(k1=0; k1<8; k1++){
		for(k2=0; k2<8; k2++){
		    if(i+k1 != j+k2) B[j+k2][i+k1]=A[i+k1][j+k2];
		}
		if(i==j) B[j+k1][i+k1]=A[i+k1][j+k1];
	    }
	}
    }
}

void transpose_64x64(int M, int N, int A[N][M], int B[M][N]){
    int i,j,k1,k2,a,b,c,d;
    int a2,b2,c2,d2;
    for(i=0; i<N; i+=8){
	for(j=0; j<M; j+=8){
	    for(k1=0; k1<8; k1++){
		for(k2=0; k2<4; k2++){
		    if(i+k2!=j+k1%4) B[j+k2][i+k1]=A[i+k1][j+k2];
		}
		if(k1==0){
		    a=A[i][j+4];
		    b=A[i][j+5];
		    c=A[i][j+6];
		    d=A[i][j+7];
		}
		if(k1==1){
		    a2=A[i+1][j+4];
		    b2=A[i+1][j+5];
		    c2=A[i+1][j+6];
		    d2=A[i+1][j+7];
		}
		if(i==j) B[j+k1%4][i+k1]=A[i+k1][j+k1%4];
	    }
	    for(k1=7; k1>1; k1--){
		for(k2=4; k2<8; k2++){
		    if(i+k1%4!=j+k2%4) B[j+k2][i+k1]=A[i+k1][j+k2];
		}
		if(i==j) B[j+k1%4+4][i+k1]=A[i+k1][j+k1%4+4];
	    }
	    B[j+4][i] = a;
	    B[j+5][i] = b;
	    B[j+6][i] = c;
	    B[j+7][i] = d;
	    B[j+4][i+1] = a2;
	    B[j+5][i+1] = b2;
	    B[j+6][i+1] = c2;
	    B[j+7][i+1] = d2;
	}
    }
}
void transpose_61x67(int M, int N, int A[N][M], int B[M][N]){
    int i,j,k1,k2;
    for(i=0; i<N; i+=16){
	for(j=0; j<M; j+=4){
	    for(k1=0; k1<16; k1++){
		if(i+k1>=N) break;
		for(k2=0; k2<4; k2++){
		    if(j+k2>=M) break;
		    if(i+k1!=j+k2) B[j+k2][i+k1] = A[i+k1][j+k2];
		}
		if(i+k1<M && i+k1-j<4 && i+k1-j>=0) B[i+k1][i+k1] = A[i+k1][i+k1];
	    }
	}
    }

}
/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
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
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
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

