/* mpmtrace library implementation
	This file is part of the mpmtrace_malloc library.

	This file is a modification of a work found within the GNU C
	Library. In particular, it is a modification of malloc's
	mtrace-impl.c made to work without malloc hooks.
	Modification date: Mar 21, 2026

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	The mtrace_malloc library is distributed in the hope that it will be
	useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with the mtrace_malloc library; if not, see
	<https://www.gnu.org/licenses/>.  */

/* This tracer assumes it is not tracing a program that uses pid namespaces,
ie the program does not include multiple containers.
See https://www.man7.org/linux/man-pages/man7/pid_namespaces.7.html
for more information */

#define _GNU_SOURCE
#include "mpmtrace_malloc.h"

#include <stddef.h>   // size_t, ptrdiff_t
#include <stdlib.h>   // alloca
#include <dlfcn.h>    // dlsym, Dl_info
#include <stdio.h>    // fprintf, sprintf
#include <string.h>   // strlen
#include <inttypes.h> // PRIxPTR
#include <fcntl.h>    // open
#include <sys/file.h> // flock
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>   // getpid
#include <pthread.h>    // pthread_mutex_t/lock/unlock/trylock/destroy

static const char mallenv[] = "MALLOC_TRACE";
static FILE* mallstream;
static intmax_t pid = 0;
static pthread_mutex_t trace_mutex = PTHREAD_MUTEX_INITIALIZER;

enum InitState {
	UNINITIALIZED, PARTIAL, INITIALIZED
};
static enum InitState init_state = UNINITIALIZED;

// If this turns out to be a problem, figure out how to port libc_freeres() from glibc-2.41/malloc/set-freeres.c
static void release_libc_mem(void) {
	if (mallstream != NULL) {
		//__libc_freeres();
	}
}

static void unlock(void) {
	// if using semaphores: if a thread still holds a semaphore, there are two options:
	//     LOCK_UN anyways while keeping the semaphore active (semaphore wait must occur before LOCK_EX)
	//         can juggle between these threads and other processes
	//         thread priority should be roughly FIFO priority
	//     hold the LOCK_EX until no semaphore waits remain:
	//         if semaphore wait before LOCK_EX, this has a chance to juggle between these threads and other processes, but not very likely
	//         if LOCK_EX before semaphore, this process dominates file access until threads are done
	// There should be no way for 2 threads to deadlock eachother with any arrangement unless something REALLY dumb is happening
	// Juggling should prevent any deadlocks between threads and other processes.
	int fd = fileno(mallstream);
	if (flock(fd, LOCK_UN) == -1) {
		int err = errno;
		fprintf(stderr, "flock: %d\n", err);
		exit(1);
	}
	pthread_mutex_unlock(&trace_mutex);
}

static void do_mtrace(void) {
	static int added_atexit_handler;
	char* mallfile;


	// Don't panic if called more than once
	if (mallstream != NULL || init_state > UNINITIALIZED) {
		return;
	}

	mallfile = secure_getenv(mallenv);
	if (mallfile != NULL) {
		init_state = PARTIAL;
		// fopen uses malloc. Careful state management is used to avoid infinite recursion
		// This works if mallfile exists at start and contains "= Start" on the first line
		// TODO process sync (mutex or sempahore) to determine if file is already opened by
		// another mtrace process would be better
		mallstream = fopen(mallfile != NULL ? mallfile : "/dev/null", "ace");
		if (mallstream != NULL) {
			// Be sure it doesn't malloc its own buffer!
			static char tracebuf[512];

			setvbuf(mallstream, tracebuf, _IOFBF, sizeof(tracebuf));
			//fprintf(mallstream, "= Start\n");
			if (!added_atexit_handler) {
				added_atexit_handler = 1;
				// TODO I need to dig into libc and figure out how to make a proper exit handler.
				//__cxa_atexit((void (*)(void *))release_libc_mem, NULL, __dso_handle);
			}
		}
		pid = (intmax_t) getpid();
		init_state = INITIALIZED;
	}
}

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

			const char* fname = info->dli_fname ? info->dli_fname: "";
			const char* index = fname;
			while (*index != '\0') {
				if (*index == '/' && *(index + 1) != '\0') {
					fname = index + 1;
				}
				index++;
			}

			fprintf(mallstream, "%" PRIdMAX " %s%s%s[0x%" PRIxPTR "] ",
					pid,
					fname ? fname : "",
					fname ? ":" : "",
					buf,
					(uint8_t*)caller - (uint8_t*)info->dli_fbase
					// cast to 1 byte width pointer to avoid compiler warnings
					// See warn pointer arith gcc extension for details
			);
		}
		else {
			// I don't think this should ever happen?
			// None of my data collection triggered this line
			fprintf(mallstream, "@ [%p] ", caller);
		}
	}
}

