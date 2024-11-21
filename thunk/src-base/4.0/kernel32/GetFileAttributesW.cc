#include "thunk-common.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <errhandlingapi.h>
#include <windows.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(
	    kernel32, 4, DWORD, WINAPI, GetFileAttributesW, _In_ LPCWSTR lpFileName)
	{
		if (internal::is_nt()) {
			auto pGetFileAttributesW = try_get_GetFileAttributesW();
			return pGetFileAttributesW(lpFileName);
		}

		auto aname = internal::narrow(lpFileName);
		if (!aname.valid()) {
			SetLastError(ERROR_OUTOFMEMORY);
			return INVALID_FILE_ATTRIBUTES;
		}
		return GetFileAttributesA(aname.c_str());
	}
} // namespace mingw_thunk
