//
//  Networking.h
//  A general purpose network socket library wrapping C's complicated socket API and OpenSSL's poorly documented API into portable and simple functions
//
//  Created by Vinny Trolia on 6/28/18.
//  Copyright © 2018 Vincent W. Trolia. All rights reserved.

#ifndef networking_h
#define networking_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctype.h>
#include "openssl/ssl.h"
#include "openssl/bio.h"
#include "openssl/err.h"
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

// This is a wrapper object so that not only the socket number, but also all of the other host info is returned together so that our functions can run
// independently of each other.
typedef struct{
    int socket;
    struct addrinfo *connectioninfo;
}
connection;

// This is so the listen() connection and the send() receive() connection on the server side can be kept track of
typedef struct{
    int sockaddr;
    int dataaddr;
}
tuple;

// This is for memory management, return the pointers in both functions so they can be freed later
typedef struct{
    SSL *ssl_connection;
    SSL_CTX *ctx;
    int socket;
    int listen;
}
ssl_tuple;

/* functions */
char *get_datetime_s(void);
connection make_connection(const char *domain, const char *port, char *client_port);
int connect_to_server(const char *domain, char *server_port, char *connection_port);
tuple connect_to_client(char *port);

// OpenSSL functions
void initialize_ssl(void);
unsigned long secure_send(SSL *ssl, void *message, int size);
unsigned long secure_recieve(SSL *ssl, void *buffer, int size);
void create_authorization(void);
ssl_tuple secure_connect_to_client(const char *prikey_file, const char *cert_file, char *port);
ssl_tuple secure_connect_to_server(char *hostname, char *port, char *user_port);
void secure_close(ssl_tuple *running_ssl, bool close_all);
void generate_SHA256_hash(char *tocrypt, char result[65]);

#endif /* networking_h */
