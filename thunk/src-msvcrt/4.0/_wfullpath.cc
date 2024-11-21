#include "thunk-common.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(msvcrt,
	               0,
	               wchar_t *,
	               __cdecl,
	               _wfullpath,
	               wchar_t *absPath,
	               const wchar_t *relPath,
	               size_t maxLength)
	{
		if (internal::is_nt()) {
			auto p_wfullpath = try_get__wfullpath();
			return p_wfullpath(absPath, relPath, maxLength);
		}

		auto apath = internal::narrow(relPath);
		if (!apath.valid()) {
			errno = ENOMEM;
			return nullptr;
		}
		char abuf[MAX_PATH];
		if (!_fullpath(abuf, apath.c_str(), MAX_PATH))
			return nullptr;
		auto wres = internal::widen(abuf);
		if (!wres.valid()) {
			errno = ENOMEM;
			return nullptr;
		}
		ssize_t len = wres.length() + 1;
		if (absPath) {
			if (len > maxLength) {
				errno = ERANGE;
				return nullptr;
			}
		} else {
			// will be freed by user
			absPath = (wchar_t *)malloc(len * sizeof(wchar_t));
			if (!absPath) {
				errno = ENOMEM;
				return nullptr;
			}
		}
		memcpy(absPath, wres.data(), len * sizeof(wchar_t));
		return absPath;
	}
} // namespace mingw_thunk
