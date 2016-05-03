#include "cfss.h"

size_t count;
struct timeval t1, t2;
value max;
stack sol;
bool stop;

// F+

__attribute__((always_inline))
inline value cvaluep(stack *st, id i) {

	return 0;
}

// F-

__attribute__((always_inline))
inline value cvaluem(stack *st, id i) {

	return 0;
}

// Total value of the coalition structure

__attribute__((always_inline))
inline value csvalue(stack *st) {

	register const id *p = st->n + N + 1;
	register id i, m = st->n[N];
	register value tot = 0;

	do {
		i = *(p++);
		tot += cvaluep(st, i) + cvaluem(st, i);
	} while (--m);

	return tot;
}

__attribute__((always_inline))
inline value csvaluep(stack *st) {

	register const id *p = st->n + N + 1;
	register id i, m = st->n[N];
	register value tot = 0;

	do {
		i = *(p++);
		tot += cvaluep(st, i);
	} while (--m);

	return tot;
}

__attribute__((always_inline))
inline value csvaluem(stack *st) {

	register const id *p = st->n + N + 1;
	register id i, m = st->n[N];
	register value tot = 0;

	do {
		i = *(p++);
		tot += cvaluem(st, i);
	} while (--m);

	return tot;
}

// Contract id between v1 and v2

