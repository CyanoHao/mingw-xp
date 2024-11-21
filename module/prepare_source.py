from hashlib import sha256
import logging
from packaging.version import Version
from pathlib import Path
from shutil import copyfile
import subprocess
from urllib.error import URLError
from urllib.request import urlopen

from module.checksum import CHECKSUMS
from module.path import ProjectPaths
from module.profile import BranchVersions, ProfileInfo

def _validate_and_download(path: Path, url: str):
  MAX_RETRY = 3
  checksum = CHECKSUMS[path.name]
  if path.exists():
    with open(path, 'rb') as f:
      body = f.read()
      if checksum != sha256(body).hexdigest():
        message = 'Validate fail: %s exists but checksum mismatch' % path.name
        logging.critical(message)
        logging.info('Please delete %s and try again' % path.name)
        raise Exception(message)
  else:
    logging.info('Downloading %s' % path.name)
    retry_count = 0
    while True:
      retry_count += 1
      try:
        response = urlopen(url)
        body = response.read()
        if checksum != sha256(body).hexdigest():
          message = 'Download fail: checksum mismatch for %s' % path.name
          logging.critical(message)
          raise Exception(message)
        with open(path, "wb") as f:
          f.write(body)
          return
      except URLError as e:
        message = 'Download fail: %s (retry %d/3)' % (e.reason, retry_count)
        if retry_count < MAX_RETRY:
          logging.warning(message)
          logging.warning('Retrying...')
        else:
          logging.critical(message)
          raise e

def _check_and_extract(path: Path, arx: Path):
  # check if already extracted
  if path.exists():
    mark = path / '.patched'
    if mark.exists():
      return False
    else:
      message = 'Extract fail: %s exists but not marked as fully patched' % path.name
      logging.critical(message)
      logging.info('Please delete %s and try again' % path.name)
      raise Exception(message)

  # extract
  res = subprocess.run([
    'bsdtar',
    '-xf',
    arx,
    '--no-same-owner',
  ], cwd = path.parent)
  if res.returncode != 0:
    message = 'Extract fail: bsdtar returned %d extracting %s' % (res.returncode, arx.name)
    logging.critical(message)
    raise Exception(message)

  return True

def _patch(path: Path, patch: Path):
  res = subprocess.run([
    'patch',
    '-Np1',
    '-i', patch,
  ], cwd = path)
  if res.returncode != 0:
    message = 'Patch fail: applying %s to %s' % (patch.name, path.name)
    logging.critical(message)
    raise Exception(message)

def _autoreconf(path: Path):
  res = subprocess.run([
    'autoreconf',
    '-fi',
  ], cwd = path)
  if res.returncode != 0:
    message = 'Autoreconf fail: %s' % path.name
    logging.critical(message)
    raise Exception(message)

def _automake(path: Path):
  res = subprocess.run([
    'automake',
  ], cwd = path)
  if res.returncode != 0:
    message = 'Automake fail: %s' % path.name
    logging.critical(message)
    raise Exception(message)

def _patch_done(path: Path):
  mark = path / '.patched'
  mark.touch()

def _binutils(ver: str, info: ProfileInfo, paths: ProjectPaths):
  url = f'https://ftpmirror.gnu.org/gnu/binutils/{paths.binutils_arx.name}'
  _validate_and_download(paths.binutils_arx, url)
  if _check_and_extract(paths.binutils, paths.binutils_arx):
    v = Version(ver)

    # Backport
    if v == Version('2.37'):
      _patch(paths.binutils, paths.patch / 'binutils' / 'backport_2.37.patch')
    elif v == Version('2.33.1'):
      _patch(paths.binutils, paths.patch / 'binutils' / 'backport_2.33.1.patch')

    # Fix path corruption
    if v >= Version('2.43'):
      _patch(paths.binutils, paths.patch / 'binutils' / 'fix-path-corruption_2.43.patch')
    elif v >= Version('2.41'):
      _patch(paths.binutils, paths.patch / 'binutils' / 'fix-path-corruption_2.41.patch')
    elif v >= Version('2.39'):
      _patch(paths.binutils, paths.patch / 'binutils' / 'fix-path-corruption_2.39.patch')

    # Ignore long path
    if info.host_winnt <= 0x03FF:
      _patch(paths.binutils, paths.patch / 'binutils' / 'ignore-long-path.patch')

    _patch_done(paths.binutils)

