#pragma once

#include "thunk-mutex.h"

#include <stdint.h>

namespace mingw_thunk
{
	inline mutex rand_mutex;

	inline uint32_t rand32()
	{
		static uint64_t seed = GetTickCount64();
		seed = 6364136223846793005ULL * seed + 1; // musl's choice
		return seed >> 32;
	}
} // namespace mingw_thunk
