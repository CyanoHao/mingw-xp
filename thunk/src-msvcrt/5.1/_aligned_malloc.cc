#include "thunk-common.h"

#include <malloc.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(msvcrt,
	               0,
	               void *,
	               __cdecl,
	               _aligned_malloc,
	               size_t size,
	               size_t alignment)
	{
		if (const auto p_aligned_malloc = try_get__aligned_malloc())
			return p_aligned_malloc(size, alignment);

		return __mingw_aligned_malloc(size, alignment);
	}
} // namespace mingw_thunk
