/* mpmtrace library API
	This file is part of the mpmtrace_malloc library.

	This file is a modification of a work found within the GNU C
	Library. In particular, it is a modification of malloc's
	mtrace.c made to work without malloc hooks.
	Modification date: Jan 14, 2026

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

#ifndef MTRACE_MALLOC_H
#define MTRACE_MALLOC_H

#include <stddef.h> //size_t

void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);
void* memalign(size_t alignment, size_t size);

#endif
