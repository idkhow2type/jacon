CC=gcc
CFLAGS=-g -Wall -Wextra -Werror
EXAMPLE_SOURCES=$(wildcard examples/*.c)
EXAMPLE_BINARIES=$(EXAMPLE_SOURCES:.c=)


all: test

jacon.o: jacon.c
	$(CC) $< -c $(CFLAGS) -o $@

test: test.c jacon.o
	$(CC) $^ $(CFLAGS) -o $@

examples: $(EXAMPLE_BINARIES)
examples/%: examples/%.c jacon.o
	$(CC) $^ $(CFLAGS) -o $@
	
playground: examples/playground
examples/playground: examples/playground.c jacon.o
	$(CC) $^ $(CFLAGS) -o $@

%: %.c jacon.o
	$(CC) $^ $(CFLAGS) -o $@

hashmap_perf: hashmap_perf.c jacon.o
	$(CC) $^ -o $@
perf-test: hashmap_perf
	./$<

clean:
	rm -f jacon_test hashmap_perf *.o $(EXAMPLE_BINARIES)

.PHONY: all test examples perf-test clean
