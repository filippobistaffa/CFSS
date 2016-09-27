#ifndef CFSS_H_
#define CFSS_H_

#include <math.h>
#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "instance.h"
#include "params.h"
#include "types.h"

#ifdef M
#define RANGE 10
#define E (M * N - (M * (M + 1)) / 2)
#define RANDOMVALUE (nextInt(RANGE * 2) - RANGE)
#endif

#define CONTAINS(V, I) ((V)[I] <= (V)[N] + N)
#define C CEILBPC(MAX(N, E))

//#define GAMMA 1.8
//#define KAPPA(S) (pow(S, GAMMA))
#include "k.i"
#define KAPPA(S) 0
//#define KAPPA(S) (lookup[S])

#include "iqsort.h"
#include "random.h"
#include "macros.h"

typedef struct {
	idc a[E], idxadj[N], adj[E];
	value v[E], sing[N], val;
	id n[2 * N + 1], s[N];
	chunk c[C];
} stack;

#endif /* CFSS_H_ */