static Dl_info* lock_and_info(const void* caller, Dl_info* mem) {
	if (caller == NULL) {
		return NULL;
	}

	pthread_mutex_lock(&trace_mutex);

	// dladdr returns 0 on complete failure, but no specific err available with dlerror()
	Dl_info* res = dladdr(caller, mem) ? mem : NULL;
	int fd = fileno(mallstream);
	if (flock(fd, LOCK_EX) == -1) {
		int err = errno;
		fprintf(stderr, "mtrace_malloc encountered an error while attempting to gain a file lock with flock:\n%d", err);
		exit(1);
	}

	return res;
}

void* malloc(size_t size) {
	static void* (*libc_malloc)(size_t) = NULL;
	if (!libc_malloc) {
		// This avoids a pedantic compiler warning about assigning from void
		*(void **) (&libc_malloc) = dlsym(RTLD_NEXT, "malloc");

		if (!libc_malloc) {
			fprintf(stderr, "mtrace_malloc enountered an error while attempting to load libc malloc with dlsym:\n%s\n", dlerror());
			exit(EXIT_FAILURE);
		}
	}

	void* block = libc_malloc(size);
	int err = errno;

	if (init_state == UNINITIALIZED) {
		do_mtrace();
	}
	if (init_state == INITIALIZED) {
		// GCC builtin black magic. See glibc-2.41/include libc-symbols.h: #define RETURN_ADDRESS (nr) for details
		// Scans up the stack to figure out who called malloc
		// No information on failure handling available. /shrug
		const void* caller = __builtin_extract_return_addr(__builtin_return_address(0));

		Dl_info mem;
		Dl_info* info = lock_and_info(caller, &mem);

		tr_where(caller, info);
		/* We could be printing a NULL here; that's OK. */
		fprintf(mallstream, "+ %p %#lx\n", block, (unsigned long int) size);
		fflush(mallstream);

		unlock();
	}

	// to preserve malloc behavior as much as possible, ensure errno reflects the status
	// of libc_malloc when an error occurs there, and only display errno from other calls
	// in this function if malloc performed correctly.
	if (!block) {
		errno = err;
	}

	return block;
}

void free(void* ptr) {
	static void (*libc_free)(void*) = NULL;
	if (!libc_free) {
		*(void **) (&libc_free) = dlsym(RTLD_NEXT, "free");

		if (!libc_free) {
			fprintf(stderr, "mtrace_malloc enountered an error while attempting to load libc free with dlsym:\n%s\n", dlerror());
			exit(EXIT_FAILURE);
		}
	}

	// Does not alter errno
	libc_free(ptr);

	if (init_state == UNINITIALIZED) {
		do_mtrace();
	}
	if (init_state == INITIALIZED) {
		if (ptr == NULL) {
			return;
		}

		const void* caller = __builtin_extract_return_addr(__builtin_return_address(0));

		Dl_info mem;
		Dl_info *info = lock_and_info(caller, &mem);

		tr_where(caller, info);
		fprintf(mallstream, "- %p\n", ptr);
		fflush(mallstream);

		unlock();
	}
	return;
}

