#include "cfss.h"

size_t count;
struct timeval t1, t2;
value min;
stack sol;
bool stop;

// F+

__attribute__((always_inline))
inline value cvaluep(stack *st, agent i) {

	register value a = st->m[i] * TS;
	register value b = st->t[i] - a;
	return b * DAYAHEAD + a * FORWARD;
}

// F-

__attribute__((always_inline))
inline value cvaluem(stack *st, agent i) {

	return KAPPA(X(st->s, i));
}

// COALITION STRUCTURE VALUE FUNCTIONS

__attribute__((always_inline))
inline value csvalue(stack *st) {

	register const agent *p = st->n + N + 1;
	register agent i, m = st->n[N];
	register value tot = 0;

	do {
		i = *(p++);
		tot += cvaluep(st, i) + cvaluem(st, i);
	} while (--m);

	return tot;
}

__attribute__((always_inline))
inline value csvaluep(stack *st) {

	register const agent *p = st->n + N + 1;
	register agent i, m = st->n[N];
	register value tot = 0;

	do {
		i = *(p++);
		tot += cvaluep(st, i);
	} while (--m);

	return tot;
}

__attribute__((always_inline))
inline value csvaluem(stack *st) {

	register const agent *p = st->n + N + 1;
	register agent i, m = st->n[N];
	register value tot = 0;

	do {
		i = *(p++);
		tot += cvaluem(st, i);
	} while (--m);

	return tot;
}

// CONTRACT EDGE

__attribute__((always_inline)) inline
void contract(stack *st, agent v1, agent v2) {

	register agent i, m = st->n[N];
	register const agent *p = st->n + N + 1;
	register edge e, f;

	do if ((i = *(p++)) != v1)
		if ((e = st->g[i * N + v2])) {
			if ((f = st->g[i * N + v1])) {
				if (!GET(st->c, f)) CLEAR(st->c, e);
				CLEAR(st->c, f);
			}
			st->g[i * N + v1] = st->g[v1 * N + i] = e;
			X(st->a, e) = v1;
			Y(st->a, e) = i;
		}
	while (--m);
}

// MERGE COALITIONS

__attribute__((always_inline)) inline
void merge(stack *st, agent v1, agent v2) {

	register agent a, b, i, j, min = v1, max = v2, *p = st->n + N + 1;

	if (Y(st->s, max) < Y(st->s, min)) {
		b = max;
		max = min;
		min = b;
	}

	a = X(st->s, min);
	b = X(st->s, max);
	max = Y(st->s, max);
	Y(st->s, v1) = min = Y(st->s, min);
	X(st->s, v1) = a + b;
	register agent *c = (agent *)malloc(sizeof(agent) * b);
	memcpy(c, st->cs + max, sizeof(agent) * b);
	memmove(st->cs + min + a + b, st->cs + min + a, sizeof(agent) * (max - min - a));
	memmove(st->cs + min, st->cs + min, sizeof(agent) * a);
	memcpy(st->cs + min + a, c, sizeof(agent) * b);
	free(c);

	if ((j = st->n[st->n[N] + N]) != v2) {
		st->n[j] = st->n[v2];
		st->n[st->n[v2]] = j;
		st->n[v2] = st->n[N] + N;
	}

	j = --(st->n[N]);

	do if ((i = *(p++)) != v1) {
		a = Y(st->s, i);
		if (a > min && a < max) Y(st->s, i) = a + b;
	} while (--j);
}

// MERGE PROFILES

__attribute__((always_inline))
inline void mergeprof(stack *st, agent v1, agent v2) {

	register __m128 a, b = _mm_set1_ps(FLT_MAX);
	st->t[v1] += st->t[v2];

	for (unsigned i = 0; i < TS; i += 4) {
		a = _mm_add_ps(_mm_load_ps(st->p + v1 * TS + i), _mm_load_ps(st->p + v2 * TS + i));
		b = _mm_min_ps(a, b);
		_mm_store_ps(st->p + v1 * TS + i, a);
	}

	b = _mm_min_ps(b, _mm_shuffle_ps(b, b, _MM_SHUFFLE(2, 1, 0, 3)));
	st->m[v1] = _mm_min_ps(b, _mm_shuffle_ps(b, b, _MM_SHUFFLE(1, 0, 3, 2)))[0];
}

// PRINT COALITION STRUCTURE

void printcs(stack *st) {

	register const agent *p = st->n + N + 1;
        register agent i, m = st->n[N];

	do {
		i = *(p++);
                printf("{ ");
                for (agent j = 0; j < X(st->s, i); j++)
                	printf("%s%u%s ", i == st->cs[Y(st->s, i) + j] ? "<" : "", 
			       1 + st->cs[Y(st->s, i) + j], i == st->cs[Y(st->s, i) + j] ? ">" : "");
                printf("} = %f\n", cvaluep(st, i) + cvaluem(st, i));
        } while (--m);

	puts("");
}

// CONTRACT ALL AVAILABLE EDGES

