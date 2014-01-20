#ifndef LEVMARQ_H
#define LEVMARQ_H

#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include "Textures.h"

//--- USER DEFINITIONS ---

//#define MAXCOUNTDATA 2450 //for compute capability 2.0 or higher - currently ca. 2450 is max. because (COUNTPARAM + 2) * MAXCOUNTDATA * sizeof(float) = 48 kB (= max. shared memory)
#define MAXCOUNTDATA 800 //for compute capability 1.x - currently ca. 800 is max. because (COUNTPARAM + 2) * MAXCOUNTDATA * sizeof(float) = 16 kB (= max. shared memory)

#define DATATYPE float //if data texture is used, can not be changed to integer types
#define MAXCALL 100
#define COUNTPARAM 3
#define PARAMSTARTVALUE { 1, 1, 1 } //any value, but not { 0, 0, 0 } (count = COUNTPARAM)

#define FITVALUETHRESHOLD 0.0 //0.5 //threshold between min (0.0) and max (1.0) value to define the data using interval to calculate the fit function
#define STARTENDPROPORTION 0.01 //proportion of countData for calculating the average of start/end value (e. g. 0.1 means average of the first 10% of data for start value and the last 10% for end value)

//------------------------

#define CUDA //defined: runs on GPU, otherwise on CPU (useful for debugging)

#ifdef CUDA
//texture<DATATYPE, 2, cudaReadModeElementType> dataTexture; //-> GLOBAL DEFINITION
#define GETSAMPLE(I, INDEXDATASET) tex2D(dataTexture, (I) + 0.5, (INDEXDATASET) + 0.5)
#else
DATATYPE *data;
#define GETSAMPLE(I, INDEXDATASET) data[I] //INDEXDATASET has no effect (only for CUDA)
#endif

//--- GLOBAL DEFINITIONS ---

/*struct fitData {
	float param[COUNTPARAM];
	float startValue;
	float endValue;
	float extremumPos;
	float extremumValue;
	int status;
};*/
const char *statusMessage[] = { //indexed by fitData.status
/* 0 */	"fatal coding error (improper input parameters)",
/* 1 */	"success (the relative error in the sum of squares is at most tol)",
/* 2 */	"success (the relative error between x and the solution is at most tol)",
/* 3 */	"success (the relative errors in the sum of squares and between x and the solution are at most tol)",
/* 4 */	"trapped by degeneracy (fvec is orthogonal to the columns of the jacobian)",
/* 5 */	"timeout (number of calls to fcn has reached maxcall*(n+1))",
/* 6 */	"failure (ftol<tol: cannot reduce sum of squares any further)",
/* 7 */	"failure (xtol<tol: cannot improve approximate solution any further)",
/* 8 */	"failure (gtol<tol: cannot improve approximate solution any further)"
};

//------------------------

#ifdef CUDA
#define GLOBAL __global__
#define DEVICE __device__
#define SHARED __shared__
#else
#define GLOBAL
#define DEVICE
#define SHARED
#endif

//machine-dependent constants from float.h
#define LM_MACHEP     FLT_EPSILON   //resolution of arithmetic
#define LM_DWARF      FLT_MIN       //smallest nonzero number
#define LM_SQRT_DWARF sqrt(FLT_MIN) //square should not underflow
#define LM_SQRT_GIANT sqrt(FLT_MAX) //square should not overflow
#define LM_USERTOL    30*LM_MACHEP  //users are recommened to require this

#define MIN(A, B) (((A) <= (B)) ? (A) : (B))
#define MAX(A, B) (((A) >= (B)) ? (A) : (B))
#define SQR(X)    ((X) * (X))

DEVICE inline void fitFunction(float x, float *param, float *y);
DEVICE inline void fitFunctionExtremum(float *param, float *x);

DEVICE void evaluate(float *param, int countData, float *fvec, int indexDataset, int xOffset);
DEVICE void qrSolve(int n, float *r, int ldr, int *ipvt, float *diag,
			   float *qtb, float *x, float *sdiag, float *wa);
DEVICE void euclidNorm(int n, float *x, float* result);
DEVICE void lmpar(int n, float *r, int ldr, int *ipvt, float *diag,
			  float *qtb, float delta, float *par, float *x,
			  float *sdiag, float *wa1, float *wa2);
DEVICE void qrFactorization(int m, int n, float *a, int pivot, int *ipvt,
			  float *rdiag, float *acnorm, float *wa);
DEVICE void lmdif(int m, int n, float *x, float *fvec, float ftol,
			  float xtol, float gtol, int maxfev, float epsfcn,
			  float *diag, int mode, float factor, int *info, int *nfev,
			  float *fjac, int *ipvt, float *qtf, float *wa1,
			  float *wa2, float *wa3, float *wa4,
			  int indexDataset, int xOffset);
DEVICE void maxValue(int countData, int indexDataset, int *x, DATATYPE *y);
DEVICE void averageValue(int start, int count, int indexDataset, float *y);
DEVICE void xOfValue(int countData, int indexDataset, char fromDirection, DATATYPE minValue, int *x);
GLOBAL void kernel(int countData, struct fitData *result);

#endif
