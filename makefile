ecosystem: main.c
	gcc -std=c99 functions.c main.c -o ecosystem -fopenmp -O3
# -mtune=native -march=native
