#include "thunk-common.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <direct.h>
#include <errno.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(msvcrt, 0, int, __cdecl, _wchdir, const wchar_t *dirname)
	{
		if (internal::is_nt())
		{
			auto p_wchdir = try_get__wchdir();
			return p_wchdir(dirname);
		}

		auto aname = internal::narrow(dirname);
		if (!aname.valid())
		{
			errno = ENOMEM;
			return -1;
		}
		return _chdir(aname.c_str());
	}
} // namespace mingw_thunk
