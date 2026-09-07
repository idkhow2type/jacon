CFLAGS=-g -Wall -Wextra -Werror

all: main

%.o: %.c
	$(CC) $< -c $(CFLAGS) -o $@
main: main.o json.o
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm -f tinywl tinywl.o

.PHONY: all clean
