#include "cfss.h"

size_t count;
struct timeval t1, t2;
value max;
stack sol;
bool stop;

typedef struct { id a; id i; } aid;

__attribute__((always_inline)) inline
unsigned binarysearch(id x, const aid *buf, unsigned n) {

	if (n) {
		#define MIDPOINT(_min, _max) (_min + ((_max - _min) / 2))
		register unsigned imin = 0, imid, imax = n - 1;

		while (imin < imax) {
			imid = MIDPOINT(imin, imax);
			if (buf[imid].a < x) imin = imid + 1;
			else imax = imid;
		}

		if (imax == imin && buf[imin].a == x) return imin;
	}
	return n + 1;
}

#define EDGEID(ADJ, IDXADJ, V1, V2) Y(ADJ, binarysearch(MAX(V1, V2), (aid *)(ADJ + 2 * Y(IDXADJ, MIN(V1, V2))), X(IDXADJ, MIN(V1, V2))) + Y(IDXADJ, MIN(V1, V2)))

// Contract edge between v1 and v2

__attribute__((always_inline)) inline
void contract(stack *st, id v1, id v2) {

	register id i, m = st->n[N];
	register const id *p = st->n + N + 1;
	register id e, f;

	do if ((i = *(p++)) != v1)
		//if ((e = st->g[i * N + v2])) {
		if ((e = EDGEID(st->adj, st->idxadj, i, v2))) {
			//if ((f = st->g[i * N + v1])) {
			if ((f = EDGEID(st->adj, st->idxadj, i, v1))) {
				if (!GET(st->c, f)) CLEAR(st->c, e);
				st->v[e] += st->v[f];
				CLEAR(st->c, f);
			}
			//st->g[i * N + v1] = st->g[v1 * N + i] = e;
			EDGEID(st->adj, st->idxadj, i, v1) = e;
			X(st->a, e) = v1;
			Y(st->a, e) = i;
		}
	while (--m);
}

// Merge coalitions of v1 and v2

__attribute__((always_inline)) inline
void merge(stack *st, id v1, id v2) {

	register id j;

	if ((j = st->n[st->n[N] + N]) != v2) {
		st->n[j] = st->n[v2];
		st->n[st->n[v2]] = j;
		st->n[v2] = st->n[N] + N;
	}

	(st->n[N])--;
}

__attribute__((always_inline)) inline
value bound(const stack *st) {

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
	if (st->val > max) { max = st->val; sol = *st; }

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
		//CLEAR(st->c, st->g[v1 * N + v2]);
		CLEAR(st->c, EDGEID(st->adj, st->idxadj, v1, v2));
		st[1] = st[0];
		merge(st + 1, v1, v2);
		contract(st + 1, v1, v2);
		st[1].val += st[1].v[e];
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

template <typename type>
__attribute__((always_inline)) inline
void exclprefixsum(const type *hi, type *ho, unsigned hn) {

	if (hn) {
		ho[0] = 0;
		for (unsigned i = 1; i < hn; i++)
			ho[i] = hi[i - 1] + ho[i - 1];
	}
}

void createadj(stack *st) {

	id *c = (id *)calloc(N, sizeof(id));

	for (id i = 0; i < E; i++)
		c[MIN(X(st->a, i), Y(st->a, i))]++;

	for (id i = 0, pfx = 0; i < N; i++) {
		X(st->idxadj, i) = c[i];
		Y(st->idxadj, i) = pfx;
		pfx += c[i];
	}

	memset(c, 0, sizeof(id) * N);

	for (id i = 0; i < E; i++) {
		id min = MIN(X(st->a, i), Y(st->a, i));
		id max = MAX(X(st->a, i), Y(st->a, i));
		X(st->adj, c[min] + Y(st->idxadj, min)) = max;
		Y(st->adj, c[min] + Y(st->idxadj, min)) = i;
		c[min]++;
	}

	free(c);
	#define LTA(X, Y) ((*(X)).a < (*(Y)).a)

	for (id i = 0; i < N; i++)
		QSORT(aid, (aid *)(st->adj) + Y(st->idxadj, i), X(st->idxadj, i), LTA);

	for (id i = 0; i < N; i++) {
		printf("%u = [ ", i);
		for (id j = 0; j < X(st->idxadj, i); j++)
			printf("%u (%u) ", X(st->adj, j + Y(st->idxadj, i)), Y(st->adj, j + Y(st->idxadj, i)));
		printf("]\n");
	}
}

void reorderedges(id *a, value *v) {

	typedef struct { id v1; id v2; value v; } ev;
	ev evb[E];

	for (id i = 0; i < E; i++) {
		evb[i].v = v[i];
		evb[i].v1 = X(a, i);
		evb[i].v2 = Y(a, i);
	}

	#define GTV(X, Y) ((*(X)).v > (*(Y)).v)
	QSORT(ev, evb, E, GTV);

	#define GT(X, Y) ((*(X)) > (*(Y)))
        QSORT(value, v, E, GT);

	for (id i = 0; i < E; i++)
		createid(a, evb[i].v1, evb[i].v2, i);
}

int main(int argc, char *argv[]) {

	printf("Tot  = %zu\n", sizeof(stack) * N);
	printf("adj  = %zu\n", sizeof(id) * N * 2 * E);
	printf("iadj = %zu\n", sizeof(id) * N * 2 * N);
	printf("a    = %zu\n", sizeof(id) * N * 2 * E);
	printf("n    = %zu\n", sizeof(id) * N * (2 * N + 1));
	printf("E    = %u\n", E);
	printf("d    = %f\n", (double)E / (N * N));

	//exit(0);

	stack *st = (stack *)malloc(sizeof(stack) * N);
	if (!st) { puts("Error allocating stack"); exit(1); }
	memset(st->c, 0, sizeof(chunk) * C);
	st->n[N] = N;

	for (id i = 0; i < N; i++) {
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
	reorderedges(st->a, st->v);
	#endif

	for (id i = 0; i < E; i++)
		printf("%u: (%u, %u) = %f\n", i, X(st->a, i), Y(st->a, i), st->v[i]);

	createadj(st);
	st->val = max = 0;
	sol = *st;
	#ifdef LIMIT
	value bou = bound(st);
	#endif

	printf("%u\n", EDGEID(st->adj, st->idxadj, 9, 10));
	printf("%u\n", EDGEID(st->adj, st->idxadj, 9, 18));
	printf("%u\n", EDGEID(st->adj, st->idxadj, 0, 16));
	printf("%u\n", EDGEID(st->adj, st->idxadj, 0, 17));
	printf("%u\n", EDGEID(st->adj, st->idxadj, 0, 13));
	printf("%u\n", EDGEID(st->adj, st->idxadj, 1, 14));
	printf("%u\n", EDGEID(st->adj, st->idxadj, 2, 5));
	printf("%u\n", EDGEID(st->adj, st->idxadj, 3, 12));

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