def _gcc(ver: str, info: ProfileInfo, paths: ProjectPaths):
  v = Version(ver)
  if v.major >= 15:
    url = f'https://gcc.gnu.org/pub/gcc/snapshots/{ver}/{paths.gcc_arx.name}'
  else:
    url = f'https://ftpmirror.gnu.org/gnu/gcc/gcc-{ver}/{paths.gcc_arx.name}'
  _validate_and_download(paths.gcc_arx, url)
  if _check_and_extract(paths.gcc, paths.gcc_arx):
    # Backport
    if v.major == 11:
      # - poisoned calloc when building with musl
      _patch(paths.gcc, paths.patch / 'gcc' / 'backport_11.patch')
    elif v.major == 10:
      # - mingw define standard PRI macros when building against musl
      _patch(paths.gcc, paths.patch / 'gcc' / 'backport_10.patch')
    elif v.major == 9:
      # - mingw define standard PRI macros when building against musl
      _patch(paths.gcc, paths.patch / 'gcc' / 'backport_9.patch')
    elif v.major == 8:
      # - gcc fails to find cc1plus if built against ucrt due to a behaviour of `_access`
      _patch(paths.gcc, paths.patch / 'gcc' / 'backport_8.patch')
    elif v.major == 7:
      # - someone declared `bool error_p = NULL`, it works until musl 1.2 defines NULL to nullptr
      _patch(paths.gcc, paths.patch / 'gcc' / 'backport_7.patch')
    elif v.major == 6:
      # - someone declared `bool error_p = NULL`, it works until musl 1.2 defines NULL to nullptr
      # - intl adds `-liconv` without proper libdir
      _patch(paths.gcc, paths.patch / 'gcc' / 'backport_6.patch')
    elif v.major == 5:
      # - someone declared `bool error_p = NULL`, it works until musl 1.2 defines NULL to nullptr
      _patch(paths.gcc, paths.patch / 'gcc' / 'backport_5.patch')

    # Fix make variable
    # - gcc 12 use `override CFLAGS +=` to handle PGO build, which breaks workaround for ucrt `access`
    if v.major >= 14:
      _patch(paths.gcc, paths.patch / 'gcc' / 'fix-make-variable_14.patch')
    elif v.major >= 12:
      _patch(paths.gcc, paths.patch / 'gcc' / 'fix-make-variable_12.patch')

    # Fix libatomic build
    if v >= Version('4.8') and v.major < 10:
      _patch(paths.gcc, paths.patch / 'gcc' / 'fix-libatomic-build.patch')

    # Fix VT sequence
    if v.major >= 12:
      _patch(paths.gcc, paths.patch / 'gcc' / 'fix-vt-seq_12.patch')
    elif v.major >= 8:
      _patch(paths.gcc, paths.patch / 'gcc' / 'fix-vt-seq_8.patch')

    # Fix locale directory
    if v.major >= 12:
      _patch(paths.gcc, paths.patch / 'gcc' / 'fix-localedir_12.patch')
    else:
      _patch(paths.gcc, paths.patch / 'gcc' / 'fix-localedir_4.8.patch')

    # Parser-friendly diagnostics
    po_dir = paths.gcc / 'gcc' / 'po'
    po_files = list(po_dir.glob('*.po'))
    res = subprocess.run([
      'sed',
      '-iE',
      '/^msgid "(error|warning): "/,+1 d',
      *po_files
    ])
    if res.returncode != 0:
      message = 'Patch fail: applying gcc parser-friendly diagnostics'
      logging.critical(message)
      raise Exception(message)

    if v.major >= 13:
      if info.host_winnt >= 0x0600:
        # Fix console code page
        _patch(paths.gcc, paths.patch / 'gcc' / 'fix-console-cp.patch')
      else:
        pass  # disable UTF-8 manifest later

    # Fix libbacktrace
    if v.major >= 15 and info.host_winnt <= 0x0500:
      _patch(paths.gcc, paths.patch / 'gcc' / 'fix-backtrace_nt50-15.patch')
      if info.host_winnt <= 0x0400:
        _patch(paths.gcc, paths.patch / 'gcc' / 'fix-backtrace_nt40-15.patch')

    # libstdc++ Win32 thunk
    if info.target_winnt <= 0x0400:
      copyfile(paths.patch / 'gcc' / 'win32-thunk.h', paths.gcc / 'libstdc++-v3' / 'libsupc++' / 'win32-thunk.h')
      if info.target_winnt == 0x0400:
        if v.major >= 9:
          _patch(paths.gcc, paths.patch / 'gcc' / 'libstdc++-win32-thunk_nt40.patch')
        else:
          # <filesystem> almost unimplemented for Windows, ignore it
          pass
      else:
        if v.major >= 15:
          _patch(paths.gcc, paths.patch / 'gcc' / 'libstdc++-win32-thunk_win9x-15.patch')
        else:
          _patch(paths.gcc, paths.patch / 'gcc' / 'libstdc++-win32-thunk_win9x-14.patch')

    _patch_done(paths.gcc)

