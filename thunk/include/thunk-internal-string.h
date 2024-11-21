#pragma once

#include "thunk-internal-memory.h"

namespace mingw_thunk::internal
{
	template <typename CharT>
	inline size_t _strlen(const CharT *str) noexcept
	{
		const CharT *end = str;
		while (*end)
			++end;
		return end - str;
	}

	template <typename CharT>
	struct basic_string_view
	{
	  public:
		static constexpr size_t npos = size_t(-1);

		constexpr basic_string_view() noexcept : _data(nullptr), _size(0) {}
		basic_string_view(const basic_string_view &) noexcept = default;
		basic_string_view &
		operator=(const basic_string_view &) noexcept = default;

		template <size_t N>
		constexpr basic_string_view(const CharT (&str)[N]) noexcept
		    : _data(str), _size(N - 1)
		{
		}

		constexpr basic_string_view(const CharT *data, size_t size) noexcept
		    : _data(data), _size(size)
		{
		}

		basic_string_view(const CharT *data) noexcept
		    : _data(data), _size(_strlen(data))
		{
		}

		constexpr const CharT *data() const noexcept { return _data; }
		constexpr size_t length() const noexcept { return _size; }
		constexpr size_t size() const noexcept { return _size; }

	  private:
		const CharT *_data;
		size_t _size;
	};

	using string_view = basic_string_view<char>;
	using wstring_view = basic_string_view<wchar_t>;

	template <typename CharT>
	struct basic_string
	{
		constexpr static size_t npos = size_t(-1);

		basic_string() noexcept
		    : _s{._is_long = false, ._size = 0, ._data = {0}}
		{
		}

		basic_string(size_t count, CharT ch) noexcept
		{
			if (count < _sso_size) {
				_set_short(count);
				for (size_t i = 0; i < count; ++i)
					_s._data[i] = ch;
			} else {
				const size_t capacity = _aligned_capacity(count);
				_l._is_long = true;
				_l._data = (CharT *)_win32_malloc(capacity * sizeof(CharT));
				if (!_l._data) {
					_l._capacity = 0;
					_l._size = npos;
					return;
				}
				_l._capacity = capacity;
				_l._size = count;
				for (size_t i = 0; i < count; ++i)
					_l._data[i] = ch;
				_l._data[count] = 0;
			}
		}

		basic_string(const CharT *str, size_t length = npos) noexcept
		{
			if (length == npos)
				length = _strlen(str);
			if (length < _sso_size) {
				_set_short(length);
				_memcpy(_s._data, str, length * sizeof(CharT));
			} else {
				const size_t capacity = _aligned_capacity(length);
				_l._is_long = true;
				_l._data = (CharT *)_win32_malloc(_l._capacity * sizeof(CharT));
				if (!_l._data) {
					_l._capacity = 0;
					_l._size = npos;
					return;
				}
				_l._capacity = capacity;
				_l._size = length;
				_memcpy(_l._data, str, length * sizeof(CharT));
				_l._data[length] = 0;
			}
		}

		template <size_t N>
		basic_string(const CharT (&str)[N]) noexcept : basic_string(str, N - 1)
		{
		}

		basic_string(const basic_string &other) = delete;
		basic_string &operator=(const basic_string &other) = delete;

		basic_string(basic_string &&other) noexcept : _r(other._r)
		{
			if (other._is_long())
				other._set_short();
		}

		basic_string &operator=(basic_string &&other) noexcept
		{
			// intentionally not check self assignment (results in empty string)
			if (_is_long())
				_win32_free(_l._data);
			_r = other._r;
			if (other._is_long())
				other._set_short();
			return *this;
		}

		~basic_string() noexcept
		{
			if (_is_long())
				_win32_free(_l._data);
		}

		static size_t _aligned_capacity(size_t length) noexcept
		{
			constexpr size_t alignment = sizeof(void *) / sizeof(CharT);
			size_t capacity = length + 1;
			if (capacity % alignment != 0)
				capacity += alignment - (capacity % alignment);
			return capacity;
		}

		const CharT *c_str() const noexcept
		{
			if (_is_long())
				return _l._data;
			else
				return _s._data;
		}

		size_t capacity() const noexcept
		{
			if (_is_long())
				return _l._capacity;
			else
				return _sso_size;
		}

		void clear() noexcept
		{
			if (_is_long())
				_win32_free(_l._data);
			_set_short();
		}

		CharT *data() noexcept
		{
			if (_is_long())
				return _l._data;
			else
				return _s._data;
		}

		size_t length() const noexcept { return size(); }

