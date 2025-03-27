option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" OFF)
add_subdirectory(googletest)

target_compile_options(gtest 
PRIVATE 
    -Wno-maybe-uninitialized
)

include(CTest)
include(GoogleTest)
enable_testing()

function(add_onsen_test target_name)
    add_executable(${target_name})

    if(ONS_COMPILER_FLAG_STYLE STREQUAL "GNU")
        target_compile_options(${target_name}
        PRIVATE
            -fprofile-arcs
            -ftest-coverage
            -O0
            -pthread
        )
        
        target_link_options(${target_name}
        PRIVATE
            --coverage
        )

        if (MINGW)
            target_link_options(${target_name}
            PRIVATE
                -mconsole
            )
        endif()
    endif()

    target_compile_features(${target_name}
    PRIVATE
        cxx_std_98
    )

    target_compile_definitions(${target_name}
    PRIVATE
        GTEST_LANG_CXX11=0
    )

    target_include_directories(${target_name}
    PRIVATE
        ..
    )

    target_link_libraries(${target_name}
    PRIVATE
        gtest
        gtest_main
    )

    gtest_discover_tests(${target_name})
endfunction()

#############################################
# onscripter_encoding_test
add_onsen_test(onscripter_encoding_test)

target_sources(onscripter_encoding_test
PRIVATE
    test_Encoding.cpp
    ../Encoding.cpp
    ../sjis2utf16.cpp
)
