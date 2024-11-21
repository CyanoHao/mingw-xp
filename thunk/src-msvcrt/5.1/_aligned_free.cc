#include "thunk-common.h"

#include <malloc.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(msvcrt, 0, void, __cdecl, _aligned_free, void *memblock)
	{
		if (const auto p_aligned_free = try_get__aligned_free())
			return p_aligned_free(memblock);

		return __mingw_aligned_free(memblock);
	}
} // namespace mingw_thunk
