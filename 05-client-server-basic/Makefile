CC = gcc
CFLAGS = -Wall -Wextra

all: Client Server

Client: Client.c
	$(CC) $(CFLAGS) -o Client Client.c

Server: Server.c
	$(CC) $(CFLAGS) -o Server Server.c

clean:
	rm -f Client Server

fclean: clean
	rm -f Client Server anagramme.*

re: fclean all

.PHONY: all clean fclean re
