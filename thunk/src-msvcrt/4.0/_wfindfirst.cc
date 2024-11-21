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
	               intptr_t,
	               __cdecl,
	               _wfindfirst,
	               const wchar_t *filespec,
	               struct _wfinddata_t *fileinfo)
	{
		if (internal::is_nt()) {
			auto p_wfindfirst = try_get__wfindfirst();
			return p_wfindfirst(filespec, fileinfo);
		}

		auto aspec = internal::narrow(filespec);
		if (!aspec.valid()) {
			errno = ENOMEM;
			return -1;
		}
		struct _finddata_t ainfo;
		auto res = _findfirst(aspec.c_str(), &ainfo);
		if (res == -1)
			return -1;
		auto winfo = internal::widen(ainfo);
		if (!winfo.name[0]) {
			_findclose(res);
			return -1;
		}
		*fileinfo = winfo;
		return res;
	}
} // namespace mingw_thunk
