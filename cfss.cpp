#include "cfss.h"

size_t count;
struct timeval t1, t2;
value max, bou;
stack sol;
bool stop;

__attribute__((always_inline)) inline
unsigned binarysearch(id x, const idc *buf, unsigned n) {

	if (n) {
		#define MIDPOINT(_min, _max) (_min + ((_max - _min) / 2))
		register unsigned imin = 0, imid, imax = n - 1;

		while (imin < imax) {
			imid = MIDPOINT(imin, imax);
			if (X(buf, imid) < x) imin = imid + 1;
			else imax = imid;
		}

		if (imax == imin && X(buf, imin) == x) return imin;
	}

	return n + 1;
}

__attribute__((always_inline)) inline
unsigned insertionordered(id x, const idc *buf, unsigned n) {

	if (n) {
		register unsigned imin = 0, imid, imax = n - 1;

		if (x < X(buf, imin)) return imin;
		if (X(buf, imax) < x) return n;

		while (imax - imin != 1) {
			imid = MIDPOINT(imin, imax);
			if (X(buf, imid) < x) imin = imid;
			else imax = imid;
		}

		return imax;
	}

	return 0;
}

#define BINARYSEARCH(_ADJ, _IDXADJ, _MIN, _MAX) binarysearch(_MAX, _ADJ + Y(_IDXADJ, _MIN), X(_IDXADJ, _MIN))
#define INSERTIONORDERED(_ADJ, _IDXADJ, _MIN, _MAX) insertionordered(_MAX, _ADJ + Y(_IDXADJ, _MIN), X(_IDXADJ, _MIN))

__attribute__((always_inline)) inline
int getedge(const idc *adj, const idc *idxadj, id v1, id v2) {

	//printf("looking for edge (%u, %u)\n", v1, v2);
	const id min = MIN(v1, v2);
	const id max = MAX(v1, v2);
	const id idx = BINARYSEARCH(adj, idxadj, min, max);
	//printf("idx = %u \n", idx);
	//printf("%d\n", (idx > X(idxadj, min)) ? -1 : Y(adj, idx + Y(idxadj, min)));
	return (idx > X(idxadj, min)) ? -1 : Y(adj, idx + Y(idxadj, min));
}

__attribute__((always_inline)) inline
void removeedge(idc *adj, idc *idxadj, id v1, id v2) {

	//printf("removing edge (%u, %u)\n", v1, v2);
	const id min = MIN(v1, v2);
	const id max = MAX(v1, v2);
	const id idx = BINARYSEARCH(adj, idxadj, min, max);
	//printf("min = %u, idx = %u\n", min, idx);
	id n = X(idxadj, min) - idx - 1;

	for (id j = min + 1; j < N; j++) {
		n += X(idxadj, j);
		Y(idxadj, j)--;
	}

	//printf("moving %u items from %u to %u\n", n, Y(idxadj, min) + idx + 1, Y(idxadj, min) + idx);
	memmove(adj + Y(idxadj, min) + idx, adj + Y(idxadj, min) + idx + 1, sizeof(idc) * n);
	X(idxadj, min)--;

	/*for (id i = 0; i < N; i++) {
		printf("%u = [ ", i);
		for (id j = 0; j < X(idxadj, i); j++)
			printf("%u (%u) ", X(adj, j + Y(idxadj, i)), Y(adj, j + Y(idxadj, i)));
		printf("]\n");
	}*/
}

// move edge e (v2, i) -> (v1, i)

