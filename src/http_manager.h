#ifndef HTTP_MANAGER_H
#define HTTP_MANAGER_H

#include <errno.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include "file_tree.h"

#define MAX_RESPONSE_SIZE \
	1024 * 1024	 // 1 MB, so we can only send 1 MB of data per request

#define METHOD_GET 100
#define METHOD_POST 101

typedef void (*request_func)(const char* request, char* response,
							 size_t* response_size);
struct http_request_data {
	request_func req;
	char url[2000];
	unsigned int method;
	unsigned int response_size;
	char* response;
};

struct http_server {
	int sockfd;					  // socket file descriptor
	struct sockaddr_in servaddr;  // server's address

	struct http_request_data* requests;

	unsigned int requests_count;
};

void client_connection_loop(struct http_server* server);
void client_connection(struct http_server* server, int connfd,
					   struct sockaddr_in client, socklen_t client_length);

void handle_request(struct http_server* server, const char* request,
					char* response, size_t* response_size);
void parse_url(const char* request, int start_pos, char* url);

void add_request_handle_file(struct http_server* server, const char* file_path,
							 const char* url, unsigned int method);
void add_request_handle(struct http_server* server, request_func func,
						const char* url, unsigned int method);
void load_server_files_from_root(const char* path);

struct http_server* initialize_server(unsigned short int port,
									  unsigned int maxClients);
void close_server(struct http_server* server);

#endif	// !HTTP_MANAGER_H
