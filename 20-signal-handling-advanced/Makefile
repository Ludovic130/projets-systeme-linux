CC = gcc
CFLAGS = -Wall -Wextra -g

TARGETS = signals sigqueue_demo

all: $(TARGETS)

signals: signals.o
	$(CC) $(CFLAGS) -o $@ $^

sigqueue_demo: sigqueue_demo.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c lib.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGETS)

.PHONY: all clean