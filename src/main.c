#include <signal.h>
#include <stdlib.h>

#include "http_manager.h"

#define PORT 8080
#define MAX_CLIENTS 10

void handle_exit(int code);

int i = 1;

void get_test_req(const char* req, char* response, size_t* response_size) {
	char buf[] =
		"HTTP/1.1 200 OK\nContent-Length: 16\nContent-Type: text/html; "
		"charset=utf-8\n\nHello World!0000";
	buf[strlen(buf) - 3] = '0' + i++;
	strcpy(response, buf);
	*response_size = strlen(buf) + 1;
}

struct http_server* server;

int main() {
	signal(SIGINT, handle_exit);

	server = initialize_server(PORT, MAX_CLIENTS);

	add_request_handle(server, get_test_req, "/", METHOD_GET);

	client_connection_loop(server);

	handle_exit(0);
}

void handle_exit(int code) {
	close_server(server);
	exit(code);
}
