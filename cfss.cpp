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

	const id min = MIN(v1, v2);
	const id max = MAX(v1, v2);
	const id idx = BINARYSEARCH(adj, idxadj, min, max);
	id n = X(idxadj, min) - idx - 1;

	for (id j = min + 1; j < N; j++) {
		n += X(idxadj, j);
		Y(idxadj, j)--;
	}

	memmove(adj + Y(idxadj, min) + idx, adj + Y(idxadj, min) + idx + 1, sizeof(idc) * n);
	X(idxadj, min)--;
}

// Move edge e (v2, i) -> (v1, i)

__attribute__((always_inline)) inline
void moveedge(idc *adj, idc *idxadj, id v1, id v2, id i, id e) {

	const id minsrc = MIN(v2, i);
	const id mindst = MIN(v1, i);
	const id maxsrc = MAX(v2, i);
	const id maxdst = MAX(v1, i);
	const id idxsrc = BINARYSEARCH(adj, idxadj, minsrc, maxsrc);		// old index of e in minsrc's list
	const id idxdst = INSERTIONORDERED(adj, idxadj, mindst, maxdst);	// new index of e in mindst's list

	if (minsrc == mindst) {
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

			memmove(adj + Y(idxadj, minsrc) + idxsrc, adj + Y(idxadj, minsrc) + idxsrc + 1, sizeof(idc) * n);
			Y(idxadj, mindst)--;

		} else { // move forwards

			id n = X(idxadj, mindst) - idxdst;
			n += idxsrc;

			for (id j = mindst + 1; j < minsrc; j++) {
				n += X(idxadj, j);
				Y(idxadj, j)++;
			}

			memmove(adj + Y(idxadj, mindst) + idxdst + 1, adj + Y(idxadj, mindst) + idxdst, sizeof(idc) * n);
			Y(idxadj, minsrc)++;
		}

		X(adj, Y(idxadj, mindst) + idxdst) = maxdst;
		Y(adj, Y(idxadj, mindst) + idxdst) = e;
		X(idxadj, mindst)++;
		X(idxadj, minsrc)--;
	}
}

// Contract edge between v1 and v2

__attribute__((always_inline)) inline
void contract(stack *st, id v1, id v2) {

	register id i, m = st->n[N];
	register const id *p = st->n + N + 1;
	register int e, f;

	do if ((i = *(p++)) != v1) {
		if ((e = getedge(st->adj, st->idxadj, i, v2)) >= 0) {
			if ((f = getedge(st->adj, st->idxadj, i, v1)) >= 0) {
				if (!GET(st->c, f)) CLEAR(st->c, e);
				st->v[e] += st->v[f];
				CLEAR(st->c, f);
				removeedge(st->adj, st->idxadj, i, v1);
			}
			moveedge(st->adj, st->idxadj, v1, v2, i, e);
			X(st->a, e) = v1;
			Y(st->a, e) = i;
		}}
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

	st->sing[v1] += st->sing[v2];
	st->s[v1] += st->s[v2];
	(st->n[N])--;
}

#if KAPPA(S) != 0
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
#endif

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

	#if KAPPA(S) != 0
	const value k = totalk(st);
	#else
	const value k = 0;
	#endif

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
		id v1 = X(st->a, e);
		id v2 = Y(st->a, e);
		CLEAR(st->c, e);
		SET(st->e, e);
		st[1] = st[0];
		merge(st + 1, v1, v2);
		contract(st + 1, v1, v2);
		st[1].val += st[1].v[e];
		cfss(st + 1);
	}
}

