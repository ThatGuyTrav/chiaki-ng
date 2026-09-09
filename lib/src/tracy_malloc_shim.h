// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
//
// Force-included (via -include) into every chiaki-lib translation unit when
// CHIAKI_ENABLE_TRACY is on, so malloc/calloc/realloc/free calls throughout
// the library are tracked for Tracy's Memory window without editing each
// call site individually. No-op entirely when TRACY_ENABLE isn't defined.
//
// This only covers allocations made inside chiaki-lib's own .c files -- it
// does NOT see allocations made inside linked third-party libraries (ffmpeg,
// opus, openssl, curl, etc), since those are already-compiled binaries this
// header never touches.
#ifndef CHIAKI_TRACY_MALLOC_SHIM_H
#define CHIAKI_TRACY_MALLOC_SHIM_H

#ifdef TRACY_ENABLE
#include <tracy/TracyC.h>
#include <stdlib.h>

static inline void *chiaki_tracy_malloc(size_t size)
{
	void *ptr = malloc(size);
	if(ptr)
		TracyCAlloc(ptr, size);
	return ptr;
}

static inline void *chiaki_tracy_calloc(size_t nmemb, size_t size)
{
	void *ptr = calloc(nmemb, size);
	if(ptr)
		TracyCAlloc(ptr, nmemb * size);
	return ptr;
}

// Tracy has no dedicated realloc event -- the manual's guidance is to model
// it as a free of the old pointer followed by an alloc of the new one,
// regardless of whether realloc happened to return the same address.
static inline void *chiaki_tracy_realloc(void *ptr, size_t size)
{
	if(ptr)
		TracyCFree(ptr);
	void *new_ptr = realloc(ptr, size);
	if(new_ptr)
		TracyCAlloc(new_ptr, size);
	return new_ptr;
}

static inline void chiaki_tracy_free(void *ptr)
{
	if(ptr)
		TracyCFree(ptr);
	free(ptr);
}

// Function-like macros only expand when followed by '(', so any code that
// takes a bare function pointer to malloc/free (none currently does, but
// safe either way) silently falls back to the untracked libc versions
// instead of failing to compile.
#define malloc(size) chiaki_tracy_malloc(size)
#define calloc(nmemb, size) chiaki_tracy_calloc(nmemb, size)
#define realloc(ptr, size) chiaki_tracy_realloc(ptr, size)
#define free(ptr) chiaki_tracy_free(ptr)

#endif // TRACY_ENABLE

#endif // CHIAKI_TRACY_MALLOC_SHIM_H
