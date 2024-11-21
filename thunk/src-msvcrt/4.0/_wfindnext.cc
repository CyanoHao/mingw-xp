#include "thunk-common.h"
#include "thunk-internal-dirent.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <errno.h>
#include <io.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(msvcrt,
	               0,
	               int,
	               __cdecl,
	               _wfindnext,
	               intptr_t handle,
	               struct _wfinddata_t *fileinfo)
	{
		if (internal::is_nt()) {
			auto p_wfindnext = try_get__wfindnext();
			return p_wfindnext(handle, fileinfo);
		}

		struct _finddata_t ainfo;
		auto res = _findnext(handle, &ainfo);
		if (res < 0)
			return res;
		auto winfo = internal::widen(ainfo);
		if (!winfo.name[0])
			return -1;
		*fileinfo = winfo;
		return res;
	}
} // namespace mingw_thunk
