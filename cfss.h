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
#define N 2000
#define K 2
#define E (K * N - (K * (K + 1)) / 2)
#include "types.h"
#endif

#define CONTAINS(V, I) ((V)[I] <= (V)[N] + N)
#define C CEILBPC(MAX(N, E))

#define RANGE 10
#define REORDER
#define LIMIT 100
#define BOUNDLEVEL 1
//#define GAMMA 1.8
//#define KAPPA(S) (pow(S, GAMMA))
#include "k.i"
#define KAPPA(S) (lookup[S])

#include "iqsort.h"
#include "random.h"
#include "macros.h"

typedef struct {
	idc a[E], idxadj[N], adj[E];
	id n[2 * N + 1], s[N];
	value v[E], val;
	chunk c[C];
} stack;

#endif /* CFSS_H_ */