def _gdb(ver: str, info: ProfileInfo, paths: ProjectPaths):
  url = f'https://ftpmirror.gnu.org/gnu/gdb/{paths.gdb_arx.name}'
  _validate_and_download(paths.gdb_arx, url)
  if _check_and_extract(paths.gdb, paths.gdb_arx):
    v = Version(ver)

    # Backport
    if v == Version('8.3.1'):
      _patch(paths.gdb, paths.patch / 'gdb' / 'backport_8.3.1.patch')

    # Fix thread
    if v.major >= 12 and info.host_winnt <= 0x0600:
      _patch(paths.gdb, paths.patch / 'gdb' / 'fix-thread.patch')

    if info.host_winnt <= 0x0500:
      copyfile(paths.patch / 'gdb' / 'win32-thunk.h', paths.gdb / 'gdb' / 'win32-thunk.h')
      # Kernel32 thunk
      if v.major >= 14:
        _patch(paths.gdb, paths.patch / 'gdb' / 'kernel32-thunk_14.patch')
      elif v.major >= 11:
        _patch(paths.gdb, paths.patch / 'gdb' / 'kernel32-thunk_11.patch')
      elif v.major >= 10:
        _patch(paths.gdb, paths.patch / 'gdb' / 'kernel32-thunk_10.patch')

      # IPv6 thunk
      if v.major >= 10:
        _patch(paths.gdb, paths.patch / 'gdb' / 'ipv6-thunk_10.patch')
      elif v >= Version('8.3'):
        _patch(paths.gdb, paths.patch / 'gdb' / 'ipv6-thunk_8.3.patch')

    # Fix VC6 CRT compatibility
    if info.host_winnt <= 0x0400:
      _patch(paths.gdb, paths.patch / 'gdb' / 'vc6-crt-compat.patch')

    _patch_done(paths.gdb)

def _gettext(ver: str, info: ProfileInfo, paths: ProjectPaths):
  url = f'https://ftpmirror.gnu.org/gnu/gettext/{paths.gettext_arx.name}'
  _validate_and_download(paths.gettext_arx, url)
  if _check_and_extract(paths.gettext, paths.gettext_arx):
    # Kernel32 thunk
    if info.host_winnt <= 0x03FF:
      copyfile(paths.patch / 'gettext' / 'win32-thunk.h', paths.gettext / 'gettext-runtime' / 'intl' / 'gnulib-lib' / 'win32-thunk.h')
      _patch(paths.gettext, paths.patch / 'gettext' / 'kernel32-thunk.patch')

    _patch_done(paths.gettext)

def _gmp(ver: str, info: ProfileInfo, paths: ProjectPaths):
  url = f'https://ftpmirror.gnu.org/gnu/gmp/{paths.gmp_arx.name}'
  _validate_and_download(paths.gmp_arx, url)
  _check_and_extract(paths.gmp, paths.gmp_arx)
  _patch_done(paths.gmp)

def _iconv(ver: str, info: ProfileInfo, paths: ProjectPaths):
  url = f'https://ftpmirror.gnu.org/gnu/libiconv/{paths.iconv_arx.name}'
  _validate_and_download(paths.iconv_arx, url)
  _check_and_extract(paths.iconv, paths.iconv_arx)
  _patch_done(paths.iconv)

def _make(ver: str, info: ProfileInfo, paths: ProjectPaths):
  url = f'https://ftpmirror.gnu.org/gnu/make/{paths.make_arx.name}'
  _validate_and_download(paths.make_arx, url)
  if _check_and_extract(paths.make, paths.make_arx):
    v = Version(ver)

    # Backport
    if v == Version('4.3'):
      _patch(paths.make, paths.patch / 'make' / 'backport_4.3.patch')

    # Fix fcntl declaration
    if v == Version('4.3'):
      _patch(paths.make, paths.patch / 'make' / 'fix-fcntl-decl.patch')

    _patch_done(paths.make)

