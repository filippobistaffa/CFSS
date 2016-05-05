.PHONY:

OPT=-Wall -march=native -Ofast -funroll-loops -funsafe-loop-optimizations -falign-functions=16 -falign-loops=16
LIB=-lm
OUT=cfss

all:
	g++ ${OPT} *.c *.cpp ${LIB} -o ${OUT}

run:
	./cfss
