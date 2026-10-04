CC = gcc
CFLAGS = -Wall -Wextra -I.

TARGET = hash_table
SRC = hash_table.c

all: $(TARGET)

$(TARGET): $(SRC) lib.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

fclean: clean
	rm -f *.o

re: fclean all

.PHONY: all clean fclean re
