#include "http_manager.h"

#include <dirent.h>
#include <string.h>

void client_connection_loop(struct http_server* server) {
	while (1) {
		struct sockaddr_in cli;
		socklen_t len = sizeof(cli);

		int connfd = accept(server->sockfd, (struct sockaddr*)&cli, &len);

		if (connfd < 0) {
			printf("Server accept failed: %s\n", strerror(errno));
		} else {
			printf("Server accepted a client...\n");
		}
		// here we can add async
		client_connection(server, connfd, cli, len);
	}
}

void client_connection(struct http_server* server, int connfd,
					   struct sockaddr_in client, socklen_t client_length) {
	char read_buff[2048];
	char* write_buff = malloc(MAX_RESPONSE_SIZE + 1);
	// while (1) {
	bzero(read_buff, 2048);
	bzero(write_buff, MAX_RESPONSE_SIZE + 1);
	int ret = read(connfd, read_buff, sizeof(read_buff));

	if (ret > 0) {
		size_t response_size = 0;
		handle_request(server, read_buff, write_buff, &response_size);

		ret = write(connfd, write_buff, response_size);
		// printf("Response Size: %d\n%s\n\n", (int)response_size, write_buff);

		if (ret <= 0) {
			// break;
		}
	} else {
		// break;
	}
	//}
	// printf("Client Socket closed...\n");
	close(connfd);
	free(write_buff);
}

void parse_url(const char* request, int start_pos, char* url) {
	int i = start_pos;
	while (1) {
		if (request[i] == ' ') break;

		url[i - start_pos] = request[i];
		i++;
	}
	url[i - start_pos] = '\0';
}

void handle_request(struct http_server* server, const char* request,
					char* response, size_t* response_size) {
	int method = 0;
	if (strncmp(request, "GET", 3) == 0) {
		method = METHOD_GET;
	} else if (strncmp(request, "POST", 4) == 0) {
		method = METHOD_POST;
	}
	char url[MAX_URL_SIZE];
	parse_url(request, 4, url);
	printf("Request URL: %s\n", url);

	for (int i = 0; i < server->requests_count; i++) {
		if (server->requests[i].method == method &&
			strcmp(url, server->requests[i].url) == 0) {
			if (server->requests[i].response_size != 0) {
				strcpy(response, server->requests[i].response);
				*response_size = server->requests[i].response_size;
			} else {
				server->requests[i].req(request, response, response_size);
			}
			return;
		}
	}
	// TODO: Handle 404 error requests
	// send back 404 error page not found?
	char buf[] = "HTTP/1.1 404 Not Found\nContent-Length: 0";
	strcpy(response, buf);
	*response_size = sizeof(buf);
}

void add_request_handle_file(struct http_server* server, const char* file_path,
							 const char* url, unsigned int method) {
	switch (method) {
		case METHOD_GET:
		case METHOD_POST:
			break;
		default:
			printf(
				"You did not specify a supported method for add_request_handle "
				"method call\n");
	}
	char* extension = strrchr(file_path, '.');
	char content_type[40];
	if (strcmp(extension, ".html") == 0) {
		strcpy(content_type, "text/html");
	} else if (strcmp(extension, ".css") == 0) {
		strcpy(content_type, "text/css");
	} else if (strcmp(extension, ".js") == 0) {
		strcpy(content_type, "text/javascript");
	} else {
		printf("Unrecognized format: %s defaulting to html file type.\n",
			   file_path);
		strcpy(content_type, "text/html");
	}

	// Creating more space for this request
	// This is slow but will be the perfect amount of memory
	// Not ideal, but was faster to implement
	server->requests_count++;
	struct http_request_data* t_requests = malloc(
		server->requests_count *
		sizeof(struct http_request_data));	// Yes this is a memory leak,
											// however, this should stay alive
											// until the program closes. And
											// there are only a limited number
											// of files, so it should be fine.
	memcpy(t_requests, server->requests,
		   sizeof(struct http_request_data) * (server->requests_count - 1));
	t_requests[server->requests_count - 1].method = method;
	t_requests[server->requests_count - 1].req = NULL;
	strcpy(t_requests[server->requests_count - 1].url,
		   url);  // url is already allocated so just strcpy

	char buf[] =
		"HTTP/1.1 200 OK\nContent-Type: "
		"; charset=utf-8\nContent-Length: ";

	char* data = NULL;
	unsigned int size;
	read_file(file_path, NULL, &size);	// get size first
	data = malloc(strlen(buf) + strlen(content_type) +
				  (floor(log10((double)size)) + 1) + 2 +
				  size);  // len of buf + digits of size + 2 for \n and the size
						  // of the content
	sprintf(data, "%s%s%s%d\n\n",
			"HTTP/1.1 200 OK\nContent-Type: ", content_type,
			"; charset=utf-8\nContent-Length: ", size);
	read_file(file_path, data + strlen(data), &size);
	t_requests[server->requests_count - 1].response_size = strlen(data);
	t_requests[server->requests_count - 1].response = data;

	free(server->requests);
	server->requests = t_requests;
}

