#include "thunk-common.h"

#include <windows.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(kernel32,
	               8,
	               DWORD,
	               WINAPI,
	               GetConsoleProcessList,
	               _Out_ LPDWORD lpdwProcessList,
	               _In_ DWORD dwProcessCount)
	{
		if (auto pGetConsoleProcessList = try_get_GetConsoleProcessList())
			return pGetConsoleProcessList(lpdwProcessList, dwProcessCount);

		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return 0;
	}
} // namespace mingw_thunk
