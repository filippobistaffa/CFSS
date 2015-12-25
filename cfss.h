#ifndef CFSS_H_
#define CFSS_H_

#include <math.h>
#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <immintrin.h>
#include "uthash/src/uthash.h"

#ifndef TWITTER
#define N 100
#define M 2
#define SEED 1
#define E (M * N - (M * (M + 1)) / 2)
#endif

#define MAXV (10)
#define MINV (-10)
#define K N

#define CONTAINS(V, I) ((V)[I] <= (V)[N] + N)
#define C CEILBPC(MAX(N, E))

#include "types.h"
#include "macros.h"
#include "random.h"
#include "iqsort.h"
#include "sorted.h"
#include "dslyce.h"

typedef struct __attribute__((aligned(128))) {
	agent a[2 * (E + 1)], n[2 * N + 1];
	agent s[2 * N], cs[N];
	edge g[N * N];
	chunk c[C];
	var *hash;
} stack;

#endif /* CFSS_H_ */
