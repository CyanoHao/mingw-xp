#include "thunk-common.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <errno.h>
#include <sys/stat.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(msvcrt,
	               0,
	               int,
	               __cdecl,
	               _wstati64,
	               const wchar_t *path,
	               struct _stat32i64 *buffer)
	{
		if (internal::is_nt()) {
			auto p_wstati64 = try_get__wstati64();
			return p_wstati64(path, buffer);
		}

		auto apath = internal::narrow(path);
		if (!apath.valid()) {
			errno = ENOMEM;
			return -1;
		}
		return _stat32i64(apath.c_str(), buffer);
	}
} // namespace mingw_thunk
