OBJ=\
bitmask.o\
io.o\
params.o\
precode.o\
rand.o\
sched.o\
spmat.o\
tuple.o\
wrkmat.o\
nanorq.o\
RaptorQ.o

CPPFLAGS = -D_DEFAULT_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS   = -fPIC -O3 -g -std=c99 -Wall -I. -Ioblas
CFLAGS  += -march=native -funroll-loops -ftree-vectorize -fno-inline

all: test libnanorq.a

test: encode decode
	$(MAKE) -f example.make

encode: encode.o libnanorq.a

decode: decode.o libnanorq.a

benchmark: benchmark.o libnanorq.a

bench: graph.dat
	cat graph.dat

bench.md: graph.dat
	cat graph.dat | awk -f graph.awk

graph.dat: benchmark
	echo "K       encode   precalc  decode  decode-oh5" > graph.dat
	./benchmark 1280   100 5.0 >> graph.dat
	./benchmark 1280   500 5.0 >> graph.dat
	./benchmark 1280  1000 5.0 >> graph.dat
	./benchmark 1280  5000 5.0 >> graph.dat
	./benchmark 1280 10000 5.0 >> graph.dat
	./benchmark 1280 50000 5.0 >> graph.dat

graph.png: graph.dat graph.gnuplot
	gnuplot -e "argtitle='Throughput (packet size=1280) `lscpu|grep -i 'model name'|cut -f2 -d:|xargs`'" graph.gnuplot

oblas/liboblas.a:
	$(MAKE) -C oblas CPPFLAGS+="-DOBLAS_SSE"

# 伪目标
.PHONY: oblas_clean
oblas_clean:
	$(MAKE) -C oblas clean

libnanorq.a: $(OBJ) oblas/liboblas.a
	$(AR) rcs $@ $(OBJ) oblas/*.o

# oblas_clean是前置条件
clean: oblas_clean
	$(RM) encode decode *.o *.a *.gcda *.gcno *.gcov callgrind.* *.gperf *.prof *.heap perf.data perf.data.old

indent:
	clang-format -style=LLVM -i *.c *.h

scan:
	scan-build $(MAKE) clean benchmark

profile:
	$(RM) callgrind.*
	valgrind --tool=callgrind ./benchmark 1280 1000 5.0
	gprof2dot --format=callgrind callgrind.out.* -z main | dot -T svg > callgrind.svg

gcov: CFLAGS += -O0 -fprofile-arcs -ftest-coverage
gcov: LDFLAGS = -lgcov --coverage
gcov: clean benchmark
	./benchmark 1280 1000 5.0

gperf: LDFLAGS = -lprofiler -ltcmalloc
gperf: clean benchmark
	CPUPROFILE_FREQUENCY=100000 CPUPROFILE=gperf.prof ./benchmark 1280 50000 5.0
	pprof ./benchmark gperf.prof --callgrind > callgrind.gperf
	gprof2dot --format=callgrind callgrind.gperf -z main | dot -T svg > gperf.svg
#	pprof ./encode gperf.prof --text

gheap: LDFLAGS = -lprofiler -ltcmalloc
gheap: clean benchmark
	$(RM) gmem.prof.*
	HEAPPROFILE=gmem.prof HEAP_PROFILE_INUSE_INTERVAL=1024000 ./benchmark 1280 50000 5.0
	pprof ./benchmark gmem.prof.*.heap --callgrind > memgrind.gperf
	gprof2dot --format=callgrind memgrind.gperf | dot -T svg > gheap.svg

perf:
	perf record -g ./benchmark 1280 5000 5.0
	perf script | gprof2dot --format=perf | dot -T svg > perf.svg
	#perf report

ubsan: CC=clang
ubsan: CFLAGS += -fsanitize=undefined,implicit-conversion,integer
ubsan: LDFLAGS += -lubsan
ubsan: clean benchmark
	./benchmark 1280 50000 0

