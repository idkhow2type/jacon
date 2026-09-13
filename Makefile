CFLAGS=-g -Wall -Wextra -Werror

all: main

%.o: %.c
	$(CC) $< -c $(CFLAGS) -o $@
main: main.o json.o
	$(CC) $^ $(CFLAGS) -o $@
hashmap_perf: hashmap_perf.o json.o
	$(CC) $^ $(CFLAGS) -o $@
perf-test: hashmap_perf


clean:
	rm -f  main test hashmap_perf *.o

.PHONY: all test perf-test clean
