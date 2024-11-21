extern "C"
{
	int __cdecl at_quick_exit(void(__cdecl *)(void));
	void __cdecl __attribute__((__nothrow__)) quick_exit(int _Code)
	    __attribute__((__noreturn__));
}
