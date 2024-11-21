#include "thunk-internal-string.h"

#include <dirent.h>
#include <errno.h>

namespace mingw_thunk::internal
{
	inline _finddata_t narrow(const _wfinddata_t &wdta)
	{
		_finddata_t adta;
		adta.attrib = wdta.attrib;
		adta.time_create = wdta.time_create;
		adta.time_access = wdta.time_access;
		adta.time_write = wdta.time_write;
		adta.size = wdta.size;
		string aname = narrow(wdta.name);
		if (!aname.valid()) {
			errno = ENOMEM;
			adta.name[0] = 0;
			return adta;
		}
		if (aname.length() >= MAX_PATH) {
			errno = ENAMETOOLONG;
			adta.name[0] = 0;
			return adta;
		}
		_memcpy(adta.name, aname.c_str(), aname.length() + 1);
		return adta;
	}

	inline _wfinddata_t widen(const _finddata_t &adta)
	{
		_wfinddata_t wdta;
		wdta.attrib = adta.attrib;
		wdta.time_create = adta.time_create;
		wdta.time_access = adta.time_access;
		wdta.time_write = adta.time_write;
		wdta.size = adta.size;
		wstring wname = widen(adta.name);
		if (!wname.valid()) {
			errno = ENOMEM;
			wdta.name[0] = 0;
			return wdta;
		}
		if (wname.length() >= MAX_PATH) {
			errno = ENAMETOOLONG;
			wdta.name[0] = 0;
			return wdta;
		}
		_memcpy(
		    wdta.name, wname.c_str(), (wname.length() + 1) * sizeof(wchar_t));
		return wdta;
	}
} // namespace mingw_thunk::internal
