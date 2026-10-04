NAME = flfcntl

CC = gcc
CFLAGS = -Wall -Wextra -Werror

SRC = flfcntl.c
OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

%.o: %.c lib.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(NAME)

fclean: clean
	rm -f $(NAME) verrou.txt

re: fclean all

.PHONY: all clean fclean re