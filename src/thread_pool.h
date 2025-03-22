#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>

struct ThreadPool {
	unsigned int threadCount;
	pthread_t* threads;
};

ThreadPool* create_thread_pool(int threadCount);
void destroy_thread_pool(ThreadPool* pool);


#endif // !THREAD_POOL_H
