#include "thunk-common.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <windows.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(kernel32,
	               20,
	               BOOL,
	               WINAPI,
	               WriteConsoleW,
	               _In_ HANDLE hConsoleOutput,
	               _In_ const VOID *lpBuffer,
	               _In_ DWORD nNumberOfCharsToWrite,
	               _Out_opt_ LPDWORD lpNumberOfCharsWritten,
	               _Reserved_ LPVOID lpReserved)
	{
		if (internal::is_nt())
		{
			auto pWriteConsoleW = try_get_WriteConsoleW();
			return pWriteConsoleW(hConsoleOutput,
			                      lpBuffer,
			                      nNumberOfCharsToWrite,
			                      lpNumberOfCharsWritten,
			                      lpReserved);
		}

		auto abuf =
		    internal::narrow((const wchar_t *)lpBuffer, nNumberOfCharsToWrite);
		if (!abuf.valid())
		{
			SetLastError(ERROR_OUTOFMEMORY);
			return FALSE;
		}
		DWORD awritten;
		int ok = WriteConsoleA(
		    hConsoleOutput, abuf.c_str(), abuf.length(), &awritten, lpReserved);
		if (lpNumberOfCharsWritten)
		{
			if (awritten == abuf.length())
				// fast path
				*lpNumberOfCharsWritten = nNumberOfCharsToWrite;
			else
			{
				auto wwritten = internal::widen(abuf.c_str(), awritten);
				if (wwritten.valid())
					*lpNumberOfCharsWritten = wwritten.length();
				else
				{
					SetLastError(ERROR_OUTOFMEMORY);
					*lpNumberOfCharsWritten = 0;
				}
			}
		}
		return ok;
	}
} // namespace mingw_thunk
