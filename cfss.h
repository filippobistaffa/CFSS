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

#if !defined(LEADERS) && defined(MAXLEADERS)
#error "MAXLEADERS defined, but LEADERS not enabled"
#endif

#ifndef LEADERS
#define MAXLEADERS N
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
	id n[2 * N + 1], s[N], l[N];
	value v[E], sing[N], val;
	chunk c[C], r[C], e[C];
} stack;

#endif /* CFSS_H_ */
