CC ?= gcc
CFLAGS ?= -O3 -march=native -std=c11 -Wall -Wextra -Wpedantic -fno-tree-vectorize -fno-tree-loop-distribute-patterns -fno-builtin -fno-optimize-strlen
STRLEN_DEFAULT_LEN ?= 1048576
STRLEN_PERF_DEFAULT_LEN ?= 16777216
STRLEN_BENCH_LEN ?= 1048576

PRINT_FLOAT_BITS := numerics/print_float_bits
PRINT_FLOAT_BITS_SRC := numerics/print_float_bits.c
STRLEN_BENCH := numerics/strlen_bench
STRLEN_SINGLE_BENCH := numerics/strlen_single_bench
STRLEN_FOUR_BENCH := numerics/strlen_four_bench
STRLEN_SINGLE_PERF := numerics/strlen_single_perf
STRLEN_FOUR_PERF := numerics/strlen_four_perf
STRLEN_BENCH_SRC := numerics/strlen_bench.c
STRLEN_IMPL_SRC := numerics/simple_strlen.c

.PHONY: all print-float-bits strlen-bench strlen-single-bench strlen-four-bench \
	strlen-single-perf strlen-four-perf \
	run-print-float-bits run-strlen-bench run-strlen-single-bench run-strlen-four-bench \
	run-strlen-single-perf run-strlen-four-perf \
	run-all clean

all: print-float-bits strlen-bench strlen-single-bench strlen-four-bench strlen-single-perf strlen-four-perf

print-float-bits: $(PRINT_FLOAT_BITS)

strlen-bench: $(STRLEN_BENCH)

strlen-single-bench: $(STRLEN_SINGLE_BENCH)

strlen-four-bench: $(STRLEN_FOUR_BENCH)

strlen-single-perf: $(STRLEN_SINGLE_PERF)

strlen-four-perf: $(STRLEN_FOUR_PERF)

$(PRINT_FLOAT_BITS): $(PRINT_FLOAT_BITS_SRC)
	$(CC) $(CFLAGS) $(PRINT_FLOAT_BITS_SRC) -o $(PRINT_FLOAT_BITS)

$(STRLEN_BENCH): $(STRLEN_BENCH_SRC) $(STRLEN_IMPL_SRC)
	$(CC) $(CFLAGS) -DSTRLEN_BENCH_LEN=$(STRLEN_BENCH_LEN) $(STRLEN_BENCH_SRC) $(STRLEN_IMPL_SRC) -o $(STRLEN_BENCH)

$(STRLEN_SINGLE_BENCH): $(STRLEN_BENCH_SRC) $(STRLEN_IMPL_SRC)
	$(CC) $(CFLAGS) -DSTRLEN_BENCH_LEN=$(STRLEN_BENCH_LEN) -DSTRLEN_BENCH_MODE=STRLEN_BENCH_SINGLE $(STRLEN_BENCH_SRC) $(STRLEN_IMPL_SRC) -o $(STRLEN_SINGLE_BENCH)

$(STRLEN_FOUR_BENCH): $(STRLEN_BENCH_SRC) $(STRLEN_IMPL_SRC)
	$(CC) $(CFLAGS) -DSTRLEN_BENCH_LEN=$(STRLEN_BENCH_LEN) -DSTRLEN_BENCH_MODE=STRLEN_BENCH_FOUR $(STRLEN_BENCH_SRC) $(STRLEN_IMPL_SRC) -o $(STRLEN_FOUR_BENCH)

$(STRLEN_SINGLE_PERF): $(STRLEN_BENCH_SRC) $(STRLEN_IMPL_SRC)
	$(CC) $(CFLAGS) -DSTRLEN_BENCH_LEN=$(STRLEN_PERF_DEFAULT_LEN) -DSTRLEN_BENCH_MODE=STRLEN_BENCH_SINGLE $(STRLEN_BENCH_SRC) $(STRLEN_IMPL_SRC) -o $(STRLEN_SINGLE_PERF)

$(STRLEN_FOUR_PERF): $(STRLEN_BENCH_SRC) $(STRLEN_IMPL_SRC)
	$(CC) $(CFLAGS) -DSTRLEN_BENCH_LEN=$(STRLEN_PERF_DEFAULT_LEN) -DSTRLEN_BENCH_MODE=STRLEN_BENCH_FOUR $(STRLEN_BENCH_SRC) $(STRLEN_IMPL_SRC) -o $(STRLEN_FOUR_PERF)

run-print-float-bits: $(PRINT_FLOAT_BITS)
	./$(PRINT_FLOAT_BITS)

run-strlen-bench: $(STRLEN_BENCH)
	./numerics/run_strlen_bench.sh

run-strlen-single-bench: $(STRLEN_SINGLE_BENCH)
	./$(STRLEN_SINGLE_BENCH)

run-strlen-four-bench: $(STRLEN_FOUR_BENCH)
	./$(STRLEN_FOUR_BENCH)

run-strlen-single-perf: $(STRLEN_SINGLE_PERF)
	./$(STRLEN_SINGLE_PERF)

run-strlen-four-perf: $(STRLEN_FOUR_PERF)
	./$(STRLEN_FOUR_PERF)

run-all: run-print-float-bits run-strlen-bench run-strlen-single-bench run-strlen-four-bench run-strlen-single-perf run-strlen-four-perf

clean:
	rm -f $(PRINT_FLOAT_BITS) $(STRLEN_BENCH) $(STRLEN_SINGLE_BENCH) $(STRLEN_FOUR_BENCH) $(STRLEN_SINGLE_PERF) $(STRLEN_FOUR_PERF)
