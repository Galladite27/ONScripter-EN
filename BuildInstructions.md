# Build Instructions

## Officially Supported Targets

ONScripter-en has historically supported an absolute smorgasbord of platforms. For now though out we only officially support the following:
- Windows (MSYS2)
- Linux

# Getting the code

This will look the same no matter how you build. You'll need git, as we use submodules for some of our dependencies.

```
git clone --recurse-submodules -j8 https://github.com/Galladite27/ONScripter-EN.git
```

# Official Instructions

## Windows

Install [MSYS2](https://www.msys2.org/), and choose if you'd like to build for x86 (32-bit) or x86-64 (64-bit) Windows.

### mingw64 (x86-64)

> NOTE: Ensure you open an MSYS2 MINGW64 prompt to run these commands

Install the toolchain and all of the required packages:
```bash
pacman -S mingw-w64-x86-64-SDL mingw-w64-x86-64-SDL_ttf mingw-w64-x86-64-SDL_mixer mingw-w64-x86-64-SDL_image mingw-w64-x86-64-bzip2 mingw-w64-x86-64-libogg mingw-w64-x86-64-libvorbis mingw-w64-x86-64-freetype mingw-w64-x86-64-smpeg mingw-w64-x86-64-iconv mingw-w64-x86-64-zlib mingw-w64-x86-64-toolchain mingw-w64-x86-64-autotools autotools make
```
> NOTE: Installing these is only needed the first time you set up this environment.

And then while inside of the ONScripter-EN directory you cloned earlier:

```bash
make -f ./msys2/Makefile.Windows.MSYS2.x86-64.insani VERBOSE=true all
```

The ONScripter-EN executable will be within the top level repo directory, the tools will be within the tools subdirectory.

### mingw32 (x86)

> NOTE: Ensure you open an MSYS2 MINGW32 prompt to run these commands

Install the toolchain and all of the required packages:
```bash
pacman -S mingw-w64-i686-SDL mingw-w64-i686-SDL_ttf mingw-w64-i686-SDL_mixer mingw-w64-i686-SDL_image mingw-w64-i686-bzip2 mingw-w64-i686-libogg mingw-w64-i686-libvorbis mingw-w64-i686-freetype mingw-w64-i686-smpeg mingw-w64-i686-iconv mingw-w64-i686-zlib mingw-w64-i686-toolchain mingw-w64-i686-autotools autotools make
```
> NOTE: Installing these is only needed the first time you set up this environment.

And then while inside of the ONScripter-EN directory you cloned earlier:
```bash
make -f ./msys2/Makefile.Windows.MSYS2.i686.insani VERBOSE=true all
```

The ONScripter-EN executable will be within the top level repo directory, the tools will be within the tools subdirectory.

## Linux
> NOTE: These instructions are for Ubuntu, so you may need to adjust the packages you install

You must have the following packages available when building ONScripter-EN:


```bash
sudo apt-get install libasound2-dev libpulse-dev libwebp-dev libxrandr-dev autoconf libtool
```
> NOTE: Installing these is only needed the first time you set up this environment.

Next you must configure libogg:

```bash
cd extlib/src/libogg-1.3.5
autoreconf -fi
autoupdate
```

Finally you can configure and build ONScripter-EN and its tools:

```bash
./configure --with-internal-libs
make all
```

The ONScripter-EN executable will be within the top level repo directory, the tools will be within the tools subdirectory.
