import argparse
import logging
from packaging.version import Version
from pathlib import Path
from shutil import copyfile
import subprocess

from module.debug import shell_here
from module.path import ProjectPaths
from module.profile import BranchVersions, ProfileInfo
from module.util import cflags_build, cflags_host, cflags_target, configure, ensure, make_custom, make_default, make_install

def _binutils(ver: str, paths: ProjectPaths, info: ProfileInfo, jobs: int):
  build_dir = paths.binutils / 'build'
  ensure(build_dir)
  configure('binutils', build_dir, [
    f'--prefix={paths.prefix}',
    f'--host={info.target}',
    f'--target={info.target}',
    '--disable-nls',
    '--with-static-standard-libraries',
    *cflags_host(),
  ])
  make_default('binutils', build_dir, jobs)
  make_custom('binutils', build_dir, ['MAKEINFO=true', f'tooldir={paths.prefix}', 'install'], jobs = 1)

def _gmp(ver: str, paths: ProjectPaths, info: ProfileInfo, jobs: int):
  build_dir = paths.gmp / 'build'
  ensure(build_dir)
  configure('gmp', build_dir, [
    f'--prefix={paths.dep}',
    f'--host={info.target}',
    '--disable-assembly',
    '--enable-static',
    '--disable-shared',
    *cflags_host(c_extra = ['-std=gnu11']),
  ])
  make_default('gmp', build_dir, jobs)
  make_install('gmp', build_dir)

def _mpfr(ver: str, paths: ProjectPaths, info: ProfileInfo, jobs: int):
  build_dir = paths.mpfr / 'build'
  ensure(build_dir)
  configure('mpfr', build_dir, [
    f'--prefix={paths.dep}',
    f'--host={info.target}',
    f'--with-gmp={paths.dep}',
    '--enable-static',
    '--disable-shared',
    *cflags_host(),
  ])
  make_default('mpfr', build_dir, jobs)
  make_install('mpfr', build_dir)

def _mpc(ver: str, paths: ProjectPaths, info: ProfileInfo, jobs: int):
  build_dir = paths.mpc / 'build'
  ensure(build_dir)
  configure('mpc', build_dir, [
    f'--prefix={paths.dep}',
    f'--host={info.target}',
    f'--with-gmp={paths.dep}',
    f'--with-mpfr={paths.dep}',
    '--enable-static',
    '--disable-shared',
    *cflags_host(),
  ])
  make_default('mpc', build_dir, jobs)
  make_install('mpc', build_dir)

def _headers(ver: str, paths: ProjectPaths, info: ProfileInfo, jobs: int):
  build_dir = paths.mingw / 'mingw-w64-headers' / 'build'
  ensure(build_dir)
  configure('mingw-w64-headers', build_dir, [
    f'--prefix={paths.prefix}',
    f'--host={info.target}',
    f'--with-default-msvcrt={info.crt}',
    f'--with-default-win32-winnt=0x{max(info.target_winnt, 0x0400):04X}',
  ])
  make_default('headers', build_dir, jobs)
  make_install('headers', build_dir)

def _crt(ver: str, paths: ProjectPaths, info: ProfileInfo, jobs: int):
  build_dir = paths.mingw / 'mingw-w64-crt' / 'build'
  multilib_flags = [
    '--enable-lib64' if info.arch == '64' else '--disable-lib64',
    '--enable-libarm64' if info.arch == 'arm64' else '--disable-libarm64',
    '--enable-lib32' if info.arch == '32' else '--disable-lib32',
    '--disable-libarm32',
  ]
  ensure(build_dir)
  configure('mingw-w64-crt', build_dir, [
    f'--prefix={paths.prefix}',
    f'--with-sysroot={paths.prefix}',
    f'--host={info.target}',
    f'--with-default-msvcrt={info.crt}',
    f'--with-default-win32-winnt=0x{max(info.target_winnt, 0x0400):04X}',
    '--disable-dependency-tracking',
    *multilib_flags,
    *cflags_target(),
  ])
  make_default('crt', build_dir, jobs)
  make_install('crt', build_dir)

def _winpthreads(ver: str, paths: ProjectPaths, info: ProfileInfo, jobs: int):
  build_dir = paths.mingw / 'mingw-w64-libraries' / 'winpthreads' / 'build'
  ensure(build_dir)
  configure('winpthreads', build_dir, [
    f'--prefix={paths.prefix}',
    f'--with-sysroot={paths.prefix}',
    f'--host={info.target}',
    '--enable-static',
    '--disable-shared',
    *cflags_target(),
  ])
  make_default('winpthreads', build_dir, jobs)
  make_install('winpthreads', build_dir)

