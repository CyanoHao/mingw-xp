#include "thunk-common.h"
#include "thunk-internal-memory.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <windows.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(kernel32,
	               16,
	               DWORD,
	               WINAPI,
	               GetFullPathNameW,
	               _In_ LPCWSTR lpFileName,
	               _In_ DWORD nBufferLength,
	               _Out_ LPWSTR lpBuffer,
	               _Out_opt_ LPWSTR *lpFilePart)
	{
		if (internal::is_nt()) {
			auto pGetFullPathNameW = try_get_GetFullPathNameW();
			pGetFullPathNameW(lpFileName, nBufferLength, lpBuffer, lpFilePart);
		}

		auto aname = internal::narrow(lpFileName);
		if (!aname.valid()) {
			SetLastError(ERROR_OUTOFMEMORY);
			return FALSE;
		}
		size_t abuflen = nBufferLength > MAX_PATH ? nBufferLength : MAX_PATH;
		internal::basic_string<char> abuf(abuflen, 0);
		char *afilepart = nullptr;
		DWORD alen =
		    GetFullPathNameA(aname.c_str(), abuflen, abuf.data(), &afilepart);
		if (alen == 0) {
			return FALSE;
		}
		if (alen > abuflen) {
			abuf.resize(alen);
			if (!abuf.valid()) {
				SetLastError(ERROR_OUTOFMEMORY);
				return FALSE;
			}
			alen = GetFullPathNameA(
			    aname.c_str(), abuflen, abuf.data(), &afilepart);
		}
		size_t aprefixlen = afilepart - abuf.data();
		abuf.resize(alen); // shrink

		auto wres = internal::widen(abuf.data(), alen);
		if (!wres.valid()) {
			SetLastError(ERROR_OUTOFMEMORY);
			return FALSE;
		}

		size_t len = wres.length() + 1;
		if (len > nBufferLength)
			return len;

		if (lpFilePart) {
			auto wprefix = internal::widen(abuf.data(), aprefixlen);
			if (!wprefix.valid()) {
				SetLastError(ERROR_OUTOFMEMORY);
				return FALSE;
			}
			*lpFilePart = lpBuffer + wprefix.length();
		}

		internal::_memcpy(lpBuffer, wres.data(), len * sizeof(wchar_t));
		return wres.length();
	}
} // namespace mingw_thunk