void add_request_handle(struct http_server* server, request_func func,
						const char* url, unsigned int method) {
	switch (method) {
		case METHOD_GET:
		case METHOD_POST:
			break;
		default:
			printf(
				"You did not specify a supported method for add_request_handle "
				"method call\n");
	}

	// Creating more space for this request
	// This is slow but will be the perfect amount of memory
	// Not ideal, but was faster to implement
	server->requests_count++;
	struct http_request_data* t_requests =
		malloc(server->requests_count * sizeof(struct http_request_data));
	memcpy(t_requests, server->requests, server->requests_count - 1);
	t_requests[server->requests_count - 1].method = method;
	t_requests[server->requests_count - 1].req = func;
	strcpy(t_requests[server->requests_count - 1].url,
		   url);  // url is already allocated so just strcpy
	t_requests[server->requests_count - 1].response_size = 0;
	t_requests[server->requests_count - 1].response = NULL;

	free(server->requests);
	server->requests = t_requests;
}

// TODO: Fix/finish
/**
 * Will cause race condition if used with different threads.
 *
 */
void load_server_files_from_root(struct http_server* server, const char* path) {
	static char* root_path = NULL;
	int root_path_flag = 0;

	if (root_path == NULL) {
		root_path_flag = 1;
		root_path = malloc(strlen(path));
		strcpy(root_path, path);
	}
	DIR* dir = opendir(path);
	errno = 0;

	struct dirent* d = readdir(dir);
	char c_url[MAX_URL_SIZE];
	strcpy(c_url, path);
	printf("Path: %s and %s\n", c_url, root_path);
	int len = strlen(path);
	int root_len = strlen(root_path);
	while (d != NULL) {
		strcpy(&c_url[len], d->d_name);
		struct stat statbuf;
		int v = stat(c_url, &statbuf);
		int ft_mode = statbuf.st_mode;

		if ((ft_mode & S_IFMT) == S_IFDIR) {
			if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) {
				// This is current and prev dir, we dont care for it
				d = readdir(dir);
				continue;
			}
			strcat(c_url, "/");
			load_server_files_from_root(server, c_url);
		} else if ((ft_mode & S_IFMT) ==
				   S_IFREG) {  // this handles regular files
			printf("Adding file request: %s on %s\n", c_url,
				   &c_url[root_len - 1]);
			add_request_handle_file(server, c_url, &c_url[root_len - 1],
									METHOD_GET);
		} else {  // prints a unsupported statement for non directories or
				  // regular files
			printf(
				"The directory contains something this program doesn't "
				"support: %s\n",
				c_url);
		}

		d = readdir(dir);
	}
	closedir(dir);
	printf("Error: %s\n", strerror(errno));

	if (root_path_flag) {
		free(root_path);
		root_path = NULL;
	}
}

struct http_server* initialize_server(unsigned short int port,
									  unsigned int maxClients) {
	struct http_server* server = malloc(sizeof(struct http_server));

	printf("Initializing Server\n");

	server->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int one = 1;
	setsockopt(server->sockfd, SOL_SOCKET, SO_REUSEADDR, &one, 1);
	setsockopt(server->sockfd, SOL_SOCKET, SO_REUSEPORT, &one, 1);

	if (server->sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	} else {
		printf("Socket successfully created..\n");
	}
	bzero(&server->servaddr, sizeof(struct sockaddr_in));

	server->servaddr.sin_family = AF_INET;
	server->servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	server->servaddr.sin_port = htons(port);

	if ((bind(server->sockfd, (struct sockaddr*)&server->servaddr,
			  sizeof(struct sockaddr_in))) != 0) {
		printf(
			"Socket binding failed. You may have to wait a bit for the "
			"OS...\n");
		exit(0);
	} else {
		printf("Socket successfully binded...\n");
	}

	// Now server is ready to listen and verification
	if ((listen(server->sockfd, maxClients)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	} else {
		printf("Server listening..\n");
	}

	return server;
}

void close_server(struct http_server* server) {
	// close all client connections right here!
	close(server->sockfd);	// TODO: make sure to fix this seg fault. Happens
							// when close socket while waiting on accept i think
	free(server);
}