def _mcfgthread(ver: str, paths: ProjectPaths, info: ProfileInfo, jobs: int):
  build_dir = paths.mcfgthread / 'build'
  ensure(build_dir)

  ret = subprocess.run([
    'meson',
    'setup',
    '--cross-file', f'meson.cross.{info.target}',
    build_dir,
  ], cwd = paths.mcfgthread)
  if ret.returncode != 0:
    message = 'Build failed: mcfgthread meson setup returned %d' % ret.returncode
    logging.critical(message)
    raise Exception(message)
  ret = subprocess.run([
    'env',
    *cflags_host(),
    'meson',
    'compile',
    'mcfgthread:static_library',
  ], cwd = build_dir)
  if ret.returncode != 0:
    message = 'Build failed: mcfgthread ninja returned %d' % ret.returncode
    logging.critical(message)
    raise Exception(message)
  copyfile(build_dir / 'libmcfgthread.a', paths.prefix / 'lib' / 'libmcfgthread.a')

  include_dir = paths.prefix / 'include' / 'mcfgthread'
  ensure(include_dir)
  header_files = [
    *paths.mcfgthread.glob('mcfgthread/*.h'),
    *paths.mcfgthread.glob('mcfgthread/*.hpp'),
    build_dir / 'version.h',
  ]
  for header_file in header_files:
    copyfile(header_file, include_dir / header_file.name)

def _iconv(ver: str, paths: ProjectPaths, info: ProfileInfo, jobs: int):
  build_dir = paths.iconv / 'build'
  ensure(build_dir)
  configure('iconv', build_dir, [
    f'--prefix={paths.dep}',
    f'--host={info.target}',
    '--disable-nls',
    '--enable-static',
    '--disable-shared',
    *cflags_host(c_extra = ['-std=gnu11']),
  ])
  make_default('iconv', build_dir, jobs)
  make_install('iconv', build_dir)

def _gettext(ver: str, paths: ProjectPaths, info: ProfileInfo, jobs: int):
  build_dir = paths.gettext / 'gettext-runtime' / 'build'
  ensure(build_dir)
  configure('gettext', build_dir, [
    f'--prefix={paths.dep}',
    f'--host={info.target}',
    '--enable-static',
    '--disable-shared',
    *cflags_host(),
  ])
  make_default('gettext', build_dir, jobs)
  make_install('gettext', build_dir)

def _gcc(ver: str, paths: ProjectPaths, info: ProfileInfo, jobs: int):
  v = Version(ver)
  build_dir = paths.gcc / 'build'
  ensure(build_dir)
  # some old version checks existence of /mingw/include
  ensure(Path('/mingw/include'))

  if info.exception == 'dwarf':
    exception_flags = ['--disable-sjlj-exceptions', '--with-dwarf2']
  else:
    exception_flags = []
  if info.host_winnt <= 0x0502 and v.major >= 13:
    manifest_flags = ['--disable-win32-utf8-manifest']
  else:
    manifest_flags = []
  if v.major >= 14:
    libintl_flags = [f'--with-libintl-prefix={paths.dep}']
  else:
    libintl_flags = ['--with-included-gettext']

  configure('gcc', build_dir, [
    f'--prefix={paths.prefix}',
    f'--libexecdir={paths.prefix / 'lib'}',
    f'--with-gcc-major-version-only',
    f'--target={info.target}',
    f'--host={info.target}',
    # prefer static
    '--enable-lto',
    '--disable-plugin',
    '--disable-shared',
    '--enable-static',
    '--without-pic',
    # features
    '--enable-checking=release',
    '--disable-dependency-tracking',
    '--enable-languages=c,c++',
    '--enable-libgomp',
    '--enable-mingw-wildcard',
    '--disable-multilib',
    '--enable-nls',
    f'--enable-threads={info.thread}',
    '--disable-win32-registry',
    *exception_flags,
    *manifest_flags,
    # packages
    f'--with-gmp={paths.dep}',
    '--without-libcc1',
    f'--with-libiconv-prefix={paths.dep}',
    f'--with-mpc={paths.dep}',
    f'--with-mpfr={paths.dep}',
    *libintl_flags,
    *cflags_host(),
    *cflags_target('_FOR_TARGET'),
  ])
  make_default('gcc', build_dir, jobs)
  make_install('gcc', build_dir)

def _gdb(ver: str, paths: ProjectPaths, info: ProfileInfo, jobs: int):
  v = Version(ver)
  build_dir = paths.gdb / 'build'
  ensure(build_dir)

  if v.major >= 14:
    gmp_mpfr_flags = []
  else:
    gmp_mpfr_flags = [
      f'--with-libgmp-prefix={paths.dep}',
      f'--with-libmpfr-prefix={paths.dep}',
    ]

  configure('gdb', build_dir, [
    f'--prefix={paths.prefix}',
    f'--host={info.target}',
    # features
    '--disable-tui',
    # packages
    f'--with-gmp={paths.dep}',
    f'--with-mpc={paths.dep}',
    f'--with-mpfr={paths.dep}',
    *gmp_mpfr_flags,
    *cflags_host(common_extra = ['-DPDC_WIDE']),
  ])
  make_default('gdb', build_dir, jobs)
  make_install('gdb', build_dir)

