CC = gcc
CFLAGS = -Wall -Wextra -g
LDLIBS = -lreadline

TARGET = mini_shell_pipe
SRCS = Mini-shell_v3.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean