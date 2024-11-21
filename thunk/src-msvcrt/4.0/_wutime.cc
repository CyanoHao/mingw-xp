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
	               _wutime,
	               const wchar_t *filename,
	               struct _utimbuf *times)
	{
		if (internal::is_nt())
		{
			auto p_wutime = try_get__wutime();
			return p_wutime(filename, times);
		}

		auto aname = internal::narrow(filename);
		if (!aname.valid())
		{
			errno = ENOMEM;
			return -1;
		}
		return _utime(aname.c_str(), times);
	}
} // namespace mingw_thunk
