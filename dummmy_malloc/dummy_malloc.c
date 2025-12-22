#include "dummy_malloc.h"

#include <stddef.h> // size_t
#include <dlfcn.h>  // dlsym
#include <stdio.h>  // fprintf

void* malloc(size_t size) {
	void* (*libc_malloc)(size_t) = (void* (*)(size_t))dlsym(RTLD_NEXT, "malloc");
	fprintf(stderr, "malloc(%zu)\n", size);
	void* address = libc_malloc(size);
	fprintf(stderr, "Address:%p)\n", address);
	void (*libc_exit)(int) = (void (*)(int))dlsym(RTLD_NEXT, "exit");
	libc_exit(0);
}
