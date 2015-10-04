.PHONY:

all:
	g++ -Wall -march=native -Ofast -funroll-loops -funsafe-loop-optimizations -falign-functions=16 -falign-loops=16 *.c -lm -o cfss

run:
	./cfss
