// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
//
// Only compiled in when CHIAKI_ENABLE_TRACY is on. Overriding the scalar
// operator new/delete is enough -- the default array forms (operator
// new[]/delete[]) and the nothrow forms forward to these per the C++
// standard, so this covers essentially all C++-side heap allocation in the
// gui target (Qt objects, std:: containers, etc) without needing separate
// overloads for each form. This does NOT see allocations made inside
// linked third-party libraries (Qt's own pool allocators where used,
// ffmpeg, libplacebo, etc).
//
// IMPORTANT: every tracked free must have a matching tracked alloc, or
// Tracy terminates the profiling session. If that happens, it usually means
// something allocated outside these operators (e.g. malloc'd in chiaki-lib,
// or returned by a C library) is being freed with `delete` somewhere, or
// vice versa -- worth checking ownership at whatever boundary the address
// crossed.
#include <cstdlib>
#include <mutex>
#include <new>

#include <tracy/Tracy.hpp>

namespace
{
	std::mutex g_tracy_memory_lock;
}

void *operator new(std::size_t count)
{
	void *ptr = std::malloc(count);
	if(!ptr)
		throw std::bad_alloc();
	std::lock_guard<std::mutex> lock(g_tracy_memory_lock);
	TracyAlloc(ptr, count);
	return ptr;
}

void operator delete(void *ptr) noexcept
{
	if(ptr)
	{
		std::lock_guard<std::mutex> lock(g_tracy_memory_lock);
		TracyFree(ptr);
	}
	std::free(ptr);
}
