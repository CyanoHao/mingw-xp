import argparse
from typing import Dict, Optional

class BranchVersions:
  gcc: str
  rev: str

  binutils: str
  gdb: str
  gettext: Optional[str]
  gmp: str
  iconv: str
  make: str
  mcfgthread: Optional[str]
  mingw: str
  mpc: str
  mpfr: str

  def __init__(
    self,

    gcc: str,
    rev: str,

    binutils: str,
    gdb: str,
    gettext: Optional[str],
    gmp: str,
    iconv: str,
    make: str,
    mcfgthread: Optional[str],
    mingw: str,
    mpc: str,
    mpfr: str,
  ):
    self.gcc = gcc
    self.rev = rev

    self.binutils = binutils
    self.gdb = gdb
    self.gettext = gettext
    self.gmp = gmp
    self.iconv = iconv
    self.make = make
    self.mcfgthread = mcfgthread
    self.mingw = mingw
    self.mpc = mpc
    self.mpfr = mpfr

class ProfileInfo:
  arch: str
  target: str

  crt: str
  exception: str
  thread: str

  host_winnt: int
  target_winnt: int

  def __init__(
    self,

    arch: str,
    target: str,

    crt: str,
    exception: str,
    thread: str,

    host_winnt: int,
    target_winnt: int,
  ):
    self.arch = arch
    self.target = target

    self.crt = crt
    self.exception = exception
    self.thread = thread

    self.host_winnt = host_winnt
    self.target_winnt = target_winnt

class FullProfile:
  ver: BranchVersions
  info: ProfileInfo

  def __init__(
    self,
    ver: BranchVersions,
    info: ProfileInfo,
  ):
    self.ver = ver
    self.info = info

BRANCHES: Dict[str, BranchVersions] = {
  '15': BranchVersions(
    gcc = '15-20241208',
    rev = '0',

    binutils = '2.43.1',
    gdb = '15.1',
    gettext = '0.22.5',
    gmp = '6.3.0',
    iconv = '1.17',
    make = '4.4.1',
    mcfgthread = '1.8-ga.4',
    mingw = '12.0.0',
    mpc = '1.3.1',
    mpfr = '4.2.1',
  ),
  '14': BranchVersions(
    gcc = '14.2.0',
    rev = '4',

    binutils = '2.43.1',
    gdb = '15.1',
    gettext = '0.22.5',
    gmp = '6.3.0',
    iconv = '1.17',
    make = '4.4.1',
    mcfgthread = '1.8-ga.4',
    mingw = '12.0.0',
    mpc = '1.3.1',
    mpfr = '4.2.1',
  ),
  '13': BranchVersions(
    gcc = '13.3.0',
    rev = '0',
    # freeze: 2024-01-01
    binutils = '2.41',
    gdb = '14.2',
    gettext = None,
    gmp = '6.3.0',
    iconv = '1.17',
    make = '4.4.1',
    # postponed until 1.8-ga.3 (2024-08-01)
    # - bootstrapping from scratch made easy
    # - gcc runtime library exception added
    mcfgthread = '1.8-ga.4',
    mingw = '11.0.1',
    mpc = '1.3.1',
    mpfr = '4.2.1',
  ),
  '12': BranchVersions(
    gcc = '12.4.0',
    rev = '0',
    # freeze: 2023-01-01
    binutils = '2.39',
    gdb = '12.1',
    gettext = None,
    gmp = '6.2.1',
    iconv = '1.17',
    make = '4.4.1',
    mcfgthread = None,
    mingw = '10.0.0',
    mpc = '1.3.1',
    mpfr = '4.1.1',
  ),
  '11': BranchVersions(
    gcc = '11.5.0',
    rev = '0',
    # freeze: 2022-01-01
    binutils = '2.37',
    gdb = '11.2',
    gettext = None,
    gmp = '6.2.1',
    iconv = '1.16',
    make = '4.3',
    mcfgthread = None,
    mingw = '9.0.0',
    mpc = '1.2.1',
    mpfr = '4.1.1',
  ),
  '10': BranchVersions(
    gcc = '10.5.0',
    rev = '0',
    # freeze: 2021-01-01
    binutils = '2.35.2',
    gdb = '10.2',
    gettext = None,
    gmp = '6.2.1',
    iconv = '1.16',
    make = '4.3',
    mcfgthread = None,
    mingw = '8.0.3',
    mpc = '1.2.1',
    mpfr = '4.1.1',
  ),
  '9': BranchVersions(
    gcc = '9.5.0',
    rev = '0',
    # freeze: 2020-01-01
    binutils = '2.33.1',
    gdb = '8.3.1',
    gettext = None,
    gmp = '6.1.2',
    iconv = '1.16',
    make = '4.2.1',
    mcfgthread = None,
    mingw = '7.0.0',
    mpc = '1.1.0',
    mpfr = '4.0.2',
  ),
  '8': BranchVersions(
    gcc = '8.5.0',
    rev = '0',
    # freeze: 2019-01-01
    binutils = '2.31.1',
    gdb = '8.2.1',
    gettext = None,
    gmp = '6.1.2',
    iconv = '1.15',
    make = '4.2.1',
    mcfgthread = None,
    mingw = '6.0.1',
    mpc = '1.1.0',
    mpfr = '4.0.2',
  ),
  '7': BranchVersions(
    gcc = '7.5.0',
    rev = '0',
    # freeze: 2018-01-01
    binutils = '2.29.1',
    gdb = '8.0.1',
    gettext = None,
    gmp = '6.1.2',
    iconv = '1.15',
    make = '4.2.1',
    mcfgthread = None,
    mingw = '5.0.5',
    mpc = '1.0.3',
    # mpfr 4.0 released, but mpc was not ready
    mpfr = '3.1.6',
  ),
  '6': BranchVersions(
    gcc = '6.5.0',
    rev = '0',
    # freeze: 2017-01-01
    binutils = '2.27',
    gdb = '7.12.1',
    gettext = None,
    gmp = '6.1.2',
    iconv = '1.14',
    make = '4.2.1',
    mcfgthread = None,
    mingw = '5.0.5',
    mpc = '1.0.3',
    mpfr = '3.1.6',
  ),
  '5': BranchVersions(
    gcc = '5.5.0',
    rev = '0',
    # freeze: 2016-01-01
    binutils = '2.25.1',
    gdb = '7.10.1',
    gettext = None,
    gmp = '6.1.2',
    iconv = '1.14',
    make = '4.1',
    mcfgthread = None,
    mingw = '4.0.6',
    mpc = '1.0.3',
    mpfr = '3.1.6',
  ),
  '4.9': BranchVersions(
    gcc = '4.9.4',
    rev = '0',
    # freeze: 2015-01-01
    binutils = '2.25.1',
    gdb = '7.8.2',
    gettext = None,
    gmp = '5.1.3',
    iconv = '1.14',
    make = '4.1',
    mcfgthread = None,
    mingw = '3.3.0',
    mpc = '1.0.3',
    mpfr = '3.1.6',
  ),
  '4.8': BranchVersions(
    gcc = '4.8.5',
    rev = '0',
    # freeze: 2014-01-01
    binutils = '2.24',
    gdb = '7.6.2',
    gettext = None,
    gmp = '5.1.3',
    iconv = '1.14',
    make = '4.0',
    mcfgthread = None,
    mingw = '3.3.0',
    mpc = '1.0.3',
    mpfr = '3.1.6',
  ),
}

