##########
# libgnurx
add_library(gnurx STATIC)

target_sources(gnurx
PRIVATE
    libgnurx/regex_internal.h
    libgnurx/regex.c
    libgnurx/regex.h
)

target_include_directories(gnurx
PUBLIC
    libgnurx/
)


##########
# batchconv
add_executable(batchconv)


target_link_options(batchconv
PUBLIC
    -static
    -s
)

target_compile_features(batchconv PUBLIC cxx_std_98)
target_sources(batchconv 
PRIVATE 
    batchconv.cpp
    conv_shared.cpp
    conv_shared_jpeg.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../SarReader.cpp
    ../SarReader.h
    ../resize_image.cpp
)

target_link_libraries(batchconv
PRIVATE
    gnurx
    bzip2
    PkgConfigWrapper::libjpeg
    libpng
    zlib
)

target_include_directories(batchconv
PUBLIC
    ../
)


##########
# nbzdec
add_executable(nbzdec)


target_link_options(nbzdec
PUBLIC
    -static
    -s
)

target_include_directories(nbzdec PRIVATE ../)

target_compile_features(nbzdec PUBLIC cxx_std_98)
target_sources(nbzdec 
PRIVATE 
    nbzdec.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../SarReader.cpp
    ../SarReader.h
)

target_compile_definitions(nbzdec PUBLIC -DTOOLS_BUILD -DSAR)

target_link_libraries(nbzdec
PRIVATE
    gnurx
    bzip2
    PkgConfigWrapper::libjpeg
    libpng
    zlib
)

##########
# nscdec
add_executable(nscdec)


target_link_options(nscdec
PUBLIC
    -static
    -s
)

target_include_directories(nscdec PRIVATE ../)

target_compile_features(nscdec PUBLIC cxx_std_98)
target_sources(nscdec 
PRIVATE 
    nscdec.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../SarReader.cpp
    ../SarReader.h
)

target_compile_definitions(nscdec PUBLIC -DTOOLS_BUILD -DSAR)

target_link_libraries(nscdec
PRIVATE
    gnurx
    bzip2
    PkgConfigWrapper::libjpeg
    libpng
    zlib
)


##########
# nscmake
add_executable(nscmake)


target_link_options(nscmake
PUBLIC
    -static
    -s
)

target_include_directories(nscmake PRIVATE ../)

target_compile_features(nscmake PUBLIC cxx_std_98)
target_sources(nscmake 
PRIVATE 
    nscmake.cpp
)

target_compile_definitions(nscmake PUBLIC -DTOOLS_BUILD -DSAR)

####################
# Archive Tools

###############
# SAR Tools

##########
# sarconv
add_executable(sarconv)


target_link_options(sarconv
PUBLIC
    -static
    -s
)

target_include_directories(sarconv PRIVATE ../)

