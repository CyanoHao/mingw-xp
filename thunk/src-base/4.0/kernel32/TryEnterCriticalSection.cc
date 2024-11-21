#include "thunk-common.h"
#include "thunk-internal-os.h"

#include <errhandlingapi.h>
#include <windows.h>

#include <atomic>
#include <cstdint>

namespace mingw_thunk
{
	// https://fanael.github.io/stockfish-on-windows-98.html
	namespace detail
	{
		struct critical_section
		{
			uint8_t type; // Always 4.
			struct critical_section_impl *impl;
			// Padding for NT compatibility, where the structure is completely
			// different.
			uint32_t reserved[4];
		};
		static_assert(sizeof(critical_section) == 24);

		struct critical_section_impl
		{
			uint8_t type;
			int recursion_count;
			void *owner_thread;
			uint32_t reserved;
			std::atomic<int>
			    lock_count; // Starts at 1 when not owned, decreasing.
			void *internal_pointers[3];
		};
		static_assert(sizeof(critical_section_impl) == 32);

		template <unsigned TdbxOffset>
		void *get_current_tdbx()
		{
			const char *tib;
			__asm__("mov {%%fs:0x18, %0|%0, fs:[0x18]}" : "=r"(tib));
			return *reinterpret_cast<void *const *>(tib + TdbxOffset);
		}

		template <unsigned TdbxOffset>
		BOOL TryEnterCriticalSection_9x(CRITICAL_SECTION *cs)
		{
			const auto actual = reinterpret_cast<critical_section *>(cs);
			if (actual->type != 4) [[unlikely]] {
				RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, 0);
				return FALSE;
			}

			critical_section_impl *impl = actual->impl;
			const auto current_tdbx = get_current_tdbx<TdbxOffset>();
			int actual_lock_count = 1;
			if (impl->lock_count.compare_exchange_strong(actual_lock_count,
			                                             0)) {
				impl->owner_thread = current_tdbx;
				++impl->recursion_count;
				return TRUE;
			} else if (impl->owner_thread == current_tdbx) {
				impl->lock_count.fetch_sub(1, std::memory_order_relaxed);
				++impl->recursion_count;
				return TRUE;
			} else {
				return FALSE;
			}
		}
	} // namespace detail

	__DEFINE_THUNK(kernel32,
	               4,
	               BOOL,
	               WINAPI,
	               TryEnterCriticalSection,
	               _Inout_ LPCRITICAL_SECTION lpCriticalSection)
	{
		if (internal::is_nt()) {
			auto pTryEnterCriticalSection = try_get_TryEnterCriticalSection();
			return TryEnterCriticalSection(lpCriticalSection);
		}

		// now 9x
		auto &osvi = internal::os_version();
		if (osvi.dwMinorVersion > 10)
			// ME
			return detail::TryEnterCriticalSection_9x<0x80>(lpCriticalSection);
		else
			// 98, 95
			return detail::TryEnterCriticalSection_9x<0x50>(lpCriticalSection);
	}
} // namespace mingw_thunk
