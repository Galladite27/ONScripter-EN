cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(onscripter-en LANGUAGES C CXX)

# Determine what architecture we're targeting.
include(cmake/TargetArch.cmake)
target_architecture(ONS_TARGET_ARCH)
message(STATUS "Target Architecture: ${ONS_TARGET_ARCH}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID}")

# Determine flag style
set(ONS_COMPILER_FLAG_STYLE "GNU")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(ONS_COMPILER_FLAG_STYLE "MSVC")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(ONS_COMPILER_FLAG_STYLE "MSVC")
endif()

if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU" AND (CMAKE_GENERATOR STREQUAL Ninja OR CMAKE_GENERATOR STREQUAL "Ninja Multi-Config"))
    # This should be used very sparingly
    add_compile_options(-fdiagnostics-color=always)
endif()




# Handle Architecture and SIMD Detection and availibility.
set(ONS_X86 0)
set(ONS_X8664 0)
set(ONS_X86_MMX_AVAILIBLE 0)
set(ONS_X86_SSE_AVAILIBLE 0)
set(ONS_X86_SSE2_AVAILIBLE 0)
set(ONS_X86_SSE3_AVAILIBLE 0)
set(ONS_X86_SSSE3_AVAILIBLE 0)
set(ONS_X86_SSE4_1_AVAILIBLE 0)
set(ONS_X86_SSE4_2_AVAILIBLE 0)
set(ONS_X86_AVX_AVAILIBLE 0)
set(ONS_X86_AVX2_AVAILIBLE 0)
set(ONS_PPC 0)
set(ONS_PPC_ALTIVEC_AVAILIBLE 0)
set(ONS_ARM 0)
set(ONS_ARM_NEON_AVAILIBLE 0)
set(ONS_ARM_SVE_AVAILIBLE 0)
set(ONS_ARM_SVE2_AVAILIBLE 0)
set(ONS_MIPS 0)

if (${ONS_TARGET_ARCH} STREQUAL "x86_64")    
    set(ONS_X8664 1)
    
    if (NOT MSVC)
        set(ONS_X86_MMX_AVAILIBLE 1)
    endif()
    
    set(ONS_X86_SSE_AVAILIBLE 1)
    set(ONS_X86_SSE2_AVAILIBLE 1)
    set(ONS_X86_SSE3_AVAILIBLE 1)
    set(ONS_X86_SSSE3_AVAILIBLE 1)
    set(ONS_X86_SSE4_1_AVAILIBLE 1)
    set(ONS_X86_SSE4_2_AVAILIBLE 1)
    set(ONS_X86_AVX_AVAILIBLE 1)
    set(ONS_X86_AVX2_AVAILIBLE 1)
elseif(${ONS_TARGET_ARCH} STREQUAL "i386")
    set(ONS_X86 1)
    
    set(ONS_X86_MMX_AVAILIBLE 1)
    set(ONS_X86_SSE_AVAILIBLE 1)
    
    if (NOT NXDK)
        set(ONS_X86_SSE2_AVAILIBLE 1)
    endif()
elseif(${ONS_TARGET_ARCH} STREQUAL "ppc" OR ${ONS_TARGET_ARCH} STREQUAL "ppc")
    set(ONS_PPC 1)
elseif(${ONS_TARGET_ARCH} STREQUAL "armv9" OR
       ${ONS_TARGET_ARCH} STREQUAL "armv8" OR
       ${ONS_TARGET_ARCH} STREQUAL "armv7" OR
       ${ONS_TARGET_ARCH} STREQUAL "armv6" OR
       ${ONS_TARGET_ARCH} STREQUAL "armv5" OR
       ${ONS_TARGET_ARCH} STREQUAL "arm")
       
    set(ONS_ARM 1)

    set(ONS_ARM_NEON_AVAILIBLE 1)
    if (NOT MSVC)
        set(ONS_ARM_SVE_AVAILIBLE 1)
        set(ONS_ARM_SVE2_AVAILIBLE 1)
    endif()
elseif(${ONS_TARGET_ARCH} STREQUAL "mips")
    set(ONS_MIPS 1)
endif()

# Options we can pass into CMake, platform will determine some defaults.
if (NINTENDO_WIIU
    OR NINTENDO_SWITCH
    OR NXDK)
    option(ONS_USE_SYSTEM_SDL "" ON)
    option(ONS_FORCE_SDLCOMPAT_FULLY_STATIC "" ON)