def _mcfgthread(ver: str, info: ProfileInfo, paths: ProjectPaths):
  url = f'https://github.com/lhmouse/mcfgthread/archive/refs/tags/v{ver}.tar.gz'
  _validate_and_download(paths.mcfgthread_arx, url)
  _check_and_extract(paths.mcfgthread, paths.mcfgthread_arx)
  _patch_done(paths.mcfgthread)

def _mingw(ver: str, info: ProfileInfo, paths: ProjectPaths):
  url = f'https://downloads.sourceforge.net/project/mingw-w64/mingw-w64/mingw-w64-release/{paths.mingw_arx.name}'
  _validate_and_download(paths.mingw_arx, url)
  if _check_and_extract(paths.mingw, paths.mingw_arx):
    v = Version(ver)

    # CRT: Fix missing function
    if info.target_winnt <= 0x0502:
      if v.major >= 12:
        _patch(paths.mingw, paths.patch / 'crt' / 'fix-missing-function_12.patch')
      elif v.major >= 7:
        _patch(paths.mingw, paths.patch / 'crt' / 'fix-missing-function_7.patch')
      _autoreconf(paths.mingw / 'mingw-w64-crt')
      _automake(paths.mingw / 'mingw-w64-crt')

    # CRT: Allow skip space check in `ftruncate64`
    if info.host_winnt <= 0x0400:
      _patch(paths.mingw, paths.patch / 'crt' / 'ftruncate64-allow-skip-space-check.patch')

    # CRT: Use ANSI API
    if info.target_winnt <= 0x03FF:
      _patch(paths.mingw, paths.patch / 'crt' / 'use-ansi-api.patch')

    # winpthreads: Disable VEH
    if info.target_winnt <= 0x0500:
      if v.major >= 8:
        _patch(paths.mingw, paths.patch / 'winpthreads' / 'disable-veh_8.patch')
      elif v.major >= 5:
        _patch(paths.mingw, paths.patch / 'winpthreads' / 'disable-veh_5.patch')

    # winpthreads: Fix thread
    if info.target_winnt <= 0x03FF:
      _patch(paths.mingw, paths.patch / 'winpthreads' / 'fix-thread.patch')

    # winpthreads: Kernel32 thunk
    if info.target_winnt <= 0x03FF:
      copyfile(paths.patch / 'winpthreads' / 'win32-thunk.h', paths.mingw / 'mingw-w64-libraries' / 'winpthreads' / 'src' / 'win32-thunk.h')
      _patch(paths.mingw, paths.patch / 'winpthreads' / 'kernel32-thunk.patch')

    _patch_done(paths.mingw)

def _mpc(ver: str, info: ProfileInfo, paths: ProjectPaths):
  url = f'https://ftpmirror.gnu.org/gnu/mpc/{paths.mpc_arx.name}'
  _validate_and_download(paths.mpc_arx, url)
  _check_and_extract(paths.mpc, paths.mpc_arx)
  _patch_done(paths.mpc)

def _mpfr(ver: str, info: ProfileInfo, paths: ProjectPaths):
  url = f'https://ftpmirror.gnu.org/gnu/mpfr/{paths.mpfr_arx.name}'
  _validate_and_download(paths.mpfr_arx, url)
  _check_and_extract(paths.mpfr, paths.mpfr_arx)
  _patch_done(paths.mpfr)

def download_and_patch(ver: BranchVersions, paths: ProjectPaths, info: ProfileInfo):
  _binutils(ver.binutils, info, paths)
  _gcc(ver.gcc, info, paths)
  _gdb(ver.gdb, info, paths)
  if ver.gettext:
    _gettext(ver.gettext, info, paths)
  _gmp(ver.gmp, info, paths)
  _iconv(ver.iconv, info, paths)
  _make(ver.make, info, paths)
  if ver.mcfgthread:
    _mcfgthread(ver.mcfgthread, info, paths)
  _mingw(ver.mingw, info, paths)
  _mpc(ver.mpc, info, paths)
  _mpfr(ver.mpfr, info, paths)
