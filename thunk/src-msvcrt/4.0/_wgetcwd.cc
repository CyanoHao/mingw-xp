#include "thunk-common.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(
	    msvcrt, 0, wchar_t *, __cdecl, _wgetcwd, wchar_t *buffer, int maxlen)
	{
		if (internal::is_nt()) {
			auto p_wgetcwd = try_get__wgetcwd();
			return p_wgetcwd(buffer, maxlen);
		}

		char *abuf = _getcwd(nullptr, 0);
		if (!abuf)
			return nullptr;

		auto wres = internal::widen(abuf);
		// allocated by `_getcwd`
		free(abuf);
		if (!wres.valid()) {
			errno = ENOMEM;
			return nullptr;
		}
		ssize_t len = wres.length() + 1;
		if (buffer) {
			if (len > maxlen) {
				errno = ERANGE;
				return nullptr;
			}
		} else {
			// The `buffer` argument can be `NULL`; a buffer of at least size
			// `maxlen` (more only if necessary) is automatically allocated,
			// using `malloc`, to store the path.
			if (len < maxlen)
				len = maxlen;
			// will be freed by user
			buffer = (wchar_t *)malloc(len * sizeof(wchar_t));
			if (!buffer) {
				errno = ENOMEM;
				return nullptr;
			}
		}
		memcpy(buffer, wres.data(), len * sizeof(wchar_t));
		return buffer;
	}
} // namespace mingw_thunk