elseif(MINGW)
    option(ONS_USE_SYSTEM_SDL "" OFF)
    option(ONS_USE_SDLCOMPAT "" OFF)
elseif(NINTENDO_GAMECUBE
       OR NINTENDO_WII
       OR PSP)
    option(ONS_USE_SYSTEM_SDL "" ON)
    option(ONS_USE_SDLCOMPAT "" OFF)
elseif(APPLE)
    option(ONS_USE_SYSTEM_SDL "" OFF)
    option(ONS_USE_SDLCOMPAT "" ON)
endif()

if (WIN32)
    option(ONS_USE_SDLMAIN "" ON)
endif()

if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL ${CMAKE_SYSTEM_NAME})
    option(ONS_HOST_PLATFORM "Platform for which could be feasibly used to develop an ONScripter title, if off, we won't compile tools/tests" ON)
endif()

option(ONS_USE_SYSTEM_SDL "" OFF)
option(ONS_USE_SDLCOMPAT "" ON)
option(ONS_USE_SDLMAIN "" ON)
option(ONS_PREFER_SYSTEM_PACKAGES "Prefer using system packages over our in-tree/submoduled libraries." OFF)
option(ONS_HOST_PLATFORM "Platform for which could be feasibly used to develop an ONScripter title, if off, we won't compile tools/tests" OFF)
option(ONS_PREFER_OLD_DEPS "When building our own local libraries, decide if we'll use the original versions or the newer ones, where possible." ON)
option(ONS_COMPILE_TESTS "Choose whether to compile our tests." ${ONS_HOST_PLATFORM})
option(ONS_COMPILE_TOOLS "Choose whether to compile the creation tools." ${ONS_HOST_PLATFORM})
option(ONS_USE_LIBMAD "Choose whether to use libmad audio decoding" OFF)
option(ONS_FORCE_SDLCOMPAT_FULLY_STATIC "" OFF)


message(STATUS "ONS_USE_SYSTEM_SDL: ${ONS_USE_SYSTEM_SDL}")
message(STATUS "ONS_USE_SDLCOMPAT: ${ONS_USE_SDLCOMPAT}")
message(STATUS "ONS_PREFER_SYSTEM_PACKAGES: ${ONS_PREFER_SYSTEM_PACKAGES}")
message(STATUS "ONS_HOST_PLATFORM: ${ONS_HOST_PLATFORM}")
message(STATUS "ONS_PREFER_OLD_DEPS: ${ONS_PREFER_OLD_DEPS}")
message(STATUS "ONS_COMPILE_TESTS: ${ONS_COMPILE_TESTS}")
message(STATUS "ONS_COMPILE_TOOLS: ${ONS_COMPILE_TOOLS}")
message(STATUS "ONS_USE_LIBMAD: ${ONS_USE_LIBMAD}")
message(STATUS "ONS_FORCE_SDLCOMPAT_FULLY_STATIC: ${ONS_FORCE_SDLCOMPAT_FULLY_STATIC}")

# A CMake implementation of embed.cpp, which we can't rely on due to a limitation in CMake's cross compilation capabilities.
# (Essentially when cross compiling, we'd need to mix compilation target architectures. Embed.cpp would need to be compiled
# for the host platform, while ONScripter (and maybe the tools) would need to target some completely different arch.
# this is something for which CMake isn't designed. There's some hacks around this, but it's easier to just write a CMake 
# script to implement the functionality, even if it's quite a bit slower.)
set(EMBED_COMMAND ${CMAKE_COMMAND} -P cmake/ResourceGenerator.cmake -- ${CMAKE_CURRENT_BINARY_DIR}/resources.cpp ons-en.ico )
set(ADDITIONAL_EMBED_DEPENDENCIES "")

