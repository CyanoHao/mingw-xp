#include "thunk-common.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <direct.h>
#include <errno.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(msvcrt, 0, int, __cdecl, _wmkdir, const wchar_t *dirname)
	{
		if (internal::is_nt())
		{
			auto p_wmkdir = try_get__wmkdir();
			return p_wmkdir(dirname);
		}

		auto aname = internal::narrow(dirname);
		if (!aname.valid())
		{
			errno = ENOMEM;
			return -1;
		}
		return _mkdir(aname.c_str());
	}
} // namespace mingw_thunk
