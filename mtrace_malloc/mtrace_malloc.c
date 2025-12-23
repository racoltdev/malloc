#define _GNU_SOURCE
#include "mtrace_malloc.h"

#include <stddef.h>   // size_t, ptrdiff_t
#include <stdlib.h>   // alloca
#include <dlfcn.h>    // dlsym, Dl_info
#include <stdio.h>    // fprintf, sprintf
#include <string.h>   // strlen
#include <inttypes.h> // PRIxPTR

static FILE *mallstream;
static const char mallenv[] = "MALLOC_TRACE";

static void tr_where(const void* caller, Dl_info* info) {
	if (caller != NULL) {
		if (info != NULL) {
			char* buf = (char*)"";

			if (info->dli_sname != NULL) {
				size_t len = strlen (info->dli_sname);
				// Allocate buf in the stack
				buf = alloca(len + 6 + 2 * sizeof(void*));

				char sign;
				ptrdiff_t offset = (ptrdiff_t)info->dli_saddr - (ptrdiff_t)caller;

				if (caller >= (const void*)info->dli_saddr) {
					sign = '+';
					offset = -offset;
				} else {
					sign = '-';
				}

				sprintf(buf, "(%s%c%" PRIxPTR ")", info->dli_sname, sign, offset);
			}

			fprintf(mallstream, "@ %s%s%s[0x%" PRIxPTR "] ",
					info->dli_fname ? info->dli_fname : "",
					info->dli_fname ? ":" : "",
					buf,
					(uint8_t*)caller - (uint8_t*)info->dli_fbase
					// cast to 1 byte width pointer to avoid compiler warnings
					// See warn pointer arith gcc extension for details
			);
		}
		else {
			fprintf(mallstream, "@ [%p] ", caller);
		}
	}
}

static Dl_info* lock_and_info(const void* caller, Dl_info* mem) {
	if (caller == NULL) {
		return NULL;
	}

	Dl_info* res = dladdr(caller, mem) ? mem : NULL;
	flockfile(mallstream);
	return res;
}

void* malloc(size_t size) {
	static void* (*libc_malloc)(size_t) = NULL;
	if (!libc_malloc) {
		// This avoids a pedantic compiler warning about assigning from void
		*(void **) (&libc_malloc) = dlsym(RTLD_NEXT, "malloc");
	}

	fprintf(stderr, "malloc(%zu)", size);
	void* address = libc_malloc(size);
	fprintf(stderr, ": %p\n", address);
	return address;
}

void free(void* ptr) {
	static void (*libc_free)(void*) = NULL;
	if (!libc_free) {
		*(void **) (&libc_free) = dlsym(RTLD_NEXT, "free");
	}

	libc_free(ptr);
	return;
}

void* calloc(size_t nmemb, size_t size) {
	static void* (*libc_calloc)(size_t, size_t) = NULL;
	if (!libc_calloc) {
		*(void **) (&libc_calloc) = dlsym(RTLD_NEXT, "calloc");
	}

	return libc_calloc(nmemb, size);
}

void* realloc(void* ptr, size_t size) {
	static void* (*libc_realloc)(void* _Nullable, size_t) = NULL;
	if (!libc_realloc) {
		*(void **) (&libc_realloc) = dlsym(RTLD_NEXT, "realloc");
	}

	return libc_realloc(ptr, size);
}