add_custom_command(
    OUTPUT resources.cpp
    COMMAND ${EMBED_COMMAND}
    DEPENDS ${ADDITIONAL_EMBED_DEPENDENCIES} ${CMAKE_CURRENT_SOURCE_DIR}/ons-en.ico
    COMMENT "This command generates resources.cpp to embed files into onscripter-en, such as the icon."
    VERBATIM
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

#############################################
# Dependencies
#include(extlib/CMakeLists.txt)
add_subdirectory(extlib)

#############################################
# Tools
if(ONS_COMPILE_TOOLS AND NOT MSVC AND NOT NXDK)
    add_subdirectory(tools)
endif()

#############################################
# Tests
if(ONS_COMPILE_TESTS AND NOT MSVC AND NOT NXDK)
    add_subdirectory(test)
endif()

#############################################
# Configure version.h

set(I \"ons-en.ico\")

string(TIMESTAMP Y %Y)
string(TIMESTAMP M %m)
string(TIMESTAMP D %d)
string(TIMESTAMP 0M %m)
string(TIMESTAMP 0D %d)

# trim off the leading 0 if the month/day has one.
string(REGEX REPLACE "^0" "" M ${M})
string(REGEX REPLACE "^0" "" D ${D})

configure_file(onscripter.rc.in onscripter.rc
    @ONLY
)

configure_file(winres.h.in winres.h
    @ONLY
)

configure_file(version.h.in version.h
    @ONLY
)

#############################################
# ONScripter-EN
add_executable(onscripter-en)

target_include_directories(onscripter-en 
PRIVATE 
    .
    ${CMAKE_CURRENT_BINARY_DIR}
    ${smpeg_include_directories}
)

target_compile_definitions(onscripter-en 
PRIVATE 
    _GNU_SOURCE=1
)

target_compile_features(onscripter-en 
PUBLIC 
    cxx_std_98
)

target_sources(onscripter-en
PRIVATE
    AnimationInfo.cpp
    AnimationInfo.h
#    AVIWrapper.cpp
#    AVIWrapper.h
    BaseReader.h
    CMakeLists.txt
    DirectReader.cpp
    DirectReader.h
    DirPaths.cpp
    DirPaths.h
    DirtyRect.cpp
    DirtyRect.h
    Encoding.cpp
    Encoding.h
    FontInfo.cpp
    FontInfo.h
    graphics_altivec.cpp
    graphics_altivec.h
    graphics_blend.h
    graphics_common.h
    graphics_cpu.h
    graphics_mmx.cpp
    graphics_mmx.h
    graphics_resize.h
    graphics_routines.cpp
    graphics_sse2.cpp
    graphics_sse2.h
    graphics_sum.h
    Layer.cpp
    Layer.h
    #nscriptdecode.cpp
    NsaReader.cpp
    NsaReader.h
    onscripter.cpp
    ONScripterLabel_animation.cpp
    ONScripterLabel_command.cpp
    ONScripterLabel_effect_breakup.cpp
    ONScripterLabel_effect_cascade.cpp
    ONScripterLabel_effect_trig.cpp
    ONScripterLabel_effect.cpp
    ONScripterLabel_event.cpp
    ONScripterLabel_file.cpp
    ONScripterLabel_file2.cpp
    ONScripterLabel_image.cpp
    ONScripterLabel_rmenu.cpp
    ONScripterLabel_sound.cpp
    ONScripterLabel_text.cpp
    ONScripterLabel.cpp
    ONScripterLabel.h
    ONScripterReporter.cpp
    ONScripterReporter.h
    resize_image.cpp
    resize_image.h
    resources.cpp
    resources.h
    SarReader.cpp
    SarReader.h
    ScriptHandler.cpp
    ScriptHandler.h
    ScriptParser_command.cpp
    ScriptParser.cpp
    ScriptParser.h
    ScriptParser_command.cpp
    sjis2utf16.cpp
    version.h
)

# Default Compiler Options
if (ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(onscripter-en
    PRIVATE
        -fdiagnostics-color=always
        -fpermissive
        $<$<CONFIG:DEBUG>:-g>
    )
    
    if (MINGW)
        target_link_options(onscripter-en
        PUBLIC
            $<$<CONFIG:RELEASE>:-s>
            -static
        )
    endif()
elseif(ONS_COMPILER_FLAG_STYLE STREQUAL "MSVC")
    target_compile_definitions(onscripter-en
        PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )

    target_compile_options(onscripter-en 
    PRIVATE
        /W4
    )
endif()

# Libraries
target_link_libraries(onscripter-en
PUBLIC
    ONS::SDL
    ONS::ogg
    ONS::vorbis
    ONS::SDL_mixer
    ONS::SDL_ttf
    ONS::SDL_image
    ONS::bzip2
    ONS::freetype
    ONS::smpeg
)

if (ONS_USE_LIBMAD)
    target_link_libraries(onscripter-en
    PUBLIC
        ONS::mad
    )
endif()
if (ONS_X8664)
    if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
        set_source_files_properties(
            graphics_sse2.cpp
            COMPILE_OPTIONS -msse2
        )
        
        set_source_files_properties(
            graphics_mmx.cpp
            COMPILE_OPTIONS -mmmx
        )

        target_compile_definitions(onscripter-en PRIVATE USE_X86_GFX)
    endif()
elseif(ONS_X86)
    if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
        set_source_files_properties(
            graphics_sse2.cpp
            COMPILE_OPTIONS -msse2
        )
        
        set_source_files_properties(
            graphics_mmx.cpp
            COMPILE_OPTIONS -mmmx
        )

        target_compile_definitions(onscripter-en PRIVATE USE_X86_GFX)
    endif()
elseif(ONS_PPC)
    if (NOT NINTENDO_GAMECUBE
        AND NOT NINTENDO_WII
        AND NOT NINTENDO_WIIU)

        set(ONS_PPC_ALTIVEC_AVAILIBLE 1)
        
        set_source_files_properties(
            graphics_altivec.cpp
            COMPILE_OPTIONS -maltivec
        )
        
        target_compile_definitions(onscripter-en PRIVATE USE_PPC_GFX)
    endif()
endif()

target_compile_definitions(onscripter-en
PRIVATE
    ONS_X86=${ONS_X86}
    ONS_X8664=${ONS_X8664}
    ONS_X86_MMX_AVAILIBLE=${ONS_X86_MMX_AVAILIBLE}
    ONS_X86_SSE_AVAILIBLE=${ONS_X86_SSE_AVAILIBLE}
    ONS_X86_SSE2_AVAILIBLE=${ONS_X86_SSE2_AVAILIBLE}
    ONS_X86_SSE3_AVAILIBLE=${ONS_X86_SSE3_AVAILIBLE}
    ONS_X86_SSSE3_AVAILIBLE=${ONS_X86_SSSE3_AVAILIBLE}
    ONS_X86_SSE4_1_AVAILIBLE=${ONS_X86_SSE4_1_AVAILIBLE}
    ONS_X86_SSE4_2_AVAILIBLE=${ONS_X86_SSE4_2_AVAILIBLE}
    ONS_X86_AVX_AVAILIBLE=${ONS_X86_AVX_AVAILIBLE}
    ONS_X86_AVX2_AVAILIBLE=${ONS_X86_AVX2_AVAILIBLE}
    ONS_PPC=${ONS_PPC}
    ONS_PPC_ALTIVEC_AVAILIBLE=${ONS_PPC_ALTIVEC_AVAILIBLE}
    ONS_ARM=${ONS_ARM}
    ONS_ARM_NEON_AVAILIBLE=${ONS_ARM_NEON_AVAILIBLE}
    ONS_ARM_SVE_AVAILIBLE=${ONS_ARM_SVE_AVAILIBLE}
    ONS_ARM_SVE2_AVAILIBLE=${ONS_ARM_SVE2_AVAILIBLE}
    ONS_MIPS=${ONS_MIPS}
)

message(STATUS "ONS_X86=${ONS_X86}")
message(STATUS "ONS_X8664=${ONS_X8664}")
message(STATUS "ONS_X86_MMX_AVAILIBLE=${ONS_X86_MMX_AVAILIBLE}")
message(STATUS "ONS_X86_SSE_AVAILIBLE=${ONS_X86_SSE_AVAILIBLE}")
message(STATUS "ONS_X86_SSE2_AVAILIBLE=${ONS_X86_SSE2_AVAILIBLE}")
message(STATUS "ONS_X86_SSE3_AVAILIBLE=${ONS_X86_SSE3_AVAILIBLE}")
message(STATUS "ONS_X86_SSSE3_AVAILIBLE=${ONS_X86_SSSE3_AVAILIBLE}")
message(STATUS "ONS_X86_SSE4_1_AVAILIBLE=${ONS_X86_SSE4_1_AVAILIBLE}")
message(STATUS "ONS_X86_SSE4_2_AVAILIBLE=${ONS_X86_SSE4_2_AVAILIBLE}")
message(STATUS "ONS_X86_AVX_AVAILIBLE=${ONS_X86_AVX_AVAILIBLE}")
message(STATUS "ONS_X86_AVX2_AVAILIBLE=${ONS_X86_AVX2_AVAILIBLE}")
message(STATUS "ONS_PPC=${ONS_PPC}")
message(STATUS "ONS_PPC_ALTIVEC_AVAILIBLE=${ONS_PPC_ALTIVEC_AVAILIBLE}")
message(STATUS "ONS_ARM=${ONS_ARM}")
message(STATUS "ONS_ARM_NEON_AVAILIBLE=${ONS_ARM_NEON_AVAILIBLE}")
message(STATUS "ONS_ARM_SVE_AVAILIBLE=${ONS_ARM_SVE_AVAILIBLE}")
message(STATUS "ONS_ARM_SVE2_AVAILIBLE=${ONS_ARM_SVE2_AVAILIBLE}")
message(STATUS "ONS_MIPS=${ONS_MIPS}")

# Platform Specific Options
if(PSP)
    create_pbp_file(
        TARGET onscripter-en
        ICON_PATH NULL
        BACKGROUND_PATH NULL
        PREVIEW_PATH NULL
        TITLE onscripter-en
        VERSION 01.00
    )

    target_compile_definitions(onscripter-en
    PRIVATE
        PSP
        PDA_WIDTH=360
        BPP16
        USE_OGG_VORBIS
        INTEGER_OGG_VORBIS
    )

    target_link_libraries(onscripter-en 
    PUBLIC
        pspge
        pspvfpu
        pspgum_vfpu
        pspgu
        pspctrl
        pspdebug
        pspvram
        psppower
        pspdisplay
        pspaudiolib
        pspaudio
        pspnet
        pspnet_apctl
        pspnet_resolver
        psprtc
        psputility
    )
    target_compile_options(onscripter-en PUBLIC -g)

elseif(NINTENDO_WIIU)
    configure_file(${CMAKE_SOURCE_DIR}/ons-en.png ${CMAKE_BINARY_DIR}/icon.png COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/cmake/wiiu/meta.xml.in ${CMAKE_BINARY_DIR}/meta.xml)
    wut_create_rpx(onscripter-en)
    wut_create_wuhb(onscripter-en
        CONTENT
        NAME "ONScripter-EN Version 1.00"
        SHORTNAME "ONScripter-EN"
        AUTHOR "Galladite"
        ICON "${CMAKE_SOURCE_DIR}/ons-en.png"
        TVSPLASH "${CMAKE_SOURCE_DIR}/ons-en.png"
        DRCSPLASH "${CMAKE_SOURCE_DIR}/ons-en.png"
    )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL NintendoWii)
    configure_file(${CMAKE_SOURCE_DIR}/ons-en.png ${CMAKE_BINARY_DIR}/package/icon.png COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/cmake/wii/meta.xml.in ${CMAKE_BINARY_DIR}/package/meta.xml)

    ogc_create_dol(onscripter-en)
    add_custom_command(TARGET onscripter-en POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                        ${CMAKE_BINARY_DIR}/onscripter-en.dol
                        ${CMAKE_BINARY_DIR}/package/boot.dol
                    COMMENT "Copying onscripter-en executable into package")

elseif(${CMAKE_SYSTEM_NAME} STREQUAL NintendoGameCube)
    target_compile_definitions(onscripter-en
    PRIVATE
        BPP16
    )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL CafeOS)
elseif(NXDK)
elseif(WIN32)
    target_sources(onscripter-en
    PRIVATE
        onscripter.rc
        winres.h
    )

    if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
        target_compile_options(onscripter-en
        PRIVATE
            -fpermissive
            -mwindows
        )

        target_link_options(onscripter-en
        PRIVATE
        )
    endif()

    target_compile_definitions(onscripter-en
    PRIVATE
        WIN32
        USE_MESSAGEBOX
        USE_OGG_VORBIS
    )

    target_link_libraries(onscripter-en
    PRIVATE
        #iconv
        #m
        user32
        gdi32
        winmm
        dxguid
        rpcrt4
    )
endif()