__attribute__((always_inline)) inline
void moveedge(idc *adj, idc *idxadj, id v1, id v2, id i, id e) {

	//printf("moving edge (%u, %u) -> (%u, %u)\n", v2, i, v1, i);
	const id minsrc = MIN(v2, i);
	const id mindst = MIN(v1, i);
	const id maxsrc = MAX(v2, i);
	const id maxdst = MAX(v1, i);
	const id idxsrc = BINARYSEARCH(adj, idxadj, minsrc, maxsrc);		// old index of e in minsrc's list
	const id idxdst = INSERTIONORDERED(adj, idxadj, mindst, maxdst);	// new index of e in mindst's list
	//printf("minsrc = %u, mindst = %u, idxsrc = %u, idxdst = %u\n", minsrc, mindst, idxsrc, idxdst);

	if (minsrc == mindst) {
		//printf("moving within same agent\n");
		X(adj, Y(idxadj, minsrc) + idxsrc) = maxdst;
		#define LTX(X, Y) ((*(X)).x < (*(Y)).x)
		QSORT(idc, adj + Y(idxadj, minsrc), X(idxadj, minsrc), LTX);
	} else {

		if (minsrc < mindst) { // move backwards

			id n = X(idxadj, minsrc) - idxsrc - 1;
			n += idxdst;

			for (id j = minsrc + 1; j < mindst; j++) {
				n += X(idxadj, j);
				Y(idxadj, j)--;
			}

			//printf("moving %u items from %u to %u\n", n, Y(idxadj, minsrc) + idxsrc + 1, Y(idxadj, minsrc) + idxsrc);
			//for (id i = 0; i < E; i++) printf("%u-%u,", X(adj, i), Y(adj, i));
			//puts("");
			memmove(adj + Y(idxadj, minsrc) + idxsrc, adj + Y(idxadj, minsrc) + idxsrc + 1, sizeof(idc) * n);
			//for (id i = 0; i < E; i++) printf("%u-%u,", X(adj, i), Y(adj, i));
			//puts("");
			Y(idxadj, mindst)--;

		} else { // move forwards

			id n = X(idxadj, mindst) - idxdst;
			n += idxsrc;

			for (id j = mindst + 1; j < minsrc; j++) {
				n += X(idxadj, j);
				Y(idxadj, j)++;
			}

			//printf("n = %u\n", n);
			memmove(adj + Y(idxadj, mindst) + idxdst + 1, adj + Y(idxadj, mindst) + idxdst, sizeof(idc) * n);
			Y(idxadj, minsrc)++;
		}

		X(adj, Y(idxadj, mindst) + idxdst) = maxdst;
		Y(adj, Y(idxadj, mindst) + idxdst) = e;
		X(idxadj, mindst)++;
		X(idxadj, minsrc)--;
	}

	/*for (id i = 0; i < N; i++) {
		printf("%u = [ ", i);
		for (id j = 0; j < X(idxadj, i); j++)
			printf("%u (%u) ", X(adj, j + Y(idxadj, i)), Y(adj, j + Y(idxadj, i)));
		printf("]\n");
	}*/
}

// Contract edge between v1 and v2

__attribute__((always_inline)) inline
void contract(stack *st, id v1, id v2) {

	/*for (id i = 0; i < N; i++) {
		printf("%u = [ ", i);
		for (id j = 0; j < X(st->idxadj, i); j++)
			printf("%u (%u) ", X(st->adj, j + Y(st->idxadj, i)), Y(st->adj, j + Y(st->idxadj, i)));
		printf("]\n");
	}*/

	register id i, m = st->n[N];
	register const id *p = st->n + N + 1;
	register int e, f;

	do if ((i = *(p++)) != v1) {
		//if ((e = st->g[i * N + v2])) {
		//printf("i = %u v1 = %u v2 = %u\n", i, v1, v2);
		if ((e = getedge(st->adj, st->idxadj, i, v2)) >= 0) {
			//printf("e = %u\n", e + 0);
			//if ((f = st->g[i * N + v1])) {
			if ((f = getedge(st->adj, st->idxadj, i, v1)) >= 0) {
				//printf("f = %u\n", f + 0);
				if (!GET(st->c, f)) CLEAR(st->c, e);
				st->v[e] += st->v[f];
				CLEAR(st->c, f);
				removeedge(st->adj, st->idxadj, i, v1);
				//printf("remove edge %u (%u, %u)\n", f, i, v1);
			}
			//st->g[i * N + v1] = st->g[v1 * N + i] = e;
			moveedge(st->adj, st->idxadj, v1, v2, i, e);
			X(st->a, e) = v1;
			Y(st->a, e) = i;
		}}
	while (--m);

	//removeedge(st->adj, st->idxadj, v1, v2);

	/*for (id i = 0; i < N; i++) {
		printf("%u = [ ", i);
		for (id j = 0; j < X(st->idxadj, i); j++)
			printf("%u (%u) ", X(st->adj, j + Y(st->idxadj, i)), Y(st->adj, j + Y(st->idxadj, i)));
		printf("]\n");
	}*/
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

	st->s[v1] += st->s[v2];
	(st->n[N])--;
}

