##########
# bzip2
if (ONS_USE_OLD_DEPS)
    set(bzip2_source_prefix src/old/bzip2-1.0.4)
else()
    set(bzip2_source_prefix src/bzip2-1.0.8)
endif()

add_library(bzip2 STATIC "")

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(bzip2
    PRIVATE
        -fdiagnostics-color=always
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(bzip2
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_sources(bzip2
PRIVATE
    ${bzip2_source_prefix}/blocksort.c
    ${bzip2_source_prefix}/bzlib_private.h
    ${bzip2_source_prefix}/bzlib.c
    ${bzip2_source_prefix}/bzlib.h
    ${bzip2_source_prefix}/compress.c
    ${bzip2_source_prefix}/crctable.c
    ${bzip2_source_prefix}/decompress.c
    ${bzip2_source_prefix}/huffman.c
    ${bzip2_source_prefix}/randtable.c
)

target_include_directories(bzip2
PUBLIC
    ${bzip2_source_prefix}/
)

##########
# libogg
if (ONS_USE_OLD_DEPS)
    set(ogg_source_prefix src/old/libogg-1.1.3)
else()
    set(ogg_source_prefix src/libogg-1.3.5)
endif()

add_library(ogg STATIC "")

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(ogg
    PRIVATE
        -fdiagnostics-color=always
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(ogg
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_sources(ogg
PRIVATE
    ${ogg_source_prefix}/src/bitwise.c
    ${ogg_source_prefix}/src/framing.c
)

target_include_directories(ogg
PUBLIC
    ${ogg_source_prefix}/include
)

##########
# libvorbis
if (ONS_USE_OLD_DEPS)
    set(vorbis_source_prefix src/old/libvorbis-1.2.0)
else()
    set(vorbis_source_prefix src/libvorbis-1.3.7)
endif()

add_library(vorbis STATIC "")

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(vorbis
    PRIVATE
        -fdiagnostics-color=always
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(vorbis
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_link_libraries(vorbis
PUBLIC
    ogg
)


if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(vorbis 
    PRIVATE
        -Wno-int-conversion
        -Wno-overflow
        -Wno-incompatible-pointer-types
        -Wno-implicit-function-declaration
    )
endif()

target_sources(vorbis
PRIVATE
    ${vorbis_source_prefix}/lib/analysis.c
    ${vorbis_source_prefix}/lib/backends.h
    #${vorbis_source_prefix}/lib/barkmel.c
    ${vorbis_source_prefix}/lib/bitrate.c
    ${vorbis_source_prefix}/lib/bitrate.h
    ${vorbis_source_prefix}/lib/block.c
    ${vorbis_source_prefix}/lib/codebook.c
    ${vorbis_source_prefix}/lib/codebook.h
    ${vorbis_source_prefix}/lib/codec_internal.h
    ${vorbis_source_prefix}/lib/envelope.c
    ${vorbis_source_prefix}/lib/envelope.h
    ${vorbis_source_prefix}/lib/floor0.c
    ${vorbis_source_prefix}/lib/floor1.c
    ${vorbis_source_prefix}/lib/highlevel.h
    ${vorbis_source_prefix}/lib/info.c
    ${vorbis_source_prefix}/lib/lookup.c
    ${vorbis_source_prefix}/lib/lookup.h
    ${vorbis_source_prefix}/lib/lookups.pl
    ${vorbis_source_prefix}/lib/lookup_data.h
    ${vorbis_source_prefix}/lib/lpc.c
    ${vorbis_source_prefix}/lib/lpc.h
    ${vorbis_source_prefix}/lib/lsp.c
    ${vorbis_source_prefix}/lib/lsp.h
    ${vorbis_source_prefix}/lib/Makefile.am
    ${vorbis_source_prefix}/lib/Makefile.in
    ${vorbis_source_prefix}/lib/mapping0.c
    ${vorbis_source_prefix}/lib/masking.h
    ${vorbis_source_prefix}/lib/mdct.c
    ${vorbis_source_prefix}/lib/mdct.h
    ${vorbis_source_prefix}/lib/misc.h
    ${vorbis_source_prefix}/lib/os.h
    ${vorbis_source_prefix}/lib/psy.c
    ${vorbis_source_prefix}/lib/psy.h
    #${vorbis_source_prefix}/lib/psytune.c
    ${vorbis_source_prefix}/lib/registry.c
    ${vorbis_source_prefix}/lib/registry.h
    ${vorbis_source_prefix}/lib/res0.c
    ${vorbis_source_prefix}/lib/scales.h
    ${vorbis_source_prefix}/lib/sharedbook.c
    ${vorbis_source_prefix}/lib/smallft.c
    ${vorbis_source_prefix}/lib/smallft.h
    ${vorbis_source_prefix}/lib/synthesis.c
    #${vorbis_source_prefix}/lib/tone.c
    ${vorbis_source_prefix}/lib/vorbisenc.c
    ${vorbis_source_prefix}/lib/vorbisfile.c
    ${vorbis_source_prefix}/lib/window.c
    ${vorbis_source_prefix}/lib/window.h
    ${vorbis_source_prefix}/vq/bookutil.c
    ${vorbis_source_prefix}/vq/bookutil.h
    #${vorbis_source_prefix}/vq/build.c
    ${vorbis_source_prefix}/vq/cascade.c
    #${vorbis_source_prefix}/vq/distribution.c
    ${vorbis_source_prefix}/vq/genericdata.c
    #${vorbis_source_prefix}/vq/huffbuild.c
    #${vorbis_source_prefix}/vq/latticebuild.c
    #${vorbis_source_prefix}/vq/latticehint.c
    #${vorbis_source_prefix}/vq/latticepare.c
    #${vorbis_source_prefix}/vq/latticetune.c
    ${vorbis_source_prefix}/vq/localcodebook.h
    ${vorbis_source_prefix}/vq/lspdata.c
    ${vorbis_source_prefix}/vq/metrics.c
    ${vorbis_source_prefix}/vq/residuedata.c
    #${vorbis_source_prefix}/vq/residuesplit.c
    #${vorbis_source_prefix}/vq/run.c
    #${vorbis_source_prefix}/vq/train.c
    ${vorbis_source_prefix}/vq/vqext.h
    ${vorbis_source_prefix}/vq/vqgen.c
    ${vorbis_source_prefix}/vq/vqgen.h
    ${vorbis_source_prefix}/vq/vqsplit.c
    ${vorbis_source_prefix}/vq/vqsplit.h
)

target_include_directories(vorbis
PUBLIC
    ${vorbis_source_prefix}/include
    ${vorbis_source_prefix}/lib
    ${vorbis_source_prefix}/vq
)

##########
# libjpeg
#if (ONS_USE_OLD_DEPS)
#    set(jpeg_source_prefix src/old/jpeg-6b)
#else()
#    set(jpeg_source_prefix src/jpeg-6b)
#endif()
#
#add_library(jpeg STATIC "")
#
#target_compile_definitions(jpeg
#PRIVATE
#    JPEG_CJPEG_DJPEG
#)
#
#if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
#    target_compile_options(jpeg
#    PRIVATE
#        -fdiagnostics-color=always
#        $<$<CONFIG:DEBUG>:-g>
#    )
#
#    target_link_options(jpeg
#    PUBLIC
#        $<$<CONFIG:RELEASE>:-s>
#    )
#endif()
#
#if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
#    target_compile_options(jpeg 
#    PRIVATE
#        -Wno-implicit-function-declaration
#        -Wno-builtin-declaration-mismatch
#    )
#endif()
#
#target_sources(jpeg
#PRIVATE
#    ${jpeg_source_prefix}/jcapimin.c
#    ${jpeg_source_prefix}/jcapistd.c
#    ${jpeg_source_prefix}/jccoefct.c
#    ${jpeg_source_prefix}/jccolor.c
#    ${jpeg_source_prefix}/jcdctmgr.c
#    ${jpeg_source_prefix}/jchuff.c
#    ${jpeg_source_prefix}/jchuff.h
#    ${jpeg_source_prefix}/jcinit.c
#    ${jpeg_source_prefix}/jcmainct.c
#    ${jpeg_source_prefix}/jcmarker.c
#    ${jpeg_source_prefix}/jcmaster.c
#    ${jpeg_source_prefix}/jcomapi.c
#    ${jpeg_source_prefix}/jcparam.c
#    ${jpeg_source_prefix}/jcphuff.c
#    ${jpeg_source_prefix}/jcprepct.c
#    ${jpeg_source_prefix}/jcsample.c
#    ${jpeg_source_prefix}/jctrans.c
#    ${jpeg_source_prefix}/jdapimin.c
#    ${jpeg_source_prefix}/jdapistd.c
#    ${jpeg_source_prefix}/jdatadst.c
#    ${jpeg_source_prefix}/jdatasrc.c
#    ${jpeg_source_prefix}/jdcoefct.c
#    ${jpeg_source_prefix}/jdcolor.c
#    ${jpeg_source_prefix}/jdct.h
#    ${jpeg_source_prefix}/jddctmgr.c
#    ${jpeg_source_prefix}/jdhuff.c
#    ${jpeg_source_prefix}/jdhuff.h
#    ${jpeg_source_prefix}/jdinput.c
#    ${jpeg_source_prefix}/jdmainct.c
#    ${jpeg_source_prefix}/jdmarker.c
#    ${jpeg_source_prefix}/jdmaster.c
#    ${jpeg_source_prefix}/jdmerge.c
#    ${jpeg_source_prefix}/jdphuff.c
#    ${jpeg_source_prefix}/jdpostct.c
#    ${jpeg_source_prefix}/jdsample.c
#    ${jpeg_source_prefix}/jdtrans.c
#    ${jpeg_source_prefix}/jerror.c
#    ${jpeg_source_prefix}/jerror.h
#    ${jpeg_source_prefix}/jfdctflt.c
#    ${jpeg_source_prefix}/jfdctfst.c
#    ${jpeg_source_prefix}/jfdctint.c
#    ${jpeg_source_prefix}/jidctflt.c
#    ${jpeg_source_prefix}/jidctfst.c
#    ${jpeg_source_prefix}/jidctint.c
#    ${jpeg_source_prefix}/jidctred.c
#    ${jpeg_source_prefix}/jinclude.h
#    ${jpeg_source_prefix}/jmemansi.c
#    ${jpeg_source_prefix}/jmemmgr.c
#    ${jpeg_source_prefix}/jmemname.c
#    ${jpeg_source_prefix}/jmemnobs.c
#    ${jpeg_source_prefix}/jmemsys.h
#    ${jpeg_source_prefix}/jmorecfg.h
#    ${jpeg_source_prefix}/jpegint.h
#    ${jpeg_source_prefix}/jpeglib.h
#    ${jpeg_source_prefix}/jpegtran.1
#    ${jpeg_source_prefix}/jpegtran.c
#    ${jpeg_source_prefix}/jquant1.c
#    ${jpeg_source_prefix}/jquant2.c
#    ${jpeg_source_prefix}/jutils.c
#    ${jpeg_source_prefix}/jversion.h
#)
#
#target_include_directories(jpeg
#PUBLIC
#    ${jpeg_source_prefix}/
#)

##########
# freetype
if (ONS_USE_OLD_DEPS)
    set(freetype_source_prefix src/old/freetype-2.3.5)
else()
    set(freetype_source_prefix src/freetype-2.13.0)
endif()

add_library(freetype STATIC "")

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(freetype
    PRIVATE
        -fdiagnostics-color=always
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(freetype
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()


if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(freetype 
    PRIVATE
        -Wno-implicit-function-declaration
    )
endif()

target_compile_definitions(freetype 
PUBLIC 
    FT2_BUILD_LIBRARY
    FT_OPTION_AUTOFIT2
)

target_sources(freetype
PRIVATE
    ${freetype_source_prefix}/src/autofit/afangles.h
    ${freetype_source_prefix}/src/autofit/afcjk.h
    ${freetype_source_prefix}/src/autofit/afdummy.h
    ${freetype_source_prefix}/src/autofit/aferrors.h
    ${freetype_source_prefix}/src/autofit/afglobal.h
    ${freetype_source_prefix}/src/autofit/afhints.h
    ${freetype_source_prefix}/src/autofit/afindic.h
    ${freetype_source_prefix}/src/autofit/aflatin.h
    ${freetype_source_prefix}/src/autofit/aflatin2.h
    ${freetype_source_prefix}/src/autofit/afloader.h
    ${freetype_source_prefix}/src/autofit/afmodule.h
    ${freetype_source_prefix}/src/autofit/aftypes.h
    ${freetype_source_prefix}/src/autofit/afwarp.h
    ${freetype_source_prefix}/src/autofit/autofit.c
    ${freetype_source_prefix}/src/autofit/Jamfile
    ${freetype_source_prefix}/src/autofit/module.mk
    ${freetype_source_prefix}/src/autofit/rules.mk
    ${freetype_source_prefix}/src/base/ftapi.c
    ${freetype_source_prefix}/src/base/ftbase.c
    ${freetype_source_prefix}/src/base/ftbbox.c
    ${freetype_source_prefix}/src/base/ftbitmap.c
    ${freetype_source_prefix}/src/base/ftdebug.c
    ${freetype_source_prefix}/src/base/ftgasp.c
    ${freetype_source_prefix}/src/base/ftglyph.c
    ${freetype_source_prefix}/src/base/ftgxval.c
    ${freetype_source_prefix}/src/base/ftinit.c
    ${freetype_source_prefix}/src/base/ftlcdfil.c
    ${freetype_source_prefix}/src/base/ftotval.c
    ${freetype_source_prefix}/src/base/ftstroke.c
    ${freetype_source_prefix}/src/base/ftsystem.c
    ${freetype_source_prefix}/src/base/fttype1.c
    ${freetype_source_prefix}/src/base/Jamfile
    ${freetype_source_prefix}/src/base/rules.mk
    ${freetype_source_prefix}/src/cache/ftcache.c
    ${freetype_source_prefix}/src/cache/ftccache.h
    ${freetype_source_prefix}/src/cache/ftccback.h
    ${freetype_source_prefix}/src/cache/ftcerror.h
    ${freetype_source_prefix}/src/cache/ftcglyph.h
    ${freetype_source_prefix}/src/cache/ftcimage.h
    ${freetype_source_prefix}/src/cache/ftcmanag.h
    ${freetype_source_prefix}/src/cache/ftcmru.h
    ${freetype_source_prefix}/src/cache/ftcsbits.h
    ${freetype_source_prefix}/src/cache/Jamfile
    ${freetype_source_prefix}/src/cache/rules.mk
    ${freetype_source_prefix}/src/cff/cff.c
    ${freetype_source_prefix}/src/cff/cffcmap.h
    ${freetype_source_prefix}/src/cff/cffdrivr.h
    ${freetype_source_prefix}/src/cff/cfferrs.h
    ${freetype_source_prefix}/src/cff/cffgload.h
    ${freetype_source_prefix}/src/cff/cffload.h
    ${freetype_source_prefix}/src/cff/cffobjs.h
    ${freetype_source_prefix}/src/cff/cffparse.h
    ${freetype_source_prefix}/src/cff/cfftoken.h
    ${freetype_source_prefix}/src/cff/cfftypes.h
    ${freetype_source_prefix}/src/cff/Jamfile
    ${freetype_source_prefix}/src/cff/module.mk
    ${freetype_source_prefix}/src/cff/rules.mk
    ${freetype_source_prefix}/src/cid/ciderrs.h
    ${freetype_source_prefix}/src/cid/cidgload.h
    ${freetype_source_prefix}/src/cid/cidload.h
    ${freetype_source_prefix}/src/cid/cidobjs.h
    ${freetype_source_prefix}/src/cid/cidparse.h
    ${freetype_source_prefix}/src/cid/cidriver.h
    ${freetype_source_prefix}/src/cid/cidtoken.h
    ${freetype_source_prefix}/src/cid/Jamfile
    ${freetype_source_prefix}/src/cid/module.mk
    ${freetype_source_prefix}/src/cid/rules.mk
    ${freetype_source_prefix}/src/cid/type1cid.c
    ${freetype_source_prefix}/src/otvalid/Jamfile
    ${freetype_source_prefix}/src/otvalid/module.mk
    ${freetype_source_prefix}/src/otvalid/otvalid.c
    ${freetype_source_prefix}/src/otvalid/otvalid.h
    ${freetype_source_prefix}/src/otvalid/otvcommn.h
    ${freetype_source_prefix}/src/otvalid/otverror.h
    ${freetype_source_prefix}/src/otvalid/otvgpos.h
    ${freetype_source_prefix}/src/otvalid/otvmod.h
    ${freetype_source_prefix}/src/otvalid/rules.mk
    ${freetype_source_prefix}/src/psaux/afmparse.h
    ${freetype_source_prefix}/src/psaux/Jamfile
    ${freetype_source_prefix}/src/psaux/module.mk
    ${freetype_source_prefix}/src/psaux/psaux.c
    ${freetype_source_prefix}/src/psaux/psauxerr.h
    ${freetype_source_prefix}/src/psaux/psauxmod.h
    ${freetype_source_prefix}/src/psaux/psconv.h
    ${freetype_source_prefix}/src/psaux/psobjs.h
    ${freetype_source_prefix}/src/psaux/rules.mk
    ${freetype_source_prefix}/src/psaux/t1cmap.h
    ${freetype_source_prefix}/src/psaux/t1decode.h
    ${freetype_source_prefix}/src/pshinter/Jamfile
    ${freetype_source_prefix}/src/pshinter/module.mk
    ${freetype_source_prefix}/src/pshinter/pshalgo.h
    ${freetype_source_prefix}/src/pshinter/pshglob.h
    ${freetype_source_prefix}/src/pshinter/pshinter.c
    ${freetype_source_prefix}/src/pshinter/pshmod.h
    ${freetype_source_prefix}/src/pshinter/pshnterr.h
    ${freetype_source_prefix}/src/pshinter/pshrec.h
    ${freetype_source_prefix}/src/pshinter/rules.mk
    ${freetype_source_prefix}/src/psnames/Jamfile
    ${freetype_source_prefix}/src/psnames/module.mk
    ${freetype_source_prefix}/src/psnames/psmodule.h
    ${freetype_source_prefix}/src/psnames/psnamerr.h
    ${freetype_source_prefix}/src/psnames/psnames.c
    ${freetype_source_prefix}/src/psnames/pstables.h
    ${freetype_source_prefix}/src/psnames/rules.mk
    ${freetype_source_prefix}/src/sfnt/Jamfile
    ${freetype_source_prefix}/src/sfnt/module.mk
    ${freetype_source_prefix}/src/sfnt/rules.mk
    ${freetype_source_prefix}/src/sfnt/sfdriver.h
    ${freetype_source_prefix}/src/sfnt/sferrors.h
    ${freetype_source_prefix}/src/sfnt/sfnt.c
    ${freetype_source_prefix}/src/sfnt/sfobjs.h
    ${freetype_source_prefix}/src/sfnt/ttbdf.h
    ${freetype_source_prefix}/src/sfnt/ttcmap.h
    ${freetype_source_prefix}/src/sfnt/ttkern.h
    ${freetype_source_prefix}/src/sfnt/ttload.h
    ${freetype_source_prefix}/src/sfnt/ttmtx.h
    ${freetype_source_prefix}/src/sfnt/ttpost.h
    ${freetype_source_prefix}/src/sfnt/ttsbit.h
    ${freetype_source_prefix}/src/sfnt/ttsbit0.c
    ${freetype_source_prefix}/src/smooth/ftgrays.h
    ${freetype_source_prefix}/src/smooth/ftsmerrs.h
    ${freetype_source_prefix}/src/smooth/ftsmooth.h
    ${freetype_source_prefix}/src/smooth/Jamfile
    ${freetype_source_prefix}/src/smooth/module.mk
    ${freetype_source_prefix}/src/smooth/rules.mk
    ${freetype_source_prefix}/src/smooth/smooth.c
    ${freetype_source_prefix}/src/truetype/Jamfile
    ${freetype_source_prefix}/src/truetype/module.mk
    ${freetype_source_prefix}/src/truetype/rules.mk
    ${freetype_source_prefix}/src/truetype/truetype.c
    ${freetype_source_prefix}/src/truetype/ttdriver.h
    ${freetype_source_prefix}/src/truetype/tterrors.h
    ${freetype_source_prefix}/src/truetype/ttgload.h
    ${freetype_source_prefix}/src/truetype/ttgxvar.h
    ${freetype_source_prefix}/src/truetype/ttinterp.h
    ${freetype_source_prefix}/src/truetype/ttobjs.h
    ${freetype_source_prefix}/src/truetype/ttpload.h
    ${freetype_source_prefix}/src/type1/Jamfile
    ${freetype_source_prefix}/src/type1/module.mk
    ${freetype_source_prefix}/src/type1/rules.mk
    ${freetype_source_prefix}/src/type1/t1afm.h
    ${freetype_source_prefix}/src/type1/t1driver.h
    ${freetype_source_prefix}/src/type1/t1errors.h
    ${freetype_source_prefix}/src/type1/t1gload.h
    ${freetype_source_prefix}/src/type1/t1load.h
    ${freetype_source_prefix}/src/type1/t1objs.h
    ${freetype_source_prefix}/src/type1/t1parse.h
    ${freetype_source_prefix}/src/type1/t1tokens.h
    ${freetype_source_prefix}/src/type1/type1.c
)

target_include_directories(freetype
PUBLIC
    ${freetype_source_prefix}/include
)


##########
# SDL_mixer
if (ONS_USE_OLD_DEPS)
    set(sdl_mixer_source_prefix src/old/SDL_mixer-1.2.8)
else()
    set(sdl_mixer_source_prefix src/SDL_mixer-1.2.12)
endif()

add_library(SDL_mixer STATIC)

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(SDL_mixer
    PRIVATE
        -fdiagnostics-color=always
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(SDL_mixer
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_link_libraries(SDL_mixer
PUBLIC
    freetype
    PkgConfigWrapper::SDL
)

target_compile_definitions(SDL_mixer
PUBLIC
    USE_NATIVE_MIDI
    MID_MUSIC
)

if(WIN32)
    target_sources(SDL_mixer
    PRIVATE
        ${sdl_mixer_source_prefix}/native_midi/native_midi_common.c
        ${sdl_mixer_source_prefix}/native_midi/native_midi_win32.c
    )
elseif(APPLE)
    target_sources(SDL_mixer
    PRIVATE
        ${sdl_mixer_source_prefix}/native_midi/native_midi_common.c
        ${sdl_mixer_source_prefix}/native_midi/native_midi_mac.c
    )
endif()

target_include_directories(SDL_mixer
PRIVATE
    ${sdl_mixer_source_prefix}/native_midi/
)

target_sources(SDL_mixer
PRIVATE
    ${sdl_mixer_source_prefix}/dynamic_mp3.c
    ${sdl_mixer_source_prefix}/dynamic_mp3.h
    ${sdl_mixer_source_prefix}/dynamic_ogg.c
    ${sdl_mixer_source_prefix}/dynamic_ogg.h
    ${sdl_mixer_source_prefix}/effects_internal.c
    ${sdl_mixer_source_prefix}/effects_internal.h
    ${sdl_mixer_source_prefix}/effect_position.c
    ${sdl_mixer_source_prefix}/effect_stereoreverse.c
    ${sdl_mixer_source_prefix}/load_aiff.c
    ${sdl_mixer_source_prefix}/load_aiff.h
    ${sdl_mixer_source_prefix}/load_ogg.c
    ${sdl_mixer_source_prefix}/load_ogg.h
    ${sdl_mixer_source_prefix}/load_voc.c
    ${sdl_mixer_source_prefix}/load_voc.h
    ${sdl_mixer_source_prefix}/Makefile.in
    ${sdl_mixer_source_prefix}/mixer.c
    ${sdl_mixer_source_prefix}/music.c
    ${sdl_mixer_source_prefix}/music_cmd.c
    ${sdl_mixer_source_prefix}/music_cmd.h
    ${sdl_mixer_source_prefix}/music_ogg.c
    ${sdl_mixer_source_prefix}/music_ogg.h
    ${sdl_mixer_source_prefix}/SDL_mixer.h
    #${sdl_mixer_source_prefix}/SDL_mixer.qpg.in
    #${sdl_mixer_source_prefix}/SDL_mixer.spec
    #${sdl_mixer_source_prefix}/SDL_mixer.spec.in
    ${sdl_mixer_source_prefix}/wavestream.c
    ${sdl_mixer_source_prefix}/wavestream.h
)


##########
# SDL_ttf-2.0.11
if (ONS_USE_OLD_DEPS)
    set(sdl_ttf_source_prefix src/old/SDL_ttf-2.0.8)
else()
    set(sdl_ttf_source_prefix src/SDL_ttf-2.0.11)
endif()

add_library(SDL_ttf STATIC)

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(SDL_ttf
    PRIVATE
        -fdiagnostics-color=always
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(SDL_ttf
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_link_libraries(SDL_ttf
PUBLIC
    freetype
    PkgConfigWrapper::SDL
)

target_sources(SDL_ttf
PRIVATE
    ${sdl_ttf_source_prefix}/SDL_ttf.c
    ${sdl_ttf_source_prefix}/SDL_ttf.h
)

##########
# zlib-1.2.3
set(zlib_source_prefix src/zlib-1.2.3)

add_library(zlib STATIC)

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(zlib
    PRIVATE
        -fdiagnostics-color=always
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(zlib
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_sources(zlib
PRIVATE
    ${zlib_source_prefix}/adler32.c
    ${zlib_source_prefix}/compress.c
    ${zlib_source_prefix}/crc32.c
    ${zlib_source_prefix}/crc32.h
    ${zlib_source_prefix}/deflate.c
    ${zlib_source_prefix}/deflate.h
    ${zlib_source_prefix}/gzio.c
    ${zlib_source_prefix}/infback.c
    ${zlib_source_prefix}/inffast.c
    ${zlib_source_prefix}/inffast.h
    ${zlib_source_prefix}/inffixed.h
    ${zlib_source_prefix}/inflate.c
    ${zlib_source_prefix}/inflate.h
    ${zlib_source_prefix}/inftrees.c
    ${zlib_source_prefix}/inftrees.h
    ${zlib_source_prefix}/trees.c
    ${zlib_source_prefix}/trees.h
    ${zlib_source_prefix}/uncompr.c
    ${zlib_source_prefix}/zconf.h
    ${zlib_source_prefix}/zconf.in.h
    ${zlib_source_prefix}/zlib.h
    ${zlib_source_prefix}/zutil.c
    ${zlib_source_prefix}/zutil.h
)

target_include_directories(zlib
PUBLIC
    ${zlib_source_prefix}/zlib-1.2.3/
)

##########
# libpng
if (ONS_USE_OLD_DEPS)
    set(libpng_source_prefix src/old/libpng-1.2.24)
else()
    set(libpng_source_prefix src/libpng-1.6.9)
endif()

add_library(libpng STATIC "")

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(libpng
    PRIVATE
        -fdiagnostics-color=always
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(libpng
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_link_libraries(libpng
PUBLIC
    zlib
)

target_sources(libpng
PUBLIC
    ${libpng_source_prefix}/png.c
    ${libpng_source_prefix}/png.h
    ${libpng_source_prefix}/pngconf.h
    ${libpng_source_prefix}/pngerror.c
    ${libpng_source_prefix}/pnggccrd.c
    ${libpng_source_prefix}/pngget.c
    ${libpng_source_prefix}/pngmem.c
    ${libpng_source_prefix}/pngpread.c
    ${libpng_source_prefix}/pngread.c
    ${libpng_source_prefix}/pngrio.c
    ${libpng_source_prefix}/pngrtran.c
    ${libpng_source_prefix}/pngrutil.c
    ${libpng_source_prefix}/pngset.c
    ${libpng_source_prefix}/pngtrans.c
    ${libpng_source_prefix}/pngvcrd.c
    ${libpng_source_prefix}/pngwio.c
    ${libpng_source_prefix}/pngwrite.c
    ${libpng_source_prefix}/pngwtran.c
    ${libpng_source_prefix}/pngwutil.c
)

target_include_directories(libpng
PUBLIC
    ${libpng_source_prefix}/
)


##########
# sdl_image
if (ONS_USE_OLD_DEPS)
    set(sdl_image_source_prefix src/old/SDL_image-1.2.6)
else()
    set(sdl_image_source_prefix src/SDL_image-1.2.12)
endif()

add_library(SDL_image STATIC "")

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(SDL_image
    PRIVATE
        -fdiagnostics-color=always
        $<$<CONFIG:DEBUG>:-g>
    )

    target_link_options(SDL_image
    PUBLIC
        $<$<CONFIG:RELEASE>:-s>
    )
endif()

target_compile_definitions(SDL_image 
PUBLIC 
    LOAD_JPG
    LOAD_PNG
    LOAD_BMP
    LOAD_GIF
    LOAD_LBM
    LOAD_PCX
    LOAD_PNM
    LOAD_TGA
)

if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(SDL_image 
    PRIVATE
        -Wno-incompatible-pointer-types
    )
endif()

target_link_libraries(SDL_image
PUBLIC
    PkgConfigWrapper::SDL
    PkgConfigWrapper::libjpeg
    #jpeg
    libpng
)

target_sources(SDL_image
PRIVATE
    ${sdl_image_source_prefix}/IMG.c
    ${sdl_image_source_prefix}/IMG_bmp.c
    ${sdl_image_source_prefix}/IMG_gif.c
    ${sdl_image_source_prefix}/IMG_jpg.c
    ${sdl_image_source_prefix}/IMG_lbm.c
    ${sdl_image_source_prefix}/IMG_pcx.c
    ${sdl_image_source_prefix}/IMG_png.c
    ${sdl_image_source_prefix}/IMG_pnm.c
    ${sdl_image_source_prefix}/IMG_tga.c
    ${sdl_image_source_prefix}/IMG_tif.c
    ${sdl_image_source_prefix}/IMG_xcf.c
    ${sdl_image_source_prefix}/IMG_xpm.c
    ${sdl_image_source_prefix}/IMG_xv.c
    ${sdl_image_source_prefix}/IMG_xxx.c
    ${sdl_image_source_prefix}/SDL_image.h
)

target_include_directories(SDL_image
PUBLIC
    ${sdl_image_source_prefix}/
)