target_compile_features(sarconv PUBLIC cxx_std_98)
target_sources(sarconv 
PRIVATE 
    arcconv.cpp
    conv_shared.cpp
    conv_shared_jpeg.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../resize_image.cpp
    ../resize_image.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(sarconv
PRIVATE
    bzip2
    PkgConfigWrapper::libjpeg
    libpng
    zlib
    gnurx
)

target_compile_definitions(sarconv PUBLIC -DTOOLS_BUILD -DSAR)

##########
# sardec
add_executable(sardec)


target_link_options(sardec
PUBLIC
    -static
    -s
)

target_include_directories(sardec PRIVATE ../)

target_compile_features(sardec PUBLIC cxx_std_98)
target_sources(sardec 
PRIVATE 
    arcdec.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(sardec
PRIVATE
    bzip2
    PkgConfigWrapper::libjpeg
    libpng
    zlib
)

target_compile_definitions(sardec PUBLIC -DTOOLS_BUILD -DSAR)

##########
# sarmake
add_executable(sarmake)


target_link_options(sarmake
PUBLIC
    -static
    -s
)

if(MSVC)
    target_include_directories(sarmake PRIVATE ../ ${DIRENT_INCLUDE_DIRS})
else()
    target_include_directories(sarmake PRIVATE ../)
endif()

target_compile_features(sarmake PUBLIC cxx_std_98)
target_sources(sarmake 
PRIVATE 
    arcmake.cpp
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(sarmake
PRIVATE
    bzip2
    PkgConfigWrapper::libjpeg
    libpng
    zlib
)

target_compile_definitions(sarmake PUBLIC -DTOOLS_BUILD -DSAR)


###############
# NS2 Tools

##########
# ns2conv
add_executable(ns2conv)


target_link_options(ns2conv
PUBLIC
    -static
    -s
)

target_include_directories(ns2conv PRIVATE ../)

target_compile_features(ns2conv PUBLIC cxx_std_98)
target_sources(ns2conv 
PRIVATE 
    arcconv.cpp
    conv_shared.cpp
    conv_shared_jpeg.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../resize_image.cpp
    ../resize_image.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(ns2conv
PRIVATE
    bzip2
    PkgConfigWrapper::libjpeg
    libpng
    zlib
    gnurx
)

target_compile_definitions(ns2conv PUBLIC -DTOOLS_BUILD -DNS2)

##########
# ns2dec
add_executable(ns2dec)


target_link_options(ns2dec
PUBLIC
    -static
    -s
)

target_include_directories(ns2dec PRIVATE ../)

target_compile_features(ns2dec PUBLIC cxx_std_98)
target_sources(ns2dec 
PRIVATE 
    arcdec.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(ns2dec
PRIVATE
    bzip2
    PkgConfigWrapper::libjpeg
    libpng
    zlib
)

target_compile_definitions(ns2dec PUBLIC -DTOOLS_BUILD -DNS2)

##########
# ns2make
add_executable(ns2make)


target_link_options(ns2make
PUBLIC
    -static
    -s
)

if(MSVC)
    target_include_directories(ns2make PRIVATE ../ ${DIRENT_INCLUDE_DIRS})
else()
    target_include_directories(ns2make PRIVATE ../)
endif()

target_compile_features(ns2make PUBLIC cxx_std_98)
target_sources(ns2make 
PRIVATE 
    arcmake.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(ns2make
PRIVATE
    bzip2
    PkgConfigWrapper::libjpeg
    libpng
    zlib
)

target_compile_definitions(ns2make PUBLIC -DTOOLS_BUILD -DNS2)

###############
# NSA Tools

##########
# nsaconv
add_executable(nsaconv)


target_link_options(nsaconv
PUBLIC
    -static
    -s
)

target_include_directories(nsaconv PRIVATE ../)

target_compile_features(nsaconv PUBLIC cxx_std_98)
target_sources(nsaconv 
PRIVATE 
    arcconv.cpp
    conv_shared.cpp
    conv_shared_jpeg.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../resize_image.cpp
    ../resize_image.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(nsaconv
PRIVATE
    bzip2
    PkgConfigWrapper::libjpeg
    libpng
    zlib
    gnurx
)

target_compile_definitions(nsaconv PUBLIC -DTOOLS_BUILD -DNSA)

##########
# nsadec
add_executable(nsadec)


target_link_options(nsadec
PUBLIC
    -static
    -s
)

target_include_directories(nsadec PRIVATE ../)

target_compile_features(nsadec PUBLIC cxx_std_98)
target_sources(nsadec 
PRIVATE 
    arcdec.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(nsadec
PRIVATE
    bzip2
    PkgConfigWrapper::libjpeg
    libpng
    zlib
)

target_compile_definitions(nsadec PUBLIC -DTOOLS_BUILD -DNSA)

##########
# nsamake
add_executable(nsamake)


target_link_options(nsamake
PUBLIC
    -static
    -s
)

if(MSVC)
    target_include_directories(nsamake PRIVATE ../ ${DIRENT_INCLUDE_DIRS})
else()
    target_include_directories(nsamake PRIVATE ../)
endif()

target_compile_features(nsamake PUBLIC cxx_std_98)
target_sources(nsamake 
PRIVATE 
    arcmake.cpp
    ../NsaReader.cpp
    ../NsaReader.h
    ../DirPaths.cpp
    ../DirPaths.h
    ../DirectReader.cpp
    ../DirectReader.h
    ../SarReader.cpp
    ../SarReader.h
)

target_link_libraries(nsamake
PRIVATE
    bzip2
    PkgConfigWrapper::libjpeg
    libpng
    zlib
)

target_compile_definitions(nsamake PUBLIC -DTOOLS_BUILD -DNSA)