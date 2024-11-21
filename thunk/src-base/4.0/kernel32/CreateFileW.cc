#include "thunk-common.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <windows.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(kernel32,
	               28,
	               HANDLE,
	               WINAPI,
	               CreateFileW,
	               _In_ LPCWSTR lpFileName,
	               _In_ DWORD dwDesiredAccess,
	               _In_ DWORD dwShareMode,
	               _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	               _In_ DWORD dwCreationDisposition,
	               _In_ DWORD dwFlagsAndAttributes,
	               _In_opt_ HANDLE hTemplateFile)
	{
		if (internal::is_nt())
		{
			auto pCreateFileW = try_get_CreateFileW();
			return pCreateFileW(lpFileName,
			                    dwDesiredAccess,
			                    dwShareMode,
			                    lpSecurityAttributes,
			                    dwCreationDisposition,
			                    dwFlagsAndAttributes,
			                    hTemplateFile);
		}

		// Windows 9x: `FILE_SHARE_DELETE` not supported
		DWORD mode = dwShareMode & ~FILE_SHARE_DELETE;

		auto aname = internal::narrow(lpFileName);
		if (!aname.valid())
		{
			SetLastError(ERROR_OUTOFMEMORY);
			return INVALID_HANDLE_VALUE;
		}
		return CreateFileA(aname.c_str(),
		                   dwDesiredAccess,
		                   mode,
		                   lpSecurityAttributes,
		                   dwCreationDisposition,
		                   dwFlagsAndAttributes,
		                   hTemplateFile);
	}

} // namespace mingw_thunk
