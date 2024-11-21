#include "thunk-common.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <windows.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(
	    kernel32, 4, BOOL, WINAPI, RemoveDirectoryW, _In_ LPCWSTR lpPathName)
	{
		if (internal::is_nt())
		{
			auto pRemoveDirectoryW = try_get_RemoveDirectoryW();
			return pRemoveDirectoryW(lpPathName);
		}

		auto aname = internal::narrow(lpPathName);
		if (!aname.valid())
		{
			SetLastError(ERROR_OUTOFMEMORY);
			return FALSE;
		}
		return RemoveDirectoryA(aname.c_str());
	}
} // namespace mingw_thunk
