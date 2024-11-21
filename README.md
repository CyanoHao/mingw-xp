# MinGW Lite

Extremely fast MinGW GCC build scripts for easy experiments.

## Build

```bash
podman build -t mingw-lite/buildenv-ubuntu support/buildenv-ubuntu

podman run -it --rm -v $PWD:/mnt -w /mnt mingw-lite/buildenv-ubuntu

# in the container
./main.py -b <branch> -p <profile>
```

Available branches:

| Branch | GCC version | GCC adoption | MinGW | Binutils |
| ------ | ----------- | ------------ | ----- | -------- |
| 15 | 15-20240908 (exp) | | 12.0.0 | 2.43.1 |
| 14 | 14.2.0 | Debian 13, EL 10 | 12.0.0 | 2.43.1 |
| 13 | 13.3.0 | Ubuntu 24.04 | 11.0.1 | 2.41 |
| 12 | 12.4.0 | Debian 12 | 10.0.0 | 2.39 |
| 11 | 11.5.0 (EOL) | Ubuntu 22.04, EL 9 | 9.0.0 | 2.37 |
| 10 | 10.5.0 (EOL) | Debian 11 | 8.0.3 | 2.35.2 |
| 9 | 9.5.0 (EOL) | Ubuntu 20.04 | 7.0.0 | 2.33.1 |
| 8 | 8.5.0 (EOL) | Debian 10, EL 8 | 6.0.1 | 2.31.1 |
| 7 | 7.5.0 (EOL) | Ubuntu 18.04 | 5.0.5 | 2.29.1 |
| 6 | 6.5.0 (EOL) | Debian 9 | 5.0.5 | 2.27 |
| 5 | 5.5.0 (EOL) | Ubuntu 16.04 | 4.0.6 | 2.25.1 |
| 4.9 | 4.9.4 (EOL) | Debian 8 | 3.3.0 | 2.25.1 |
| 4.8 | 4.8.5 (EOL) | Ubuntu 14.04, EL 7 | 3.3.0 | 2.24 |

Available profiles:

| Profile | Arch | CRT | Threads | Exception | HOST | TARGET |
| ------- | ---- | --- | ------- | --------- | ---- | ------ |
| 64-mcf | x86_64 | ucrt | mcf | seh | 7+ | 7+ |
| 64-ucrt | x86_64 | ucrt | posix | seh | 7+ | 2003+ |
| 64-msvcrt | x86_64 | msvcrt | posix | seh | 2003+ | 2003+ |
| | | | | | | |
| 32-mcf | i686 | ucrt | mcf | dwarf | 7+ | 7+ |
| 32-ucrt | i686 | ucrt | posix | dwarf | 7+ | XP+ |
| 32-msvcrt | i686 | msvcrt | posix | dwarf | NT+ | NT+, 9x |
| | | | | | | |
| 32-legacy | i486 | msvcrt | posix | dwarf | NT+ | NT+, 9x |

Notes:

- To expose build directories for debugging:
  ```bash
  mkdir -p build pkg

  podman run -it --rm \
    -v $PWD:/mnt -w /mnt \
    -v $PWD/build:/tmp/build \
    -v $PWD/pkg:/opt \
    mingw-lite/buildenv-ubuntu
  ```
- Version “freeze” happens at next 01-01 after GCC’s release.
- The “32-legacy” profile partially runs on Windows 9x host.
  - GDB does not work.
  - On Windows 95, where dlopen-ing dlls with `.tls` section is impossible, GCC should be called with `-fno-lto` to ensure `ld` would not load LTO plugin. ([KB118816](https://www.betaarchive.com/wiki/index.php/Microsoft_KB_Archive/118816))

## Patches

The MinGW Lite project intends to apply as few patches as possible to the upstream source. There are two kinds of patches: thunks and normal patches.

Inspired by [YY-Thunks](https://github.com/Chuyu-Team/YY-Thunks), the thunks are small pieces of code that wrap the original Win32 and CRT functions. They are used to make the toolchain work on Windows prior to Windows XP. Thunks are built into import libraris, and will affect user programs. See [the thunk list](doc/thunk-list.md) for implementation details and quirks.

Normal patches are applied to the upstream source code. They are used to fix build failures, bugs, and (some tricky) compatibility issues. They do not affect user programs. See [the patch list](doc/patch-list.md) for implementation details.

In the patching module, releases of Windows are treated _totally ordered_, assuming “Windows 9x (internally _WIN32_WINNT = 0x03FF) < Windows NT 4.0”. There does exist some APIs that are available on Windows 9x but not on Windows NT 4.0, while the patching module does not handle such cases.
