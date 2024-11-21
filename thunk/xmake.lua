add_rules('mode.release')
set_languages('c11', 'cxx17')

target('thunk-base')
	set_kind('static')
	set_exceptions('none')
	add_defines('_WIN32_WINNT=0x0f00')
	if is_arch('i386') then
		add_files(
			'src-base/4.0/**.cc',
			'src-base/5.0/**.cc',
			'src-base/5.1/**.cc')
	end
	add_includedirs('include')

target('thunk-msvcrt')
	set_kind('static')
	add_defines(
		'USE_LIBSTDCXX_UCRT_FIX',
		'_CRTBLD',
		'__CRT__NO_INLINE',
		'__MSVCRT_VERSION__=0x700',
		'__USE_MINGW_ANSI_STDIO=0')
	if is_arch('i386') then
		add_files('src-msvcrt/**.cc')
	end
	add_includedirs('include')