		void resize(size_t count) noexcept
		{
			if (count <= size()) {
				if (_is_long()) {
					_l._size = count;
					_l._data[count] = 0;
				} else {
					_s._size = count;
					_s._data[count] = 0;
				}
			} else {
				resize(count, CharT(0));
			}
		}

		void resize(size_t count, CharT ch) noexcept
		{
			if (count <= size()) {
				if (_is_long()) {
					_l._size = count;
					_l._data[count] = 0;
				} else {
					_s._size = count;
					_s._data[count] = 0;
				}
			} else {
				if (_is_long()) {
					if (count > _l._capacity) {
						size_t new_capacity = _aligned_capacity(count);
						CharT *new_data = (CharT *)_win32_realloc(
						    _l._data, new_capacity * sizeof(CharT));
						if (!new_data) {
							_win32_free(_l._data);
							_set_invalid();
							return;
						}
						_l._data = new_data;
						_l._capacity = new_capacity;
					}
					for (size_t i = _l._size; i < count; ++i)
						_l._data[i] = ch;
					_l._data[count] = 0;
					_l._size = count;
				} else {
					if (count < _sso_size) {
						for (size_t i = _s._size; i < count; ++i)
							_s._data[i] = ch;
						_s._data[count] = 0;
						_s._size = count;
					} else {
						size_t new_capacity = _aligned_capacity(count);
						CharT *new_data = (CharT *)_win32_malloc(new_capacity *
						                                         sizeof(CharT));
						if (!new_data) {
							_set_invalid();
							return;
						}
						_memcpy(new_data, _s._data, _s._size * sizeof(CharT));
						for (size_t i = _s._size; i < count; ++i)
							new_data[i] = ch;
						new_data[count] = 0;
						_l._is_long = true;
						_l._capacity = new_capacity;
						_l._size = count;
						_l._data = new_data;
					}
				}
			}
		}

		size_t size() const noexcept
		{
			if (_is_long())
				return _l._size;
			else
				return _s._size;
		}

		bool valid() noexcept
		{
			if (_l._is_long) {
				return _l._data != nullptr;
			} else {
				return true;
			}
		}

	  private:
		bool _is_long() const noexcept { return _l._is_long; }
		void _set_invalid() noexcept
		{
			_l._is_long = true;
			_l._capacity = 0;
			_l._size = npos;
			_l._data = nullptr;
		}
		void _set_short(size_t size = 0) noexcept
		{
			_s._is_long = false;
			_s._size = size;
			_s._data[size] = 0;
		}

	  private:
		struct _raw
		{
			size_t _words[3];
		};
		struct _long
		{
			size_t _is_long : 1;
			size_t _capacity : sizeof(size_t) * 8 - 1;
			size_t _size;
			CharT *_data;
		};
		static_assert(sizeof(_long) == sizeof(_raw));
		static constexpr size_t _sso_size = (sizeof(_raw) - 1) / sizeof(CharT);
		struct _short
		{
			unsigned char _is_long : 1;
			unsigned char _size : 7;
			char _padding[sizeof(CharT) - 1];
			CharT _data[_sso_size];
		};
		static_assert(sizeof(_short) == sizeof(_raw));

		union
		{
			_raw _r;
			_long _l;
			_short _s;
		};
	};

	using string = basic_string<char>;
	using wstring = basic_string<wchar_t>;

	inline wstring widen(const char *str, size_t length)
	{
		if (length == 0)
			return {};
		int requiredLength =
		    MultiByteToWideChar(CP_ACP, 0, str, length, nullptr, 0);
		wstring result(requiredLength, 0);
		if (!result.valid())
			return result;
		MultiByteToWideChar(
		    CP_ACP, 0, str, length, result.data(), requiredLength);
		return result;
	}

	inline wstring widen(const char *str) noexcept
	{
		size_t length = _strlen(str);
		return widen(str, length);
	}

	inline string narrow(const wchar_t *str, size_t length)
	{
		if (length == 0)
			return {};
		int requiredLength = WideCharToMultiByte(
		    CP_ACP, 0, str, length, nullptr, 0, nullptr, nullptr);
		string result(requiredLength, 0);
		if (!result.valid())
			return result;
		WideCharToMultiByte(CP_ACP,
		                    0,
		                    str,
		                    length,
		                    result.data(),
		                    requiredLength,
		                    nullptr,
		                    nullptr);
		return result;
	}

	inline string narrow(const wchar_t *str) noexcept
	{
		size_t length = _strlen(str);
		return narrow(str, length);
	}
} // namespace mingw_thunk::internal
