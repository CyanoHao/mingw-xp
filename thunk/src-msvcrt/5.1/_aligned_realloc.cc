#include "thunk-common.h"

#include <malloc.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(msvcrt,
	               0,
	               void *,
	               __cdecl,
	               _aligned_realloc,
	               void *memblock,
	               size_t size,
	               size_t alignment)
	{
		if (const auto p_aligned_realloc = try_get__aligned_realloc())
			return p_aligned_realloc(memblock, size, alignment);

		return __mingw_aligned_realloc(memblock, size, alignment);
	}
} // namespace mingw_thunk
