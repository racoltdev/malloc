#ifndef MTRACE_MALLOC_H
#define MTRACE_MALLOC_H

#include <stddef.h> //size_t

void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);
void* memalign(size_t alignment, size_t size);

#endif
