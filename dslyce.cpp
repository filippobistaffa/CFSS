#include "cfss.h"

typedef int16_t sign;

#define coallt(_a, _b) (memcmp(_a, _b, sizeof(coal)) < 0)

void printc(const agent *c, value v) {

	register agent n = *c;
	printf("{ ");
	while (n--) printf("%u ", *(++c));
	printf("} = %f\n", v);
}

__attribute__((always_inline)) inline
sign twiddle(sign *x, sign *y, sign *z, sign *p) {

	register sign i, j = 1, k;
	while (p[j] <= 0) j++;

	if (p[j - 1] == 0) {

		for (i = j - 1; i != 1; i--) p[i] = -1;

		p[j] = 0;
		*x = *z = 0;
		p[1] = 1;
		*y = j - 1;
	}
	else {
		if (j > 1) p[j - 1] = 0;

		do j++;
		while (p[j] > 0);

		k = j - 1;
		i = j;

		while (p[i] == 0) p[i++] = -1;

		if (p[i] == -1) {

			p[i] = p[k];
			*z = p[k] - 1;
			*x = i - 1;
			*y = k - 1;
			p[k] = -1;
		}
		else {
			if (i == p[0]) return 1;
			else {
				p[j] = p[i];
				*z = p[i] - 1;
				p[i] = 0;
				*x = j - 1;
				*y = i - 1;
			}
		}
	}

	return 0;
}

__attribute__((always_inline)) inline
void inittwiddle(sign m, sign n, sign *p) {

	register sign i;
	p[0] = n + 1;

	for (i = 1; i != n - m + 1; i++) p[i] = 0;

	while (i != n + 1) {
		p[i] = i + m - n;
		i++;
	}

	p[n + 1] = -2;
	if (m == 0) p[1] = 1;
}

__attribute__((always_inline)) inline
void adjacencylist(const agent *a, agent *l) {

	register agent e;
	for (e = 0; e < N; e++) l[e * N] = 0;
	e = E;

	do {
		l[a[0] * N + (l[a[0] * N]++) + 1] = a[1];
		l[a[1] * N + (l[a[1] * N]++) + 1] = a[0];
		a += 2;
	} while (--e);

	for (; e < N; e++) QSORT(agent, l + e * N + 1, l[e * N], LT);
}

__attribute__((always_inline)) inline
void removeagent(agent x, const agent *l, agent *o) {

	for (agent i = 0; i < N; i++) {
		agent j = binarysearch(x, l + i * N + 1, l[i * N]);
		if (j < l[i * N]) {
			memcpy(o + i * N + 1, l + i * N + 1, sizeof(agent) * j);
			memcpy(o + i * N + j + 1, l + i * N + j + 2, sizeof(agent) * (l[i * N] - j - 1));
			o[i * N] = l[i * N] - 1;
		}
		else memcpy(o + i * N, l + i * N, sizeof(agent) * N);
	}
}

__attribute__((always_inline)) inline
void neighbours(const agent *f, agent m, const agent *l, agent *n) {

	if (m) {
		agent t[N + 1];
		memcpy(n, l + *f * N, sizeof(agent) * (l[*f * N] + 1));
		f++;

		while (--m) {
			unionsorted(n + 1, *n, l + *f * N + 1, l[*f * N], t + 1, t);
			memcpy(n, t, sizeof(agent) * (*t + 1));
			f++;
		}
	}
	else *n = 0;
}

__attribute__((always_inline)) inline
void nbar(const agent *f, agent n, const agent *r, const agent *ruf, const agent *l, agent *nb) {

	agent a[N + 1], b[N + 1];
	neighbours(f, n, l, a);
	agent i = 0;
	while (i < *a && LE(a + i + 1, ruf + 1)) i++;
	memmove(a + 1, a + i + 1, sizeof(agent) * (*a - i));
	*a -= i;
	neighbours(r + 1, *r, l, nb);
	unionsorted(nb + 1, *nb, ruf + 1, *ruf, b + 1, b);
	differencesorted(a + 1, *a, b + 1, *b, nb + 1, nb);
}

__attribute__((always_inline)) inline
uint8_t coalition(agent *c, var **hash) {

	register var *t = (var *)calloc(1, sizeof(var));
	memcpy(t->c, c, sizeof(agent) * (c[0] + 1));
	t->v = ((MAXV) - (MINV)) * nextFloat() + (MINV);
	printf("Value for coalition #%u ", HASH_CNT(hh, *hash));
	printc(c, t->v);
	HASH_ADD(hh, *hash, c, sizeof(agent) * (K + 1), t);
	return 1;
}

size_t slyce(agent *r, agent *f, agent m, const agent *l, var **hash) {

	size_t ret = 0;

	if (*r) ret += coalition(r, hash);

	if (*f && m) {

		agent *nr = r + K + 1, *nf = f + N + 1, *nfs = nr + *r + 1, fs[N], rt[N];
		memcpy(rt, r + 1, sizeof(agent) * *r);
		sign w, y, z, p[N + 2];

		for (agent k = 1; k <= MIN(*f, m); k++) {
			*nr = *r + k;
			memcpy(nr + 1, r + 1, sizeof(agent) * *r);
			memcpy(fs, f + *f - k + 1, sizeof(agent) * k);
			memcpy(nfs, fs, sizeof(agent) * k);
			QSORT(agent, nr + 1, *nr, LT);
			nbar(fs, k, r, nr, l, nf);
			ret += slyce(nr, nf, m - k, l, hash);
			inittwiddle(k, *f, p);
			while (!twiddle(&w, &y, &z, p)) {
				fs[z] = f[w + 1];
				memcpy(nr + 1, rt, sizeof(agent) * *r);
				memcpy(nfs, fs, sizeof(agent) * k);
				QSORT(agent, nr + 1, *nr, LT);
				nbar(fs, k, r, nr, l, nf);
				ret += slyce(nr, nf, m - k, l, hash);
			}
		}
	}

	return ret;
}

size_t enumerate(const agent *a, var **hash) {

	agent l[N * N], r[(K + 1) * N], f[(N + 1) * N];
	adjacencylist(a + 2, l);
	register size_t ret = 0;

        for (agent i = 0; i < N; i++) {
                r[0] = 0; f[0] = 1; f[1] = i;
		ret += slyce(r, f, K, l, hash);
        }

	return ret;
}
