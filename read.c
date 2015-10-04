#include "cfss.h"

void read(value *p, value *t, value *m) {

	FILE *f = fopen(FILENAME, "rb");
	if (!f) { puts("Error opening energy profiles"); exit(1); }
	register size_t i, j;
	register value v;
	char line[LINE], val[WIDTH];
	val[WIDTH - 1] = '\0';

	for (i = 0; i < N; i++) {
		t[i] = 0;
		m[i] = FLT_MAX;
		fseek(f, nextInt(MAXJUMP) * LINE, SEEK_CUR);
		j = fread(line, LINE, 1, f);
		for (j = 0; j < TS; j++) {
			memcpy(val, line + j * WIDTH, WIDTH - 1);
			t[i] += (v = p[i * TS + j] = strtof(val, NULL));
			if (v < m[i]) m[i] = v;
		}
	}

	fclose(f);
}
