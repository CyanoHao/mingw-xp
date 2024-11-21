#include "thunk-common.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <errno.h>
#include <sys/utime.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(msvcrt,
	               0,
	               int,
	               __cdecl,
	               _wutime64,
	               const wchar_t *filename,
	               struct __utimbuf64 *times)
	{
		if (auto p_wutime64 = try_get__wutime64()) {
			return p_wutime64(filename, times);
		}

		struct _utimbuf times32;
		int ret = _wutime(filename, &times32);
		if (ret != 0) {
			*times = {};
			return ret;
		}
		times->actime = times32.actime;
		times->modtime = times32.modtime;
		return ret;
	}
} // namespace mingw_thunk
