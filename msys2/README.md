# insani.org Modified configure and Makefile for onscripter-en
**Skip to the Guide section if you just want to compile and install.**
insani.org modified configure and Makefile for the onscripter-en build system, capable of building an x86-64 or an i686 binary for onscripter-en on MSYS2.
https://github.com/insani-org/onscripter-en-msys2-configure-makefile

Unnatural things have been done to make this Makefile work on MSYS2.  Primarily, the existing build system for onscripter-en makes assumptions that fail -- for instance, it creates and compiles code to test for dependencies, but those code snippets *simply fail to compile* in MSYS2, causing cascade failures of dependency build.  I am well aware that this has not been done in the most elegant fashion, but I have found this level of disassembly necessary to begin to plumb the depths of *why* the onscripter-en build system fails as hard as it does on MSYS2.

Preliminary findings follow:

- The build system was written against the original MSYS, which is radically different from MSYS2 in a number of ways.
- Windows system includes and libraries that should work simply *are not properly included or linked* by this build system, causing "undefined reference to '__imp_uwu'" errors everywhere.
- extlibs build fails 100% of the time, in part due to the code snippet compilation issue noted above
- The build system makes faulty assumptions about 32/64-bit, which makes me think that there is a syntax error somewhere in the configure script that causes this behavior (it is present even when compiling for a 32-bit target)
- The build system actually assumes extlibs *even when you have system versions of the libraries in question*, which can lead to cascade failures during the link stage
- -Werror is a monumental mistake; you will simply be unable to compile on Windows if you leave it at that
  - Use -Wall instead
- **OYABB INSANITY SPIRIT IS ALIVE :3**

## Guide
### MSYS2 Setup
Install MSYS2 (https://www.msys2.org) and accept the default install location (C:\msys64).  Once you are in, update MSYS2:

```pacman -Syuu```

This will likely cause MSYS2 to close.  Relaunch and run that command again until nothing further happens.  Then run:

```pacman -S mingw-w64-x86_64-SDL mingw-w64-x86_64-SDL_ttf mingw-w64-x86_64-SDL_mixer mingw-w64-x86_64-SDL_image mingw-w64-x86_64-bzip2 mingw-w64-x86_64-libogg mingw-w64-x86_64-libvorbis mingw-w64-x86_64-freetype mingw-w64-x86_64-smpeg mingw-w64-x86_64-iconv mingw-w64-x86_64-zlib mingw-w64-x86_64-toolchain```

```pacman -S mingw-w64-i686-SDL mingw-w64-i686-SDL_ttf mingw-w64-i686-SDL_mixer mingw-w64-i686-SDL_image mingw-w64-i686-bzip2 mingw-w64-i686-libogg mingw-w64-i686-libvorbis mingw-w64-i686-freetype mingw-w64-i686-smpeg mingw-w64-i686-iconv mingw-w64-i686-zlib mingw-w64-i686-toolchain make```

This will install both the 64-bit and 32-bit versions of these dependencies.

### Compilation

#### 64-Bit
Launch into the MINGW64 environment.  Run:

```make -f Makefile.Windows.MSYS2.x86-64.insani```

#### 32-bit
Launch into the MINGW32 environment.  Run:

```make -f Makefile.Windows.MSYS2.i686.insani```
