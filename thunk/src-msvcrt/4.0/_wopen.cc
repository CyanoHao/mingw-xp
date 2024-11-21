#include "thunk-common.h"
#include "thunk-internal-os.h"
#include "thunk-internal-string.h"

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdarg.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(msvcrt,
	               0,
	               int,
	               __cdecl,
	               _wopen,
	               const wchar_t *filename,
	               int oflag,
	               ...)
	{
		int pmode = 0;
		if (oflag & _O_CREAT)
		{
			va_list args;
			va_start(args, oflag);
			pmode = va_arg(args, int);
			va_end(args);
		}

		if (internal::is_nt())
		{
			auto p_wopen = try_get__wopen();
			return p_wopen(filename, oflag, pmode);
		}

		auto aname = internal::narrow(filename);
		if (!aname.valid())
		{
			errno = ENOMEM;
			return -1;
		}
		return _open(aname.c_str(), oflag, pmode);
	}
} // namespace mingw_thunk
