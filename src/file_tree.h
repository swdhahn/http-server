#ifndef FILE_TREE_H
#define FILE_TREE_H

/**
 * file_path: path to file
 * data: mem location to char* that you want to store file data
 * size: mem location to store size of file in bytes
 */
int read_file(const char* file_path, char* data, unsigned int* size);
void write_file(const char* file_path, const char* data, unsigned int size);

#endif	// !FILE_TREE_H