void boundlevel(stack *st, id l) {

	#if KAPPA(S) != 0
	value b = bound(st) - totalk(st);
	#else
	value b = bound(st);
	#endif
	chunk tmp[C];
	memcpy(tmp, st->c, sizeof(chunk) * C);
	id popc = MASKPOPCNT(tmp, C);
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

__attribute__((always_inline)) inline
void createedge(idc *a, value *v, id v1, id v2, id e, value w) {

	X(a, e) = v1;
	Y(a, e) = v2;
	v[e] = w;
}

#ifdef M
void scalefree(idc *a, value *v) {

	id k = 0;
	unsigned deg[N] = {0};

	for (id i = 1; i <= M; i++) {
		for (id j = 0; j < i; j++) {
			createedge(a, v, i, j, k++, RANDOMVALUE);
			deg[i]++;
			deg[j]++;
		}
	}

	id t = 0;

	for (id i = M + 1; i < N; i++) {
		t &= ~((1ULL << i) - 1);
		for (id j = 0; j < M; j++) {
			id d = 0;
			for (id h = 0; h < i; h++)
				if (!((t >> h) & 1)) d += deg[h];
			if (d > 0) {
				int p = nextInt(d);
				id q = 0;
				while (p >= 0) {
					if (!((t >> q) & 1)) p = p - deg[q];
					q++;
				}
				q--;
				t |= 1ULL << q;
				createedge(a, v, i, q, k++, RANDOMVALUE);
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
		createedge(a, v, evb[i].v1, evb[i].v2, i, v[i]);
}

void printsol(chunk *e, idc *a) {

	puts("Solution's spanning forest:");
	id popc = MASKPOPCNT(e, C);

	for (id i = 0, j = MASKFFS(e, C); i < popc; i++, j = MASKCLEARANDFFS(e, j, C))
		printf("%u: (%u, %u)\n", j, X(a, j), Y(a, j));
}

int main(int argc, char *argv[]) {

	//printf("Stack size = %zu bytes\n", sizeof(stack) * N);
	/*
	printf("adj  = %zu\n", sizeof(idc) * N * E);
	printf("iadj = %zu\n", sizeof(idc) * N * N);
	printf("a    = %zu\n", sizeof(idc) * N * E);
	printf("n    = %zu\n", sizeof(id) * N * (2 * N + 1));
	printf("E    = %u\n", E);
	printf("d    = %f\n", (double)E / (N * N));
	*/

	stack *st = (stack *)malloc(sizeof(stack) * N);
	if (!st) { puts("Error allocating stack"); exit(1); }
	memset(st->e, 0, sizeof(chunk) * C);
	st->n[N] = N;

	for (id i = 0; i < N; i++) {
		st->n[st->n[i] = N + i + 1] = i;
		st->s[i] = 1;
	}

	ONES(st->c, E, C);
	st->val = 0;

	// Create graph

	#ifdef M
	id seed = atoi(argv[1]);
	init(seed);
	scalefree(st->a, st->v);
	memset(st->sing, 0, sizeof(value) * N);
	#else
	FILE *f = fopen(argv[1], "r");
	for (id i = 0; i < N; i++) {
		fscanf(f, "%f", st->sing + i);
		st->val += st->sing[i];
	}
	for (id i = 0; i < E; i++)
		fscanf(f, "%u %u %f", &X(st->a, i), &Y(st->a, i), st->v + i);
	fclose(f);
	#endif

	max = st->val;

	#ifdef DOT
	puts("graph G {");
	for (id i = 0; i < E; i++)
		printf("\t%u -- %u;\n", X(st->a, i), Y(st->a, i));
	puts("}\n");
	#endif

	#ifdef DEBUG
	puts("Singletons:");
	for (id i = 0; i < N; i++)
		printf("%u = %f\n", i, st->sing[i]);
	puts("\nEdges:");
	for (id i = 0; i < E; i++)
		printf("%u: (%u, %u) = %f\n", i, X(st->a, i), Y(st->a, i), st->v[i]);
	puts("");
	#endif

	// Compute lower bound for characteristic function

	/*#ifdef LIMIT
	value lb = -N;
	for (id i = 0; i < E; i++)
		lb += st->v[i] < 0 ? st->v[i] : 0;
	#endif*/

	#ifdef REORDER
	reorderedges(st->a, st->v);
	#ifdef DEBUG
	puts("Reordered edges:");
	for (id i = 0; i < E; i++)
		printf("%u: (%u, %u) = %f\n", i, X(st->a, i), Y(st->a, i), st->v[i]);
	puts("");
	#endif
	#endif

	createadj(st);
	sol = *st;
	idc in[E];
	memcpy(in, sol.a, sizeof(idc) * E);

	/*#ifdef LIMIT
	for (id i = 1; i <= BOUNDLEVEL; i++) {
		bou = FLT_MAX;
		ONES(st->c, E, C);
		boundlevel(st, i);
	}
	#endif*/

	ONES(st->c, E, C);
	gettimeofday(&t1, NULL);
	cfss(st);
	gettimeofday(&t2, NULL);
	free(st);

	#ifdef CSV
	#ifdef LIMIT
	printf("%u,%u,%s,%f,%f,%f,%f,%f,%zu\n", N, E, argv[1], max, bou, lb, (bou - lb) / (max - lb), (double)(t2.tv_usec - t1.tv_usec) / 1e6 + t2.tv_sec - t1.tv_sec, count);
	#else
	printf("%u,%u,%s,%f,%f,%zu\n", N, E, argv[1], max, (double)(t2.tv_usec - t1.tv_usec) / 1e6 + t2.tv_sec - t1.tv_sec, count);
	#endif
	#else
	printf("Visited nodes = %zu\n", count);
	printf("Optimal value = %f\n", max);
	printsol(sol.e, in);
	#endif

	return 0;
}
