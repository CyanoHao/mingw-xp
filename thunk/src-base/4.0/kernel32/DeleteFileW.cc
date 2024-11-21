#include "thunk-common.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <windows.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(
	    kernel32, 4, BOOL, WINAPI, DeleteFileW, _In_ LPCWSTR lpFileName)
	{
		if (internal::is_nt())
		{
			auto pDeleteFileW = try_get_DeleteFileW();
			return pDeleteFileW(lpFileName);
		}

		auto aname = internal::narrow(lpFileName);
		if (!aname.valid())
		{
			SetLastError(ERROR_OUTOFMEMORY);
			return FALSE;
		}
		return DeleteFileA(aname.c_str());
	}
} // namespace mingw_thunk