/*__attribute__((always_inline)) inline
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
}*/

// Print coalition structure

/*void printcs(stack *st) {

	const id *p = st->n + N + 1;
        id m = st->n[N];

	do {
		id i = *(p++);
                printf("{ ");
                for (id j = 0; j < X(st->s, i); j++)
                	printf("%s%u%s ", i == st->cs[Y(st->s, i) + j] ? "<" : "",
			       st->cs[Y(st->s, i) + j], i == st->cs[Y(st->s, i) + j] ? ">" : "");
                printf("}");
        } while (--m);

	puts("");
}*/

__attribute__((always_inline))
inline value totalk(const stack *st) {

	value k = 0;
	const id *p = st->n + N + 1;
        id m = st->n[N];

	do {
		id i = *(p++);
		k += KAPPA(st->s[i]);
        } while (--m);

	return k;
}

__attribute__((always_inline)) inline
value bound(const stack *st) {

	register value maxgain = 0;
	chunk tmp[C];
	memcpy(tmp, st->c, sizeof(chunk) * C);
	register id popc = MASKPOPCNT(tmp, C);

	for (id i = 0, e = MASKFFS(tmp, C); i < popc; i++, e = MASKCLEARANDFFS(tmp, e, C))
		if (st->v[e] > 0) maxgain += st->v[e];

	return maxgain + st->val;
}

void cfss(stack *st) {

	count++;
	//printcs(st);

	const value k = totalk(st);
	if (st->val - k > max) { max = st->val - k; sol = *st; }

	#ifdef LIMIT
	if (!stop) {
		gettimeofday(&t2, NULL);
		if ((double)(t2.tv_usec - t1.tv_usec) / 1e6 + t2.tv_sec - t1.tv_sec > LIMIT) stop = true;
	}
	#endif

	if (stop || bound(st) - k <= max) return;

	chunk tmp[C];
	memcpy(tmp, st->c, sizeof(chunk) * C);
	register id popc = MASKPOPCNT(tmp, C);

	for (id i = 0, e = MASKFFS(tmp, C); !stop && i < popc; i++, e = MASKCLEARANDFFS(tmp, e, C)) {
		register id v1 = X(st->a, e);
		register id v2 = Y(st->a, e);
		//CLEAR(st->c, st->g[v1 * N + v2]);
		//printf("%lu\n", st->c[0]);
		//printf("clearing %u\n", e);
		CLEAR(st->c, e);
		//printf("%lu\n", st->c[0]);
		st[1] = st[0];
		merge(st + 1, v1, v2);
		contract(st + 1, v1, v2);
		st[1].val += st[1].v[e];
		cfss(st + 1);
	}
}

void boundlevel(stack *st, id l) {

	value b = bound(st) - totalk(st);
	chunk tmp[C];
	memcpy(tmp, st->c, sizeof(chunk) * C);
	register id popc = MASKPOPCNT(tmp, C);
	if (l - 1 == 0 || popc == 0) bou = b > bou ? b : bou;

	if (l - 1) for (id i = 0, e = MASKFFS(tmp, C); i < popc; i++, e = MASKCLEARANDFFS(tmp, e, C)) {
		register id v1 = X(st->a, e);
		register id v2 = Y(st->a, e);
		CLEAR(st->c, e);
		st[1] = st[0];
		merge(st + 1, v1, v2);
		contract(st + 1, v1, v2);
		boundlevel(st + 1, l - 1);
	}
}

inline void createedge(idc *a, id v1, id v2, id e) {

	X(a, e) = v1;
	Y(a, e) = v2;
}

