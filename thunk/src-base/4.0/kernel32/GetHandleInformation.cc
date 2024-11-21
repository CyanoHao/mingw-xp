#include "thunk-common.h"
#include "thunk-internal-os.h"

#include <errhandlingapi.h>
#include <windows.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(kernel32,
	               8,
	               BOOL,
	               WINAPI,
	               GetHandleInformation,
	               _In_ HANDLE hObject,
	               _Out_ LPDWORD lpdwFlags)
	{
		if (internal::is_nt()) {
			auto pGetHandleInformation = try_get_GetHandleInformation();
			return pGetHandleInformation(hObject, lpdwFlags);
		}

		if (!hObject) {
			SetLastError(ERROR_INVALID_HANDLE);
			return FALSE;
		}
		*lpdwFlags = 0;
		return TRUE;
	}
} // namespace mingw_thunk
