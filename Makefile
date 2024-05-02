# -*- makefile-gmake -*-
#
# THIS IS A GENERATED FILE - changes will not be kept if configure is
# run again.  If you wish to customise it, please be sure to give your
# version a different filename.
#
# Makefile for ONScripter-EN

TOPSRC=/home/grodzio/ONScripter-EN

WIN32=
OBJSUFFIX=.o
LIBSUFFIX=.a
EXESUFFIX=

PREFIX ?= /usr/local

# Extra handling for internal libraries.
EXTRADEPS=
INTERNAL_SDL=$(findstring true,false)
INTERNAL_SMPEG=$(findstring true,false)
INTERNAL_LIBPNG=$(findstring true,false)
INTERNAL_LIBJPEG=$(findstring true,false)
INTERNAL_OGGLIBS=$(findstring true,false)
SDL_CONFIG=sdl-config

export PATH     :=   $(shell pwd)/extlib/bin:$(PATH)
export CFLAGS   := -I$(shell pwd)/extlib/include $(CFLAGS) 
export CPPFLAGS := -I$(shell pwd)/extlib/include $(CPPFLAGS) 
export LDFLAGS  := -L$(shell pwd)/extlib/lib $(LDFLAGS) 
export CSTD     := -std=c99
export CXXSTD   := -std=c++98

export CC      := clang
export CXX     := clang++
export MAKE    := gmake
export GNUMAKE := gmake
export AR      := ar
export RANLIB  := ranlib

# ONScripter variables
OSCFLAGSEXTRA =   $(OSCTMPFLAGS)
INCS = -Iextlib/include $(shell $(SDL_CONFIG) --cflags)      \
                        $(shell smpeg-config --cflags)    \
                        $(shell freetype-config --cflags) \
       $(shell [ -d extlib/include/SDL ] && echo -Iextlib/include/SDL)

TOOL_LIBS = -Lextlib/lib \
            -ljpeg -lpng -lz \
            -lbz2
LIBS = -Lextlib/lib \
       -lSDL_image $(if $(findstring true,false),-ljpeg -lpng -lz) \
       -lSDL_mixer $(if $(or $(findstring true,false),$(findstring true,true)),-lvorbisfile -lvorbis -logg) \
       $(shell $(SDL_CONFIG) --libs)      \
       $(shell smpeg-config --libs)    \
       -lSDL_ttf $(shell freetype-config --libs) \
       -lbz2 $(if $(findstring true,false),-L/usr/X11R6/lib -lX11 -lXi -lXrandr -lossaudio)

DEFS = -DLINUX -DUSE_OGG_VORBIS 
EXT_OBJS = 

.SUFFIXES:
.SUFFIXES: .o .cpp .h .c

ifdef DEBUG
OSCFLAGS = -O0 -g -pg -ggdb -pipe -Wpointer-arith   $(OSCFLAGSEXTRA)
export LDFLAGS  := -pg $(LDFLAGS)
else
  ifdef PROF
  OSCFLAGS = -O2 -pg -pipe -Wpointer-arith   $(OSCFLAGSEXTRA)
  export LDFLAGS  := -pg $(LDFLAGS)
  else
  OSCFLAGS = -O2 -fomit-frame-pointer -pipe -Wpointer-arith   $(OSCFLAGSEXTRA)
  endif
endif

TARGET ?= onscripter-en

binary: $(TARGET)

.PHONY: all clean distclean tools binary
all: $(TARGET) tools

SDLOTHERCONFIG := 
OTHERCONFIG := 

OTHER_OBJS =
RC_HDRS =

ICONFILE ?= ons-en.png
RESOURCES ?= $(ICONFILE) =icon.png
OTHER_OBJS = resources$(OBJSUFFIX) 
RC_HDRS = resources.h
RCCLEAN = resources.cpp embed $(OTHER_OBJS)

resources$(OBJSUFFIX): $(RC_HDRS)

resources.cpp: embed $(filter-out =%,$(RESOURCES))
	./embed $(patsubst =%,%,$(RESOURCES)) > $@

embed: embed.cpp
	$(CXX) $(CXXSTD) $< -o $@

include Makefile.extlibs
include Makefile.onscripter

.PHONY: libtoolreplace
libtoolreplace: | $(EB)
	@cp $(ES)/required/freetype-config $(EB)

clean: pclean $(CLEAN_TARGETS) libtoolreplace
distclean: clean pdistclean $(DISTCLEAN_TARGETS)
	rm -r -f extlib/bin extlib/lib extlib/include \
	         extlib/share extlib/man
	rm -f Makefile 

install-bin:
	./install-sh -c -s $(TARGET) $(PREFIX)/bin/$(TARGET)
install: install-bin
uninstall:
	rm $(PREFIX)/bin/$(TARGET)

.PHONY: dist
dist:
	git archive --prefix=onscripter-en-20230628/ HEAD
	tar cf onscripter-en-20230628-fullsrc.tar onscripter-en-20230628
	rm -rf onscripter-en-20230628/extlib onscripter-en-20230628/win_dll \
			onscripter-en-20230628/tools/libgnurx
	tar cf onscripter-en-20230628-src.tar onscripter-en-20230628
	bzip2 -9 onscripter-en-20230628-*src.tar
	rm -rf onscripter-en-20230628
