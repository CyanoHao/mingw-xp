#pragma once

#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace mingw_thunk::internal
{
	constexpr inline int __char_to_hex(wchar_t c)
	{
		if (c >= '0' && c <= '9')
			return c - '0';
		if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;
		if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		return -1;
	}

	constexpr inline unsigned __bchar_to_hex(const wchar_t *str)
	{
		return (__char_to_hex(str[0]) << 4) + __char_to_hex(str[1]);
	}

	constexpr inline unsigned __qchar_to_hex(const wchar_t *str)
	{
		return (__bchar_to_hex(str) << 8) + __bchar_to_hex(str + 2);
	}

	constexpr inline unsigned __ochar_to_hex(const wchar_t *str)
	{
		return (__qchar_to_hex(str) << 16) + __qchar_to_hex(str + 4);
	}

	constexpr inline bool __is_hex_char(wchar_t c)
	{
		return __char_to_hex(c) >= 0;
	}

	constexpr inline bool __is_hex_qchar(const wchar_t *str)
	{
		return __is_hex_char(str[0]) && __is_hex_char(str[1]) &&
		       __is_hex_char(str[2]) && __is_hex_char(str[3]);
	}

	inline bool StringToGuid(const wchar_t *str, GUID *guid)
	{
		*guid = {};
		if (!str)
			return false;

		if (str[0] == L'{')
			str++;

		if (!(__is_hex_qchar(str) && __is_hex_qchar(str + 4) &&
		      str[8] == L'-' && __is_hex_qchar(str + 9) && str[13] == L'-' &&
		      __is_hex_qchar(str + 14) && str[18] == L'-' &&
		      __is_hex_qchar(str + 19) && str[23] == L'-' &&
		      __is_hex_qchar(str + 24) && __is_hex_qchar(str + 28) &&
		      __is_hex_qchar(str + 32)))
			return false;

		guid->Data1 = __ochar_to_hex(str);
		guid->Data2 = __qchar_to_hex(str + 9);
		guid->Data3 = __qchar_to_hex(str + 14);
		guid->Data4[0] = __bchar_to_hex(str + 19);
		guid->Data4[1] = __bchar_to_hex(str + 21);
		guid->Data4[2] = __bchar_to_hex(str + 24);
		guid->Data4[3] = __bchar_to_hex(str + 26);
		guid->Data4[4] = __bchar_to_hex(str + 28);
		guid->Data4[5] = __bchar_to_hex(str + 30);
		guid->Data4[6] = __bchar_to_hex(str + 32);
		guid->Data4[7] = __bchar_to_hex(str + 34);
		return true;
	}
} // namespace mingw_thunk::internal