PROFILES: Dict[str, ProfileInfo] = {
  '64-mcf': ProfileInfo(
    arch = '64',
    target = 'x86_64-w64-mingw32',

    crt = 'ucrt',
    exception = 'seh',
    thread = 'mcf',

    host_winnt = 0x0601,
    target_winnt = 0x0601,
  ),
  '64-ucrt': ProfileInfo(
    arch = '64',
    target = 'x86_64-w64-mingw32',

    crt = 'ucrt',
    exception = 'seh',
    thread = 'posix',

    host_winnt = 0x0601,
    target_winnt = 0x0502,
  ),
  '64-msvcrt': ProfileInfo(
    arch = '64',
    target = 'x86_64-w64-mingw32',

    crt = 'msvcrt',
    exception = 'seh',
    thread = 'posix',

    host_winnt = 0x0502,
    target_winnt = 0x0502,
  ),

  'arm64-mcf': ProfileInfo(
    arch = 'arm64',
    target = 'aarch64-w64-mingw32',

    crt = 'ucrt',
    exception = 'seh',
    thread = 'mcf',

    host_winnt = 0x0A00,
    target_winnt = 0x0A00,
  ),
  'arm64-ucrt': ProfileInfo(
    arch = 'arm64',
    target = 'aarch64-w64-mingw32',

    crt = 'ucrt',
    exception = 'seh',
    thread = 'posix',

    host_winnt = 0x0A00,
    target_winnt = 0x0A00,
  ),

  '32-mcf': ProfileInfo(
    arch = '32',
    target = 'i686-w64-mingw32',

    crt = 'ucrt',
    exception = 'dwarf',
    thread = 'mcf',

    host_winnt = 0x0601,
    target_winnt = 0x0601,
  ),
  '32-ucrt': ProfileInfo(
    arch = '32',
    target = 'i686-w64-mingw32',

    crt = 'ucrt',
    exception = 'dwarf',
    thread = 'posix',

    host_winnt = 0x0601,
    target_winnt = 0x0501,
  ),
  '32-msvcrt': ProfileInfo(
    arch = '32',
    target = 'i686-w64-mingw32',

    crt = 'msvcrt',
    exception = 'dwarf',
    thread = 'posix',

    host_winnt = 0x0500,
    target_winnt = 0x0400,
  ),

  '32-legacy': ProfileInfo(
    arch = '32',
    target = 'i486-w64-mingw32',

    crt = 'msvcrt',
    exception = 'dwarf',
    thread = 'posix',

    host_winnt = 0x03FF,
    target_winnt = 0x03FF,
  )
}

def get_full_profile(config: argparse.Namespace) -> FullProfile:
  return FullProfile(
    ver = BRANCHES[config.branch],
    info = PROFILES[config.profile],
  )
