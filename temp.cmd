#!/bin/bash
function run_cmd () {
  echo "$*"
  $*
}

run_cmd cc -O3 -g -std=c99 -Wall -I. -Ioblas -march=native -funroll-loops -ftree-vectorize -fno-inline -D_DEFAULT_SOURCE -D_FILE_OFFSET_BITS=64  -c -o RaptorQ.o RaptorQ.c
run_cmd cc -O3 -g -std=c99 -Wall -I. -Ioblas -march=native -funroll-loops -ftree-vectorize -fno-inline -D_DEFAULT_SOURCE -D_FILE_OFFSET_BITS=64  -c -o temp.o temp.c
run_cmd cc RaptorQ.o temp.o libnanorq.a -o temp
run_cmd ./temp
