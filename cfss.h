#ifndef CFSS_H_
#define CFSS_H_

#include <math.h>
#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <immintrin.h>

#ifndef TWITTER
#define N 20
#define K 2
#define SEED 1
#define E (K * N - (K * (K + 1)) / 2)
#include "types.h"
#endif

#define CONTAINS(V, I) ((V)[I] <= (V)[N] + N)
#define C CEILBPC(MAX(N, E))

#include "iqsort.h"
#include "random.h"
#include "macros.h"

typedef struct {
	agent a[2 * (E + 1)], n[2 * N + 1];
	agent s[2 * N], cs[N];
	edge g[N * N];
	chunk c[C];
} stack;

#endif /* CFSS_H_ */