def _gmake(ver: str, paths: ProjectPaths, info: ProfileInfo, jobs: int):
  build_dir = paths.make / 'build'
  ensure(build_dir)
  configure('make', build_dir, [
    f'--prefix={paths.prefix}',
    f'--host={info.target}',
    '--disable-nls',
    *cflags_host(),
  ])
  make_default('make', build_dir, jobs)
  copyfile(build_dir / 'make.exe', paths.prefix / 'bin' / 'mingw32-make.exe')

def _licenses(ver: BranchVersions, paths: ProjectPaths, info: ProfileInfo):
  license_dir = paths.prefix / 'share' / 'licenses'
  ensure(license_dir)

  ensure(license_dir / 'binutils')
  for file in ['README', 'COPYING', 'COPYING3', 'COPYING.LIB', 'COPYING3.LIB']:
    copyfile(paths.binutils / 'COPYING3', license_dir / 'binutils' / 'COPYING3')

  ensure(license_dir / 'gcc')
  for file in ['README', 'COPYING', 'COPYING3', 'COPYING.RUNTIME', 'COPYING.LIB', 'COPYING3.LIB']:
    copyfile(paths.gcc / file, license_dir / 'gcc' / file)

  ensure(license_dir / 'gdb')
  for file in ['README', 'COPYING', 'COPYING3', 'COPYING.LIB', 'COPYING3.LIB']:
    copyfile(paths.gdb / file, license_dir / 'gdb' / file)

  if Version(ver.gcc).major >= 14:
    ensure(license_dir / 'gettext-runtime-intl')
    copyfile(paths.gettext / 'gettext-runtime' / 'intl' / 'COPYING.LIB', license_dir / 'gettext-runtime-intl' / 'COPYING.LIB')

  ensure(license_dir / 'gmp')
  if Version(ver.gmp).major < 6:
    gmp_files = ['README', 'COPYING', 'COPYING.LIB']
  else:
    gmp_files = ['README', 'COPYINGv2', 'COPYINGv3', 'COPYING.LESSERv3']
  for file in gmp_files:
    copyfile(paths.gmp / file, license_dir / 'gmp' / file)

  ensure(license_dir / 'iconv')
  copyfile(paths.iconv / 'COPYING.LIB', license_dir / 'iconv' / 'COPYING.LIB')

  ensure(license_dir / 'make')
  copyfile(paths.make / 'COPYING', license_dir / 'make' / 'COPYING')

  if info.thread == 'mcf':
    ensure(license_dir / 'mcfgthread')
    copyfile(paths.mcfgthread / 'LICENSE.TXT', license_dir / 'mcfgthread' / 'LICENSE.TXT')
    for file in ['gcc-exception-3.1.txt', 'gpl-3.0.txt', 'lgpl-3.0.txt']:
      copyfile(paths.mcfgthread / 'licenses' / file, license_dir / 'mcfgthread' / file)

  ensure(license_dir / 'mingw-w64')
  copyfile(paths.mingw / 'COPYING', license_dir / 'mingw-w64' / 'COPYING')

  ensure(license_dir / 'mingw-w64-libraries-winpthreads')
  copyfile(paths.mingw / 'mingw-w64-libraries' / 'winpthreads' / 'COPYING', license_dir / 'mingw-w64-libraries-winpthreads' / 'COPYING')

  ensure(license_dir / 'mpc')
  copyfile(paths.mpc / 'COPYING.LESSER', license_dir / 'mpc' / 'COPYING.LESSER')

  ensure(license_dir / 'mpfr')
  copyfile(paths.mpfr / 'COPYING.LESSER', license_dir / 'mpfr' / 'COPYING.LESSER')

def build_mingw_toolchain(ver: BranchVersions, paths: ProjectPaths, info: ProfileInfo, config: argparse.Namespace):
  _binutils(ver.binutils, paths, info, config.jobs)

  _gmp(ver.gmp, paths, info, config.jobs)

  _mpfr(ver.mpfr, paths, info, config.jobs)

  _mpc(ver.mpc, paths, info, config.jobs)

  _headers(ver.mingw, paths, info, config.jobs)

  _crt(ver.mingw, paths, info, config.jobs)

  _winpthreads(ver.mingw, paths, info, config.jobs)

  if info.thread == 'mcf':
    _mcfgthread(ver.mcfgthread, paths, info, config.jobs)

  _iconv(ver.iconv, paths, info, config.jobs)

  if Version(ver.gcc).major >= 14:
    _gettext(ver.gettext, paths, info, config.jobs)

  _gcc(ver.gcc, paths, info, config.jobs)

  _gdb(ver.gdb, paths, info, config.jobs)

  _gmake(ver.make, paths, info, config.jobs)

  _licenses(ver, paths, info)
