CC=gcc
CFLAGS=-g -Wall -Wextra -Werror
EXAMPLE_SOURCES=$(wildcard examples/*.c)
EXAMPLE_BINARIES=$(EXAMPLE_SOURCES:.c=)


all: test

jacon.o: jacon.c
	$(CC) $< -c $(CFLAGS) -o $@

jacon_test: test.c jacon.o
	$(CC) $^ $(CFLAGS) -o $@
test: jacon_test
	JSONTestSuite/run_tests.py test_meta.json .

examples: $(EXAMPLE_BINARIES)
examples/%: examples/%.c jacon.o
	$(CC) $^ $(CFLAGS) -o $@

hashmap_perf: hashmap_perf.c jacon.o
	$(CC) $^ -o $@
perf-test: hashmap_perf
	./$<

clean:
	rm -f jacon_test hashmap_perf *.o $(EXAMPLE_BINARIES)

.PHONY: all test examples perf-test clean