void* calloc(size_t nmemb, size_t size) {
	static void* (*libc_calloc)(size_t, size_t) = NULL;
	if (!libc_calloc) {
		*(void **) (&libc_calloc) = dlsym(RTLD_NEXT, "calloc");

		if (!libc_calloc) {
			fprintf(stderr, "mtrace_malloc enountered an error while attempting to load libc calloc with dlsym:\n%s\n", dlerror());
			exit(EXIT_FAILURE);
		}
	}

	void* block = libc_calloc(nmemb, size);
	int err = errno;

	if (init_state == UNINITIALIZED) {
		do_mtrace();
	}
	if (init_state == INITIALIZED) {
		// GCC builtin black magic. See glibc-2.41/include libc-symbols.h: #define RETURN_ADDRESS (nr) for details
		const void* caller = __builtin_extract_return_addr(__builtin_return_address(0));

		Dl_info mem;
		Dl_info* info = lock_and_info(caller, &mem);

		tr_where(caller, info);
		/* We could be printing a NULL here; that's OK. */
		fprintf(mallstream, "+ %p %#lx\n", block, (unsigned long int) size);
		fflush(mallstream);

		unlock();
	}

	if (!block) {
		errno = err;
	}

	return block;
}

void* realloc(void* ptr, size_t size) {
	static void* (*libc_realloc)(void*, size_t) = NULL;
	if (!libc_realloc) {
		*(void **) (&libc_realloc) = dlsym(RTLD_NEXT, "realloc");

		if (!libc_realloc) {
			fprintf(stderr, "mtrace_malloc enountered an error while attempting to load libc realloc with dlsym:\n%s\n", dlerror());
			exit(EXIT_FAILURE);
		}
	}

	void* block = libc_realloc(ptr, size);
	int err = errno;

	if (init_state == UNINITIALIZED) {
		do_mtrace();
	}
	if (init_state == INITIALIZED) {
		const void* caller = __builtin_extract_return_addr(__builtin_return_address(0));

		Dl_info mem;
		Dl_info* info = lock_and_info(caller, &mem);

		tr_where(caller, info);
		if (block == NULL) {
			if (size != 0) {
				/* Failed realloc. */
				/* We could be printing a NULL here; that's OK. */
				fprintf(mallstream, "+ %p %#lx\n", block, (unsigned long int) size);
			} else {
				fprintf(mallstream, "- %p\n", ptr);
			}
		} else if (ptr == NULL) {
			fprintf(mallstream, "+ %p %#lx\n", block, (unsigned long int) size);
		} else {
			fprintf(mallstream, "< %p\n", ptr);
			tr_where(caller, info);
			fprintf(mallstream, "> %p %#lx\n", block, (unsigned long int) size);
		}
		fflush(mallstream);

		unlock();
	}

	if (!block) {
		errno = err;
	}

	return block;
}

void* memalign(size_t alignment, size_t size) {
	static void* (*libc_memalign)(size_t, size_t) = NULL;
	if (!libc_memalign) {
		*(void**) (&libc_memalign) = dlsym(RTLD_NEXT, "memalign");

		if (!libc_memalign) {
			fprintf(stderr, "mtrace_malloc enountered an error while attempting to load libc memalign with dlsym:\n%s\n", dlerror());
			exit(EXIT_FAILURE);
		}
	}

	void* block = libc_memalign(alignment, size);
	int err = errno;

	if (init_state == UNINITIALIZED) {
		do_mtrace();
	}
	if (init_state == INITIALIZED) {
		const void* caller = __builtin_extract_return_addr(__builtin_return_address(0));

		Dl_info mem;
		Dl_info* info = lock_and_info(caller, &mem);

		tr_where(caller, info);
		fprintf(mallstream, "+ %p %#lx\n", block, (unsigned long int) size);
		fflush(mallstream);

		unlock();
	}

	if (!block) {
		errno = err;
	}

	return block;
}
