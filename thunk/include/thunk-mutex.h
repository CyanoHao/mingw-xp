#pragma once

#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <synchapi.h>
#include <windows.h>

namespace mingw_thunk
{
	struct mutex
	{
		CRITICAL_SECTION cs;
		mutex()
		{
			InitializeCriticalSection(&cs);
		}
		~mutex()
		{
			DeleteCriticalSection(&cs);
		}
		void lock()
		{
			EnterCriticalSection(&cs);
		}
		void unlock()
		{
			LeaveCriticalSection(&cs);
		}
	};
} // namespace mingw_thunk
