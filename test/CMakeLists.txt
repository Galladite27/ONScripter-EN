option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" OFF)
add_subdirectory(googletest)

include(CTest)
include(GoogleTest)
enable_testing()

#############################################
# onscripter_encoding_test
add_executable(onscripter_encoding_test)

target_sources(onscripter_encoding_test
PRIVATE
    test_Encoding.cpp
    ../Encoding.cpp
    ../sjis2utf16.cpp
)
if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
    target_compile_options(onscripter_encoding_test
    PRIVATE
        -fprofile-arcs
        -ftest-coverage
        -O0
        -pthread
    )
    
    target_link_options(onscripter_encoding_test
    PRIVATE
        -lgcov
        --coverage
        -mconsole
    )
endif()

target_compile_features(onscripter_encoding_test
PRIVATE
    cxx_std_98
)

target_compile_definitions(onscripter_encoding_test
PRIVATE
    GTEST_LANG_CXX11=0
)

target_include_directories(onscripter_encoding_test
PRIVATE
    ..
)

target_link_libraries(onscripter_encoding_test
PRIVATE
    gtest
    gtest_main
)

gtest_discover_tests(onscripter_encoding_test)