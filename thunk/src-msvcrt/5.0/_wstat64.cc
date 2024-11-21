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
	               _wstat64,
	               const wchar_t *path,
	               struct __stat64 *buffer)
	{
		if (auto p_wstat64 = try_get__wstat64())
			return p_wstat64(path, buffer);

		struct _stat32i64 buffer32;
		int ret = _wstati64(path, &buffer32);
		if (ret != 0) {
			*buffer = {};
			return ret;
		}
		buffer->st_dev = buffer32.st_dev;
		buffer->st_ino = buffer32.st_ino;
		buffer->st_mode = buffer32.st_mode;
		buffer->st_nlink = buffer32.st_nlink;
		buffer->st_uid = buffer32.st_uid;
		buffer->st_gid = buffer32.st_gid;
		buffer->st_rdev = buffer32.st_rdev;
		buffer->st_size = buffer32.st_size;
		buffer->st_atime = buffer32.st_atime;
		buffer->st_mtime = buffer32.st_mtime;
		buffer->st_ctime = buffer32.st_ctime;
		return ret;
	}
} // namespace mingw_thunk
