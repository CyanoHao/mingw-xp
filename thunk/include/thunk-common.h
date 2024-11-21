#pragma once

#ifdef USE_LIBSTDCXX_UCRT_FIX
#include "thunk-internal-libstdc++-ucrt-fix.h"
#endif

#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace mingw_thunk
{
	struct module_handle
	{
		HMODULE module;

		module_handle(const char *module_name) noexcept
		{
			module = LoadLibraryA(module_name);
		}

		module_handle(const module_handle &) = delete;

		~module_handle() noexcept
		{
			if (module)
				FreeLibrary(module);
		}

		template <typename Fn>
		Fn *get_function(const char *function_name) const noexcept
		{
			return module ? reinterpret_cast<Fn *>(
			                    GetProcAddress(module, function_name))
			              : nullptr;
		}
	};
} // namespace mingw_thunk

#ifdef _WIN64
#define __IMP_SYMBOL(function) __imp_##function
#define __MANGLED_NAME___stdcall(function, size) #function
#else
// x86 __cdecl:
// - should be like: `xxx` --mangled-> `_xxx` --prefixed-> `__imp__xxx`
// - our workaround: `xxx` --prefixed-> `_imp__xxx` --mangled-> `__imp__xxx`
// x86 __stdcall:
// - should be like: `xxx` --mangled-> `_xxx@n` --prefixed-> `__imp__xxx@n`
// - our workaround: `xxx` --prefixed-> `_imp__xxx` --mangled-> `__imp__xxx@n`
#define __IMP_SYMBOL(function) _imp__##function
#define __MANGLED_NAME___stdcall(function, size) #function "@" #size
#endif
#define __MANGLED_NAME___cdecl(function, size) #function
#define __MANGLED_NAME_WINAPI(function, size)                                  \
	__MANGLED_NAME___stdcall(function, size)
#define __MANGLED_NAME_WSAAPI(function, size)                                  \
	__MANGLED_NAME___stdcall(function, size)

#define __DEFINE_THUNK(                                                        \
    module, size, return_type, calling_convention, function, ...)              \
                                                                               \
	using fn_##function##_t = return_type calling_convention(__VA_ARGS__);     \
                                                                               \
	inline const auto &get_##module() noexcept                                 \
	{                                                                          \
		static module_handle handle{#module};                                  \
		return handle;                                                         \
	}                                                                          \
                                                                               \
	inline auto *try_get_##function() noexcept                                 \
	{                                                                          \
		static auto &module = get_##module();                                  \
		static auto *pfn = module.get_function<fn_##function##_t>(#function);  \
		return pfn;                                                            \
	}                                                                          \
                                                                               \
	extern "C" return_type calling_convention __IMP_SYMBOL(function)(          \
	    __VA_ARGS__)                                                           \
	    __attribute__((                                                        \
	        alias(__MANGLED_NAME_##calling_convention(function, size))));      \
                                                                               \
	extern "C" return_type calling_convention function(__VA_ARGS__)

#define __WarningMessage__(...)
