#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace mingw_thunk::internal
{
	inline const OSVERSIONINFOA &os_version() noexcept
	{
		static OSVERSIONINFOA osvi = {sizeof(OSVERSIONINFOA)};
		if (!osvi.dwMajorVersion)
			GetVersionExA(&osvi);
		return osvi;
	}

	inline bool is_nt() noexcept
	{
		return os_version().dwPlatformId >= VER_PLATFORM_WIN32_NT;
	}
} // namespace mingw_thunk::internal
