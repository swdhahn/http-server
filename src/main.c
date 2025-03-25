#include <signal.h>
#include <stdlib.h>

#include "http_manager.h"

#define PORT 8080
#define MAX_CLIENTS 10

// dirent.h for searching through directories

void handle_exit(int code);

struct http_server* server;

int main() {
	// load_server_files_from_root("src/");

	signal(SIGINT, handle_exit);

	server = initialize_server(PORT, MAX_CLIENTS);

	// add_request_handle(server, get_test_req, "/", METHOD_GET);
	add_request_handle_file(server, "index.txt", "/", METHOD_GET);
	add_request_handle_file(server, "test.txt", "/test", METHOD_GET);

	client_connection_loop(server);

	handle_exit(0);
}

void handle_exit(int code) {
	close_server(server);
	exit(code);
}
