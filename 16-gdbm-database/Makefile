CC = gcc
CFLAGS = -Wall -Wextra -I.
LDFLAGS = -lgdbm

TARGET = gdbm
SRC = gdbm.c

all: $(TARGET)

$(TARGET): $(SRC) lib.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET) test.gdbm

fclean: clean
	rm -f *.o

re: fclean all

.PHONY: all clean fclean re
