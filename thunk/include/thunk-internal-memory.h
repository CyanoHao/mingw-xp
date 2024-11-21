#pragma once

#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace std
{
	struct nothrow_t;
}

namespace mingw_thunk::internal
{
	inline void *_win32_malloc(size_t size) noexcept
	{
		return HeapAlloc(GetProcessHeap(), 0, size);
	}

	inline void *_win32_realloc(void *ptr, size_t size) noexcept
	{
		return HeapReAlloc(GetProcessHeap(), 0, ptr, size);
	}

	inline void _win32_free(void *ptr) noexcept
	{
		if (ptr)
			HeapFree(GetProcessHeap(), 0, ptr);
	}

	inline void *_memcpy(void *dest, const void *src, size_t size) noexcept
	{
		char *dest_char = static_cast<char *>(dest);
		const char *src_char = static_cast<const char *>(src);
		while (size) {
			--size;
			*dest_char++ = *src_char++;
		}
		return dest;
	}

	template <typename T, typename... Args>
	T *New(Args &&...args) noexcept
	{
		T *ptr = static_cast<T *>(_win32_malloc(sizeof(T)));
		if (ptr)
			new (ptr) T(static_cast<Args &&>(args)...);
		return ptr;
	}

	template <typename T>
	void Delete(T *ptr) noexcept
	{
		if (ptr) {
			ptr->~T();
			_win32_free(ptr);
		}
	}

	struct CppAlloc
	{
		void *operator new(size_t count) noexcept
		{
			return _win32_malloc(count);
		}
		void *operator new[](size_t count) noexcept
		{
			return _win32_malloc(count);
		}
		void operator delete(void *ptr) noexcept { _win32_free(ptr); }
		void operator delete[](void *ptr) noexcept { _win32_free(ptr); }

		void *operator new(size_t count, const std::nothrow_t &) noexcept
		{
			return _win32_malloc(count);
		}
		void *operator new[](size_t count, const std::nothrow_t &) noexcept
		{
			return _win32_malloc(count);
		}
		void operator delete(void *ptr, const std::nothrow_t &) noexcept
		{
			_win32_free(ptr);
		}
		void operator delete[](void *ptr, const std::nothrow_t &) noexcept
		{
			_win32_free(ptr);
		}

		void *operator new(size_t count, void *ptr) noexcept { return ptr; }
		void *operator new[](size_t count, void *ptr) noexcept { return ptr; }
		void operator delete(void *ptr, void *) noexcept {}
		void operator delete[](void *ptr, void *) noexcept {}
	};
} // namespace mingw_thunk::internal
