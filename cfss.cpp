#include "cfss.h"

size_t count;
struct timeval t1, t2;
value max;
stack sol;

// Contract edge between v1 and v2

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

// Merge coalitions of v1 and v2

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

// Value of the coalition

__attribute__((always_inline)) inline
value cvalue(const stack *st, agent i) {

	register var *t;
	agent tc[K + 1] = { 0 };
	tc[0] = X(st->s, i);
	memcpy(tc + 1, st->cs + Y(st->s, i), sizeof(agent) * (*tc));
	QSORT(agent, tc + 1, *tc, LT);
	HASH_FIND(hh, st->hash, tc, sizeof(agent) * (K + 1), t);
	return t->v;
}

// Total value of the coalition structure

__attribute__((always_inline))
inline value csvalue(stack *st) {

	register const agent *p = st->n + N + 1;
	register agent m = st->n[N];
	register value tot = 0;

	do {
		register agent i = *(p++);
		tot += cvalue(st, i);
	} while (--m);

	return tot;
}

// Print coalition structure

void printcs(stack *st) {

	register const agent *p = st->n + N + 1;
        register agent m = st->n[N];

	do {
		register agent i = *(p++);
                printf("{ ");
                for (agent j = 0; j < X(st->s, i); j++)
                	printf("%s%u%s ", i == st->cs[Y(st->s, i) + j] ? "<" : "", 
			       st->cs[Y(st->s, i) + j], i == st->cs[Y(st->s, i) + j] ? ">" : "");
                printf("} = %f\n", cvalue(st, i));
        } while (--m);

	puts("");
}

void cfss(stack *st) {

	count++;
	//printcs(st);
	register value cur = csvalue(st);
	if (cur > max) { max = cur; sol = *st; }

	chunk tmp[C];
	memcpy(tmp, st->c, sizeof(chunk) * C);
	register edge popc = MASKPOPCNT(tmp, C);

	for (edge i = 0, e = MASKFFS(tmp, C); i < popc; i++, e = MASKCLEARANDFFS(tmp, e, C)) {

		register agent v1 = X(st->a, e);
		register agent v2 = Y(st->a, e);
		CLEAR(st->c, st->g[v1 * N + v2]);
		st[1] = st[0];
		merge(st + 1, v1, v2);
		contract(st + 1, v1, v2);
		cfss(st + 1);
	}
}

inline void createedge(edge *g, agent *a, agent v1, agent v2, edge e) {

	g[v1 * N + v2] = g[v2 * N + v1] = e;
	X(a, e) = v1;
	Y(a, e) = v2;
}

#ifndef TWITTER
void scalefree(edge *g, agent *a) {

	unsigned deg[N] = {0};
	register uint_fast64_t d, i, j, h, k = 1, q, t = 0;
	register int p;

	for (i = 1; i <= M; i++) {
		for (j = 0; j < i; j++) {
			createedge(g, a, i, j, k++);
			deg[i]++;
			deg[j]++;
		}
	}

	for (i = M + 1; i < N; i++) {
		t &= ~((1ULL << i) - 1);
		for (j = 0; j < M; j++) {
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
#endif

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

	// Initialise graph

	#ifdef TWITTER
	memcpy(st->g, g, sizeof(edge) * N * N);
	memcpy(st->a, a, sizeof(agent) * 2 * (E + 1));
	#else
	init(SEED);
	scalefree(st->g, st->a);
	#endif

	// Put coalitional values in hash table

	st->hash = NULL;
	printf("%zu coalitions\n", enumerate(st->a, &(st->hash)));
	value in = max = csvalue(st);
	sol = *st;

	// Solve

	gettimeofday(&t1, NULL);
	cfss(st);
	gettimeofday(&t2, NULL);
	free(st);
	printcs(&sol);

	// Free hash table contents

	var *tmp, *p;
	HASH_ITER(hh, st->hash, p, tmp) {
		HASH_DELETE(hh, st->hash, p);
		free(p);
	}

	printf("%u,%u,%u,%f,%f,%f,%f,%zu\n", N, E, SEED, in, max, (in - max) / in,
	       (double)(t2.tv_usec - t1.tv_usec) / 1e6 + t2.tv_sec - t1.tv_sec, count);

	return 0;
}
