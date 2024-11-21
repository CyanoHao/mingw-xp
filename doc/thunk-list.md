# The Thunk List

Flags:
- Q: quirk (succeed, but not work as correct as possible).
- Y: from YY-Thunks.

## Windows NT Wide API

### `kernel32`

| Function | Fallback | Flags |
| -------- | -------- | ----- |
| CreateFileW | Unset `FILE_SHARE_DELETE` (which is not supported by 9x) and call `CreateFileA`. | |
| DeleteFileW | Call `DeleteFileA`. | |
| GetFileAttributesW | Call `GetFileAttributesA`. | |
| GetFullPathNameW | Call `GetFullPathNameA`. | |
| MoveFileExW | Try `MoveFileA` then `CopyFileA` (`MoveFileExA` is a stub on 9x). | |
| RemoveDirectoryW | Call `RemoveDirectoryA`. | |
| WriteConsoleW | Call `WriteConsoleA`. | |

### `msvcrt`

| Function | Fallback | Flags |
| -------- | -------- | ----- |
| _wchdir | Call `_chdir`. | |
| _wchmod | Call `_chmod`. | |
| _wfindfirst | Call `_findfirst`. | |
| _wfindnext | Call `_findnext`. | |
| _wfopen | Call `_fopen`. | |
| _wgetcwd | Call `_getcwd`. | |
| _wmkdir | Call `_mkdir`. | |
| _wopen | Call `_open`. | |
| _wutime | Call `_utime`. | |

## Windows NT 4.0

### `kernel32`

| Function | Fallback | Flags |
| -------- | -------- | ----- |
| GetHandleInformation | Pretent to succeed (required by winpthreads). | :Q: |
| TryEnterCriticalSection | Use undocumented structure. | |

## Windows NT 5.0 (Windows 2000)

### `kernel32`

| Function | Fallback | Flags |
| -------- | -------- | ----- |
| CreateHardLinkW | Fail with `ERROR_NOT_SUPPORTED`. | |

### `msvcrt`

| Function | Fallback | Flags |
| -------- | -------- | ----- |
| _wstat64 | Call `_wstati64` (which may call `_stati64`). | |
| _wutime64 | Call `_wutime` (which may call `_utime`). | |

## Windows NT 5.1 (Windows XP)

### `kernel32`

| Function | Fallback | Flags |
| -------- | -------- | ----- |
| GetConsoleProcessList | Fail with `ERROR_CALL_NOT_IMPLEMENTED`. | |
| GetSystemWow64DirectoryA | Fail with `ERROR_CALL_NOT_IMPLEMENTED`. | |

### `msvcrt`

| Function | Fallback | Flags |
| -------- | -------- | ----- |
| _aligned_free | Call `__mingw_aligned_free`. | |
| _aligned_malloc | Call `__mingw_aligned_malloc`. | |
| _aligned_realloc | Call `__mingw_aligned_realloc`. | |

### `ws2_32`

| Function | Fallback | Flags |
| -------- | -------- | ----- |
| freeaddrinfo | Internal implementation. | |
