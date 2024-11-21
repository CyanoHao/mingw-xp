# The Patch List

| Family | Patch | Applies to | Description |
| ------ | ----- | ---------- | ----------- |
| Binutils | Fix path corruption | ≥ 2.39 | [niXman/mingw-builds#649](https://github.com/niXman/mingw-builds/issues/649) |
| Binutils | Ignore long path | (H) 9x | Do not resolve filename to UNC path on Windows 9x. |
| CRT | Fix missing function | 2003- | The thunk for `_wassert` is marked as x86 only. `_wassert` is not implemented in Windows Server 2003 R2 x64 either. |
| CRT | Allow skip space check in `ftruncate64` | NT- | The space check calls find volume APIs. |
| CRT | Use ANSI API | 9x | MinGW-w64 uses `GetModuleHandleW` to load `msvcrt.dll` in thunks. |
| winpthreads | Disable VEH | ≥ 5, 2000- | MinGW-w64’s winpthreads use VEH (vectored exception handler, since Windows XP) to set thread name. |
| winpthreads | Fix thread | 9x | Fix invalid args calling `_beginthreadex` on Windows 9x. |
| GCC | Fix VT sequence | ≥ 8 | Fix VT sequence confliction with UTF-8 prefix. |
| GCC | Fix locale directory | Any | [niXman/mingw-builds#666](https://github.com/niXman/mingw-builds/issues/666) |
| GCC | Parser-friendly diagnostics | Any | Keep the phrases “error:” and “warning:” unlocalized to make diagnostic messages parser-friendly. |
| GCC | Fix console code page | ≥ 13, (H) Vista+ | GCC 13 introduced UTF-8 manifest for i18n support, making GCC’s ACP differ from console code page. |
| GCC | Fix libbacktrace | ≥ 15, (H) 2000- | 2000: missing library loader APIs. NT-: plus tool help APIs (actually not required on 9x). |
| GDB | Fix thread | (H) Vista- | Do not touch threads that are not suspended, which leads to crash on 2003/Vista x64. |
| GDB | IPv6 thunk | ≥ 8.3, (H) 2000- | Thunks for `getaddrinfo`, `getnameinfo`, `freeaddrinfo`. |

Note:

- Patches to fix build errors are not listed.
- “(H)” for HOST.
