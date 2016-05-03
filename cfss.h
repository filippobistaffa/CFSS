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

#define RANGE 10
//#define REORDER
#define LIMIT 100

#include "iqsort.h"
#include "random.h"
#include "macros.h"

typedef struct {
	id a[2 * E], n[2 * N + 1];
	id s[2 * N], cs[N];
	id *adj[N];
	id g[N * N];
	chunk c[C];
	value v[E], val;
} stack;

#endif /* CFSS_H_ */
