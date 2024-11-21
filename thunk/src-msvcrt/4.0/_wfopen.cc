#include "thunk-common.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <errno.h>
#include <stdio.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(msvcrt,
	               0,
	               FILE *,
	               __cdecl,
	               _wfopen,
	               const wchar_t *filename,
	               const wchar_t *mode)
	{
		if (internal::is_nt())
		{
			auto p_wfopen = try_get__wfopen();
			p_wfopen(filename, mode);
		}

		auto aname = internal::narrow(filename);
		auto amode = internal::narrow(mode);
		if (!aname.valid() || !amode.valid())
		{
			errno = ENOMEM;
			return nullptr;
		}
		return fopen(aname.c_str(), amode.c_str());
	}
} // namespace mingw_thunk
