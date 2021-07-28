#!/bin/bash
function run_cmd () {
  echo "$*"
  $*
}
run_cmd cc -O3 -g -std=c99 -Wall -I. -Ioblas -march=native -funroll-loops -ftree-vectorize -fno-inline -D_DEFAULT_SOURCE -D_FILE_OFFSET_BITS=64  -c -o benchmark.o benchmark.c
run_cmd cc benchmark.o libnanorq.a -o benchmark
run_cmd ./benchmark 360 20 0 0
