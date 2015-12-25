#ifndef SORTED_H_
#define SORTED_H_

/*
 * x = first input set
 * m = cardinality of x
 * y = second input set
 * n = cardinality of y
 * z = output set
 * o = cardinality of z
 */

template <typename type, typename out> __attribute__((always_inline)) inline
void unionsorted(const type *x, unsigned m, const type *y, unsigned n, type *z, out *o) {

	*o = 0;

	while (m && n) {
		if (LT(x, y)) { *(z++) = *(x++); m--; }
		else if (LT(y, x)) { *(z++) = *(y++); n--; }
		else { *(z++) = *(y++); x++; m--; n--; }
		(*o)++;
	}

	(*o) += m + n;
	if (m) memcpy(z, x, sizeof(type) * m);
	else memcpy(z, y, sizeof(type) * n);
}

template <typename type, typename out> __attribute__((always_inline)) inline
void intersectionsorted(const type *x, unsigned m, const type *y, unsigned n, type *z, out *o) {

	*o = 0;

	while (m && n) {
		if (LT(x, y)) { x++; m--; }
		else if (LT(y, x)) { y++; n--; }
		else { *(z++) = *(y++); x++; m--; n--; (*o)++; }
	}
}

template <typename type, typename out> __attribute__((always_inline)) inline
void differencesorted(const type *x, unsigned m, const type *y, unsigned n, type *z, out *o) {

	*o = 0;

	while (m && n) {
		if (LT(x, y)) { *(z++) = *(x++); m--; (*o)++; }
		else if (LT(y, x)) { y++; n--; }
		else { y++; x++; m--; n--; }
	}

	if (!m) return;
	(*o) += m;
	memcpy(z, x, sizeof(type) * m);
}

template <typename type> __attribute__((always_inline)) inline
uint8_t intersectionchecksorted(const type *x, unsigned m, const type *y, unsigned n) {

	while (m && n) {
		if (LT(x, y)) { x++; m--; }
		else if (LT(y, x)) { y++; n--; }
		else return 1;
	}

	return 0;
}

template <typename type> __attribute__((always_inline)) inline
unsigned binarysearch(type x, const type *buf, unsigned n) {

	if (n) {
		#define MIDPOINT(_min, _max) (_min + ((_max - _min) / 2))
		register unsigned imin = 0, imid, imax = n - 1;

		while (imin < imax) {
			imid = MIDPOINT(imin, imax);
			if (LT(buf + imid, &x)) imin = imid + 1;
			else imax = imid;
		}

		if (imax == imin && buf[imin] == x) return imin;
	}
	return n + 1;
}

template <typename type> __attribute__((always_inline)) inline
unsigned insertionordered(type x, const type *buf, unsigned n) {

	if (n) {
		register unsigned imin = 0, imid, imax = n - 1;

		if (LT(&x, buf + imin)) return imin;
		if (LT(buf + imax, &x)) return n;

		while (imax - imin != 1) {
			imid = MIDPOINT(imin, imax);
			//printf("1: %u %u %u\n", imin, imid, imax);
			if (LT(buf + imid, &x)) imin = imid;
			else imax = imid;
			//printf("2: %u %u %u\n", imin, imid, imax);
		}

		return imax;
	}
	return 0;
}

#endif /* SORTED_H_ */