__attribute__((always_inline)) inline
void contract(stack *st, id v1, id v2) {

	register id i, m = st->n[N];
	register const id *p = st->n + N + 1;
	register id e, f;

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
void merge(stack *st, id v1, id v2) {

	register id a, b, i, j, min = v1, max = v2, *p = st->n + N + 1;

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
	register id *c = (id *)malloc(sizeof(id) * b);
	memcpy(c, st->cs + max, sizeof(id) * b);
	memmove(st->cs + min + a + b, st->cs + min + a, sizeof(id) * (max - min - a));
	memmove(st->cs + min, st->cs + min, sizeof(id) * a);
	memcpy(st->cs + min + a, c, sizeof(id) * b);
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

// Print coalition structure

void printcs(stack *st) {

	register const id *p = st->n + N + 1;
        register id i, m = st->n[N];

	do {
		i = *(p++);
                printf("{ ");
                for (id j = 0; j < X(st->s, i); j++)
                	printf("%s%u%s ", i == st->cs[Y(st->s, i) + j] ? "<" : "", 
			       1 + st->cs[Y(st->s, i) + j], i == st->cs[Y(st->s, i) + j] ? ">" : "");
                printf("} = %f\n", cvaluep(st, i) + cvaluem(st, i));
        } while (--m);

	puts("");
}

__attribute__((always_inline)) inline
value bound(stack *st) {

	register value maxgain = 0;
	chunk tmp[C];
	memcpy(tmp, st->c, sizeof(chunk) * C);
	register id popc = MASKPOPCNT(tmp, C);

	for (id i = 0, e = MASKFFS(tmp, C); !stop && i < popc; i++, e = MASKCLEARANDFFS(tmp, e, C))
		if (st->v[e] > 0) maxgain += st->v[e];

	return maxgain + st->val;
}

void cfss(stack *st) {

	count++;
	//printcs(st);
	register value cur = csvalue(st);
	if (cur > max) { max = cur; sol = *st; }

	#ifdef LIMIT
	if (!stop) {
		gettimeofday(&t2, NULL);
		if ((double)(t2.tv_usec - t1.tv_usec) / 1e6 + t2.tv_sec - t1.tv_sec > LIMIT) stop = true;
	}
	#endif

	if (stop || bound(st) <= max) return;

	chunk tmp[C];
	memcpy(tmp, st->c, sizeof(chunk) * C);
	register id popc = MASKPOPCNT(tmp, C);

	for (id i = 0, e = MASKFFS(tmp, C); !stop && i < popc; i++, e = MASKCLEARANDFFS(tmp, e, C)) {
		register id v1 = X(st->a, e);
		register id v2 = Y(st->a, e);
		CLEAR(st->c, st->g[v1 * N + v2]);
		st[1] = st[0];
		merge(st + 1, v1, v2);
		contract(st + 1, v1, v2);
		cfss(st + 1);
	}
}

inline void createid(id *a, id v1, id v2, id e) {

	X(a, e) = v1;
	Y(a, e) = v2;
}

#ifndef TWITTER
void scalefree(id *a) {

	unsigned deg[N] = {0};
	register uint_fast64_t d, i, j, h, k = 0, q, t = 0;
	register int p;

	for (i = 1; i <= K; i++) {
		for (j = 0; j < i; j++) {
			createid(a, i, j, k++);
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
				createid(a, i, q, k++);
				deg[i]++;
				deg[q]++;
			}
		}
	}
}
#endif

void createadj(id *a, id *adj[N]) {

	id *c = (id *)calloc(N, sizeof(id));

	for (id i = 0; i < E; i++) {
		c[X(a, i)]++;
		c[Y(a, i)]++;
	}

	for (id i = 0; i < N; i++) {
		adj[i] = (id *)malloc(sizeof(id) * (2 * c[i] + 1));
		*adj[i] = c[i];
	}

	memset(c, 0, sizeof(id) * N);

	for (id i = 0; i < E; i++) {
		//printf("i = %u, X(a, i) = %u, Y(a, i) = %u, c[X(a, i)] = %u, c[Y(a, i)] = %u\n", i, X(a, i), Y(a, i), c[X(a, i)], c[Y(a, i)]);
		X(adj[X(a, i)] + 1, c[X(a, i)]) = Y(a, i);
		Y(adj[X(a, i)] + 1, c[X(a, i)]) = i;
		X(adj[Y(a, i)] + 1, c[Y(a, i)]) = X(a, i);
		Y(adj[Y(a, i)] + 1, c[Y(a, i)]) = i;
		c[X(a, i)]++;
		c[Y(a, i)]++;
	}

	free(c);
	typedef struct { id a; id i; } aid;
	#define ltaid(X, Y) ((*(X)).a < (*(Y)).a)

	for (id i = 0; i < N; i++) {
		aid *buf = (aid *)(adj[i] + 1);
		QSORT(aid, buf, *adj[i], ltaid);
	}

	for (id i = 0; i < N; i++) {
		printf("%u = [ ", i);
		for (id j = 0; j < *adj[i]; j++)
			printf("%u (%u) ", X(adj[i] + 1, j), Y(adj[i] + 1, j));
		printf("]\n");
	}
}

void reorderids(id *a, value *v) {

	typedef struct { id v1; id v2; value v; } ev;
	ev evb[E];

	for (id i = 0; i < E; i++) {
		evb[i].v = v[i];
		evb[i].v1 = X(a, i);
		evb[i].v2 = Y(a, i);
	}

	#define gtv(a, b) ((*(a)).v > (*(b)).v)
	QSORT(ev, evb, E, gtv);

	#define gt(a, b) ((*(a)) > (*(b)))
        QSORT(value, v, E, gt);

	for (id i = 0; i < E; i++)
		createid(a, evb[i].v1, evb[i].v2, i);
}

int main(int argc, char *argv[]) {

	printf("Total = %zu\n", sizeof(stack) * N);
	printf("g  = %zu\n", sizeof(id) * N * N * N);
	printf("cs = %zu\n", sizeof(id) * N * N);
	printf("a  = %zu\n", sizeof(id) * N * 2 * E);
	printf("n  = %zu\n", sizeof(id) * N * (2 * N + 1));
	printf("s  = %zu\n", sizeof(id) * N * 2 * N);
	printf("d  = %f\n", (double)E / (N * N));

	stack *st = (stack *)malloc(sizeof(stack) * N);
	if (!st) { puts("Error allocating stack"); exit(1); }
	memset(st->c, 0, sizeof(chunk) * C);
	st->n[N] = N;

	for (id i = 0; i < N; i++) {
		X(st->s, i) = 1;
		Y(st->s, i) = st->cs[i] = i;
		st->n[st->n[i] = N + i + 1] = i;
	}

	ONES(st->c, E, C);

	// Initialise graph

	#ifdef TWITTER
	memcpy(st->a, a, sizeof(id) * 2 * E);
	#else
	init(SEED);
	scalefree(st->a);
	#endif

	// initialise id values

	for (id i = 0; i < E; i++)
		st->v[i] = nextInt(RANGE * 2) - RANGE;

	#ifdef REORDER
	reorderids(st->a, st->v);
	#endif

	for (id i = 0; i < E; i++)
		printf("%u: (%u, %u) = %f\n", i, X(st->a, i), Y(st->a, i), st->v[i]);

	createadj(st->a, st->adj);
	st->val = max = 0;
	sol = *st;
	#ifdef LIMIT
	value bou = bound(st);
	#endif

	//gettimeofday(&t1, NULL);
	//cfss(st);
	//gettimeofday(&t2, NULL);
	//free(st);

	//printcs(&sol);
	#ifdef LIMIT
	printf("%u,%u,%u,%f,%f,%f,%f,%zu\n", N, E, SEED, max, bou, max / bou, (double)(t2.tv_usec - t1.tv_usec) / 1e6 + t2.tv_sec - t1.tv_sec, count);
	#else
	printf("%u,%u,%u,%f,%f,%zu\n", N, E, SEED, max, (double)(t2.tv_usec - t1.tv_usec) / 1e6 + t2.tv_sec - t1.tv_sec, count);
	#endif

	return 0;
}
