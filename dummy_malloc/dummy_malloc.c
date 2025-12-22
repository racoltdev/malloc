#define _GNU_SOURCE
#include "dummy_malloc.h"

#include <stddef.h> // size_t
#include <dlfcn.h>  // dlsym
#include <stdio.h>  // printf

void* malloc(size_t size) {
	void* (*libc_malloc)(size_t);
	void* (*libc_exit)(int);

	*(void **) (&libc_malloc) = dlsym(RTLD_NEXT, "malloc");
	*(void **) (&libc_exit) = dlsym(RTLD_NEXT, "exit");

	fprintf(stderr, "malloc(%zu)\n", size);
	void* address = libc_malloc(size);
	fprintf(stderr, "Address:%p\n", address);
	libc_exit(0);
	return address;
}
