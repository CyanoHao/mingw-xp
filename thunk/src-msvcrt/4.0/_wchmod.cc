#include "thunk-common.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <errno.h>
#include <io.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(
	    msvcrt, 0, int, __cdecl, _wchmod, const wchar_t *filename, int pmode)
	{
		if (internal::is_nt())
		{
			auto p_wchmod = try_get__wchmod();
			p_wchmod(filename, pmode);
		}

		auto aname = internal::narrow(filename);
		if (!aname.valid())
		{
			errno = ENOMEM;
			return -1;
		}
		return _chmod(aname.c_str(), pmode);
	}
} // namespace mingw_thunk
