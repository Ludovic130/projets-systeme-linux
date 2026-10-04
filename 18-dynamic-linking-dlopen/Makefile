CC = gcc
CFLAGS = -Wall -Wextra -I. -fPIC
LDFLAGS = -ldl

TARGET = dlopen
LIB = libmathN.so

all: $(TARGET) $(LIB)

$(TARGET): dlopen.c lib.h $(LIB)
	$(CC) $(CFLAGS) -o $(TARGET) dlopen.c $(LDFLAGS)

$(LIB): libmathN.c
	$(CC) $(CFLAGS) -shared -o $(LIB) libmathN.c

clean:
	rm -f $(TARGET) $(LIB)

fclean: clean
	rm -f *.o

re: fclean all

.PHONY: all clean fclean re
