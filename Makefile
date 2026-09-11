CFLAGS=-g -Wall -Wextra -Werror

all: main

%.o: %.c
	$(CC) $< -c $(CFLAGS) -o $@
main: main.o json.o
	$(CC) $^ $(CFLAGS) -o $@
test: test.o json.o
	$(CC) $^ $(CFLAGS) -lm -o $@

clean:
	rm -f  main test *.o

.PHONY: all test clean
