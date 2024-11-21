#include "thunk-common.h"

#include <windows.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(kernel32,
	               12,
	               BOOL,
	               WINAPI,
	               CreateHardLinkW,
	               _In_ LPCWSTR lpFileName,
	               _In_ LPCWSTR lpExistingFileName,
	               _Reserved_ LPSECURITY_ATTRIBUTES lpSecurityAttributes)
	{
		if (auto pCreateHardLinkW = try_get_CreateHardLinkW())
		{
			return pCreateHardLinkW(
			    lpFileName, lpExistingFileName, lpSecurityAttributes);
		}

		SetLastError(ERROR_NOT_SUPPORTED);
		return FALSE;
	}
} // namespace mingw_thunk
