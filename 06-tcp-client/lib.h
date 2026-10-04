#if !defined(LIB)
#define LIB

#define _GNU_SOURCE   // Required for getopt, gethostbyname, getservbyname

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define LG_BUFFER 4096   // Size of the read buffer

#endif // LIB
