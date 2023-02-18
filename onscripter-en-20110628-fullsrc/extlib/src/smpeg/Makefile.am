
## Makefile.am for the smpeg library

bin_SCRIPTS = smpeg-config

CCLD = $(CXXLD)

# The smpeg library target
lib_LTLIBRARIES = libsmpeg.la

libsmpeg_la_SOURCES =		\
	MPEG.cpp		\
	MPEGring.cpp		\
	MPEGlist.cpp		\
	MPEGstream.cpp		\
	MPEGsystem.cpp		\
	MPEGfilter.c		\
	smpeg.cpp		\
	audio/MPEGaudio.cpp	\
	audio/bitwindow.cpp	\
	audio/filter.cpp	\
	audio/filter_2.cpp	\
	audio/hufftable.cpp	\
	audio/mpeglayer1.cpp	\
	audio/mpeglayer2.cpp	\
	audio/mpeglayer3.cpp	\
	audio/mpegtable.cpp	\
	audio/mpegtoraw.cpp	\
	video/MPEGvideo.cpp	\
	video/decoders.cpp	\
	video/decoders.h	\
	video/dither.h		\
	video/floatdct.cpp	\
	video/gdith.cpp		\
	video/jrevdct.cpp	\
	video/motionvec.cpp	\
	video/parseblock.cpp	\
	video/proto.h		\
	video/readfile.cpp	\
	video/util.cpp		\
	video/util.h		\
	video/video.cpp		\
	video/video.h		\
	video/vhar128.cpp	\
	video/vhar128.h		\
	video/mmxflags_asm.S	\
	video/mmxidct_asm.S

libsmpegincludedir = $(includedir)/smpeg
libsmpeginclude_HEADERS =	\
	MPEG.h			\
	MPEGaction.h		\
	MPEGaudio.h		\
	MPEGerror.h		\
	MPEGfilter.h		\
	MPEGring.h		\
	MPEGlist.h		\
	MPEGstream.h		\
	MPEGsystem.h		\
	MPEGvideo.h		\
	smpeg.h

libsmpeg_la_LDFLAGS = 		\
        -release $(LT_RELEASE)	\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

EXTRA_DIST =			\
	CHANGES			\
	COPYING			\
	TODO			\
	README			\
	smpeg.m4		\
	gcc-fat.sh		\
	autogen.sh		\
	audio/AUTHORS		\
	audio/COPYING.LIB	\
	audio/README		\
	audio/README.LIB	\
	video/README		\
	video/COPYRIGHT


video/16bit_mmx.lo: video/16bit_mmx.cpp
	$(CXX) -c -o $@ $^ $(CXXFLAGS)
video/32bit_mmx.lo: video/32bit_mmx.cpp
	$(CXX) -c -o $@ $^ $(CXXFLAGS)

# M4 macro file for inclusion with autoconf
m4datadir = $(datadir)/aclocal
m4data_DATA = smpeg.m4


