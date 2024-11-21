#pragma once

#include <windows.h>
#include <synchapi.h>

static int Win32Thunk_Detail_IsNt()
{
  static int isNt = 0;
  static int isNtInit = 0;

  if (!isNtInit) {
    OSVERSIONINFOA osvi = { sizeof (OSVERSIONINFOA) };
    GetVersionExA (&osvi);
    isNt = osvi.dwPlatformId == VER_PLATFORM_WIN32_NT;
    isNtInit = 1;
  }

  return isNt;
}

// not work in Windows 9x
static BOOL Win32Thunk_GetHandleInformation(
  _In_ HANDLE hObject,
  _Out_ LPDWORD lpdwFlags
) {
  if (Win32Thunk_Detail_IsNt())
    return GetHandleInformation(hObject, lpdwFlags);

  // only return value matters
  return TRUE;
}

// Windows 98
static BOOL Win32Thunk_IsDebuggerPresent()
{
  typedef typeof (&IsDebuggerPresent) type;
  static type real = 0;
  static int realInit = 0;

  if (!realInit) {
    real = (type)GetProcAddress(GetModuleHandleA("kernel32.dll"), "IsDebuggerPresent");
    realInit = 1;
  }
  if (real)
    return real();

  return FALSE;
}

// Windows 98
static BOOL Win32Thunk_SetProcessAffinityMask(
  _In_ HANDLE hProcess,
  _In_ DWORD_PTR dwProcessAffinityMask
) {
  typedef typeof (&SetProcessAffinityMask) type;
  static type real = 0;
  static int realInit = 0;

  if (!realInit) {
    real = (type)GetProcAddress(GetModuleHandleA("kernel32.dll"), "SetProcessAffinityMask");
    realInit = 1;
  }
  if (real)
    return real(hProcess, dwProcessAffinityMask);

  SetLastError(ERROR_NOT_SUPPORTED);
  return FALSE;
}

#define GetHandleInformation Win32Thunk_GetHandleInformation
#define IsDebuggerPresent Win32Thunk_IsDebuggerPresent
#define SetProcessAffinityMask Win32Thunk_SetProcessAffinityMask
