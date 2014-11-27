#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>

int bind_socket(struct addrinfo* addr_list);
int create_server_socket(char* port);

#endif