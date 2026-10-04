CC = gcc
CFLAGS = -Wall -Wextra -g

TARGETS = tcp_client tcp_server

all: $(TARGETS)

tcp_client: tcp_client.o
	$(CC) $(CFLAGS) -o $@ $^

tcp_server: tcp_server.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c lib.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGETS)

.PHONY: all clean