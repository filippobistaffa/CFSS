#ifndef CFSS_H_
#define CFSS_H_

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <immintrin.h>

#include "types.h"
#include "macros.h"

#define N 100
#define K 2
#define SEED 1
#define LIMIT 10
#define E (K * N - (K * (K + 1)) / 2)
#define C CEILBPC(MAX(N, E))

#define FORWARD 40
#define DAYAHEAD 80
#define GAMMA 1.8
#define KAPPA(S) (pow(S, GAMMA))

#define CONTAINS(V, I) ((V)[I] <= (V)[N] + N)

#include "random.h"
#include "read.h"

typedef struct __attribute__((aligned(128))) {
	value p[N * TS], t[N], m[N];
	agent a[2 * (E + 1)], n[2 * N + 1];
	agent s[2 * N], cs[N];
	edge g[N * N];
	chunk c[C];
} stack;

#endif /* CFSS_H_ */
