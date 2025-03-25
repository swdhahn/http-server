#include "file_tree.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int read_file(const char* file_path, char* data, unsigned int* size) {
	int fd = open(file_path, 444, O_RDONLY);

	if (fd == -1) {
		printf("Error opening file: \n", errno);
		return -1;
	}

	if (data == NULL) {
		*size = lseek(fd, 0, SEEK_END);
		close(fd);
		return 1;
	}
	lseek(fd, 0, SEEK_SET);

	char buf[512];
	int i = 0;
	while (*size - i > 0) {
		int l = read(fd, buf, 512);
		memcpy(data + i, buf, l);
		i += l;
	}

	close(fd);

	return 0;
}

void write_file(const char* file_path, const char* data, unsigned int size) {}
