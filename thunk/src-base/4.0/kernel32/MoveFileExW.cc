#include "thunk-common.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <windows.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(kernel32,
	               12,
	               BOOL,
	               WINAPI,
	               MoveFileExW,
	               _In_ LPCWSTR lpExistingFileName,
	               _In_opt_ LPCWSTR lpNewFileName,
	               _In_ DWORD dwFlags)
	{
		if (internal::is_nt())
		{
			auto pMoveFileExW = try_get_MoveFileExW();
			pMoveFileExW(lpExistingFileName, lpNewFileName, dwFlags);
		}

		// Windows 9x: `MoveFileExA` is a stub
		auto aexist = internal::narrow(lpExistingFileName);
		auto anew = internal::narrow(lpNewFileName);
		if (!aexist.valid() || !anew.valid())
		{
			SetLastError(ERROR_OUTOFMEMORY);
			return FALSE;
		}
		int ok = MoveFileA(aexist.c_str(), anew.c_str());
		if (!ok &&
		    (dwFlags & (MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)))
		{
			bool replace = dwFlags & MOVEFILE_REPLACE_EXISTING;
			ok = CopyFileA(aexist.c_str(), anew.c_str(), !replace);
			if (ok)
				DeleteFileA(aexist.c_str());
		}
		return ok;
	}
} // namespace mingw_thunk
