#!/bin/bash
function run_cmd () {
  echo "$*"
  $*
}
rem run_cmd rm -rf *.o *.a
run_cmd rm -rf *.dat
run_cmd make
run_cmd cc -O3 -g -std=c99 -Wall -I. -Ioblas -march=native -funroll-loops -ftree-vectorize -fno-inline -D_DEFAULT_SOURCE -D_FILE_OFFSET_BITS=64  -c -o test.o test.c
run_cmd cc test.o libnanorq.a -o test

run_cmd ./test 375 20 0 10 >> graph_p10.dat
run_cmd ./test 375 20 0 30 >> graph_p30.dat

run_cmd gnuplot graph.gnuplot
