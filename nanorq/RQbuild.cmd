#!/bin/bash
function run_cmd () {
  echo "$*"
  $*
}
run_cmd rm -rf *.o *.a *.so
run_cmd make clean
run_cmd make libnanorq.a
run_cmd gcc -shared bitmask.o io.o params.o precode.o rand.o sched.o spmat.o tuple.o wrkmat.o nanorq.o RaptorQ.o  oblas/*.o -o libRQ.so