#ifndef TWITTER
void scalefree(idc *a) {

	unsigned deg[N] = {0};
	register uint_fast64_t d, i, j, h, k = 0, q, t = 0;
	register int p;

	for (i = 1; i <= K; i++) {
		for (j = 0; j < i; j++) {
			createedge(a, i, j, k++);
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
				createedge(a, i, q, k++);
				deg[i]++;
				deg[q]++;
			}
		}
	}
}
#endif

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

	for (id i = 0; i < N; i++)
		QSORT(idc, st->adj + Y(st->idxadj, i), X(st->idxadj, i), LTX);

	/*for (id i = 0; i < N; i++) {
		printf("%u = [ ", i);
		for (id j = 0; j < X(st->idxadj, i); j++)
			printf("%u (%u) ", X(st->adj, j + Y(st->idxadj, i)), Y(st->adj, j + Y(st->idxadj, i)));
		printf("]\n");
	}*/
}

void reorderedges(idc *a, value *v) {

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
		createedge(a, evb[i].v1, evb[i].v2, i);
}

int main(int argc, char *argv[]) {

	/*
	printf("Tot  = %zu\n", sizeof(stack) * N);
	printf("adj  = %zu\n", sizeof(idc) * N * E);
	printf("iadj = %zu\n", sizeof(idc) * N * N);
	printf("a    = %zu\n", sizeof(idc) * N * E);
	printf("n    = %zu\n", sizeof(id) * N * (2 * N + 1));
	printf("E    = %u\n", E);
	printf("d    = %f\n", (double)E / (N * N));
	*/

	//exit(0);

	#ifndef TWITTER
	if (argc != 2) {
		fprintf(stderr, "Usage: cfss seed\n");
		exit(1);
	}
	#endif

	stack *st = (stack *)malloc(sizeof(stack) * N);
	if (!st) { puts("Error allocating stack"); exit(1); }
	memset(st->c, 0, sizeof(chunk) * C);
	st->n[N] = N;

	for (id i = 0; i < N; i++) {
		st->n[st->n[i] = N + i + 1] = i;
		st->s[i] = 1;
	}

	ONES(st->c, E, C);

	// Initialise graph

	#ifdef TWITTER
	memcpy(st->a, a, sizeof(idc) * E);
	id seed = SEED;
	#else
	id seed = atoi(argv[1]);
	init(seed);
	scalefree(st->a);
	#endif

	// initialise edge values

	init(seed);
	value lb = -N; // lower bound for characteristic function

	for (id i = 0; i < E; i++) {
		st->v[i] = nextInt(RANGE * 2) - 14;
		lb += st->v[i] < 0 ? st->v[i] : 0;
	}

	#ifdef REORDER
	reorderedges(st->a, st->v);
	#endif

	//for (id i = 0; i < E; i++)
	//	printf("%u: (%u, %u) = %f\n", i, X(st->a, i), Y(st->a, i), st->v[i]);

	createadj(st);
	st->val = 0;
	max = -N;
	sol = *st;

	#ifdef LIMIT
	for (id i = 1; i <= BOUNDLEVEL; i++) {
		bou = -FLT_MAX;
		ONES(st->c, E, C);
		boundlevel(st, i);
	}
	#endif

	ONES(st->c, E, C);
	gettimeofday(&t1, NULL);
	cfss(st);
	gettimeofday(&t2, NULL);
	free(st);

	//printcs(&sol);
	#ifdef LIMIT
	printf("%u,%u,%u,%f,%f,%f,%f,%f,%zu\n", N, E, seed, max, bou, lb, (bou - lb) / (max - lb), (double)(t2.tv_usec - t1.tv_usec) / 1e6 + t2.tv_sec - t1.tv_sec, count);
	#else
	printf("%u,%u,%u,%f,%f,%zu\n", N, E, seed, max, (double)(t2.tv_usec - t1.tv_usec) / 1e6 + t2.tv_sec - t1.tv_sec, count);
	#endif

	return 0;
}
