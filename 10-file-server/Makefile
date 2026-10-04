CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lpthread
TARGET = file_server
SRCS = file_server.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.txt

.PHONY: all clean