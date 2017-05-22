#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset
#include <netdb.h> // getservbyname

/* prototypes */
int server_socket(const char *service, const char *transport, int queueLength);
int client_socket(const char *host, const char *service, const char *transport);