__attribute__((always_inline)) inline
void connect(stack *st) {

	register agent m = st->n[N];
	register const agent *p = st->n + N + 1;
	register agent *q = (agent *)malloc(sizeof(agent) * N);
	register agent *l = (agent *)malloc(sizeof(agent) * N * N);
	register agent *h = (agent *)calloc(N, sizeof(agent));
	register edge popc = MASKPOPCNT(st->c, C);

	for (edge i = 0, e = MASKFFS(st->c, C); i < popc; i++, e = MASKCLEARANDFFS(st->c, e, C)) {
		agent v1 = X(st->a, e);
		agent v2 = l[v1 * N + h[v1]++] = Y(st->a, e);
		l[v2 * N + h[v2]++] = v1;
	}

	do {
		edge e = 1, f = 0;
		agent i = *(p++);
		q[f] = i;

		do {
			for (agent j = 0; j < h[q[f]]; j++) {
				agent b = l[q[f] * N + j];
				if (i != b && CONTAINS(st->n, b)) {
					//
					merge(st, i, b);
					mergeprof(st, i, b);
					//
					q[e++] = b;
					m--;
				}
			}
			f++;
		}
		while (f != e);
	} while (--m);

	free(q);
	free(l);
	free(h);
}

__attribute__((always_inline)) inline
value bound(stack *st) {

	register stack *cst = (stack *)malloc(sizeof(stack));
	*cst = *st;
	connect(cst);
	//printcs(cst);
	register value vcst = csvaluep(cst);
	free(cst);
	return vcst + csvaluem(st);
}

void cfss(stack *st) {

	count++;
	//printcs(st);
	register value cur = csvalue(st);
	if (cur < min) { min = cur; sol = *st; }

	#ifdef LIMIT
	if (!stop) {
		gettimeofday(&t2, NULL);
		if ((double)(t2.tv_usec - t1.tv_usec) / 1e6 + t2.tv_sec - t1.tv_sec > LIMIT) stop = true;
	}
	#endif

	if (stop || bound(st) > min) return;

	chunk tmp[C];
	memcpy(tmp, st->c, sizeof(chunk) * C);
	register edge popc = MASKPOPCNT(tmp, C);

	for (edge i = 0, e = MASKFFS(tmp, C); !stop && i < popc; i++, e = MASKCLEARANDFFS(tmp, e, C)) {

		register agent v1 = X(st->a, e);
		register agent v2 = Y(st->a, e);
		CLEAR(st->c, st->g[v1 * N + v2]);
		st[1] = st[0];
		merge(st + 1, v1, v2);
		mergeprof(st + 1, v1, v2);
		contract(st + 1, v1, v2);
		cfss(st + 1);
	}
}

inline void createedge(edge *g, agent *a, agent v1, agent v2, edge e) {

	g[v1 * N + v2] = g[v2 * N + v1] = e;
	X(a, e) = v1;
	Y(a, e) = v2;
}

void scalefree(edge *g, agent *a) {

	unsigned deg[N] = {0};
	register uint_fast64_t d, i, j, h, k = 1, q, t = 0;
	register int p;

	for (i = 1; i <= K; i++) {
		for (j = 0; j < i; j++) {
			createedge(g, a, i, j, k++);
			deg[i]++;
			deg[j]++;
		}
	}

	for (i = K + 1; i < N; i++) {
		t &= ~((1ULL << i) - 1);
		for (j = 0; j < K; j++) {
			d = 0;
			for (h = 0; h < i; h++)
				if (!((t >> h) & 1)) d += deg[h];
			if (d > 0) {
				p = nextInt(d);
				q = 0;
				while (p >= 0) {
					if (!((t >> q) & 1)) p = p - deg[q];
					q++;
				}
				q--;
				t |= 1ULL << q;
				createedge(g, a, i, q, k++);
				deg[i]++;
				deg[q]++;
			}
		}
	}
}

int main(int argc, char *argv[]) {

	stack *st = (stack *)malloc(sizeof(stack) * N);
	if (!st) { puts("Error allocating stack"); exit(1); }
	memset(st->g, 0, sizeof(edge) * N * N);
	memset(st->c, 0, sizeof(chunk) * C);
	st->n[N] = N;

	for (agent i = 0; i < N; i++) {
		X(st->s, i) = 1;
		Y(st->s, i) = st->cs[i] = i;
		st->n[st->n[i] = N + i + 1] = i;
	}

	ONES(st->c, E + 1, C);
	CLEAR(st->c, 0);

	init(SEED);
	scalefree(st->g, st->a);
	init(SEED);
	read(st->p, st->t, st->m);

	value in = min = csvalue(st);
	sol = *st;
	#ifdef LIMIT
	value bou = bound(st);
	#endif

	gettimeofday(&t1, NULL);
	cfss(st);
	gettimeofday(&t2, NULL);
	free(st);

	//printcs(&sol);
	#ifdef LIMIT
	printf("%u,%u,%u,%f,%f,%f,%f,%f,%f,%zu\n", N, E, SEED, in, min, bou, (in - min) / in, min / bou,
	       (double)(t2.tv_usec - t1.tv_usec) / 1e6 + t2.tv_sec - t1.tv_sec, count);
	#else
	printf("%u,%u,%u,%f,%f,%f,%f,%zu\n", N, E, SEED, in, min, (in - min) / in,
	       (double)(t2.tv_usec - t1.tv_usec) / 1e6 + t2.tv_sec - t1.tv_sec, count);
	#endif

	return 0;
}
