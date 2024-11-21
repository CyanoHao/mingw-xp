#include "thunk-common.h"

#include <windows.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(kernel32,
	               8,
	               UINT,
	               WINAPI,
	               GetSystemWow64DirectoryA,
	               _Out_ LPSTR lpBuffer,
	               _In_ UINT uSize)
	{
		if (auto pGetSystemWow64DirectoryA = try_get_GetSystemWow64DirectoryA())
			return pGetSystemWow64DirectoryA(lpBuffer, uSize);

		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return 0;
	}
} // namespace mingw_thunk
