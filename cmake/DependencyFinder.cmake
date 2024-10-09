# Various helpers for finding and unifying our dependencies across targets/systems.

find_package(PkgConfig)

# For libraries that can only be found directly querying pkgconfig.
function(PkgConfig_Find_Module module_name)
    pkg_check_modules(${module_name}_TO_FIND ${module_name})

    if (${${module_name}_TO_FIND} MATCHES "-NOTFOUND")
        return()
    endif()
    
    message(STATUS "PkgConfig: ${module_name}")

    if (${ARGC} EQUAL "1")
        set(target_name ${module_name})
    elseif(${ARGC} EQUAL "2")
        set(target_name ${ARGV1})
    endif()

    list(LENGTH ${module_name}_TO_FIND_LIBRARY_DIRS include_dir_length)

    if (${include_dir_length} GREATER 2)
        #message(STATUS "We've encountered a pkg-config library (${module_name}) with more than a debug and release include directory.")
        #foreach(dir ${${module_name}_TO_FIND_LIBRARY_DIRS})
        #    message(STATUS "\t${dir}")
        #endforeach()
    endif()

    # STATIC should be variable here, should detect it somehow.
    add_library(PkgConfigWrapper::${target_name} INTERFACE IMPORTED)
    
    set(${module_name}_TO_FIND_LIBRARY_DIRS_DEBUG ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib)
    set(${module_name}_TO_FIND_LIBRARY_DIRS_RELEASE ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib)

    foreach(library ${${module_name}_TO_FIND_STATIC_LIBRARIES})
        if (TARGET PkgConfigWrapperSubLib::${library})
            continue()
        endif()

        message(STATUS "\t${library}")

        # Get the release library
        unset(${module_name}_TO_FIND_${library}_RELEASE_LIBRARY_FOUND)
        find_library(${module_name}_TO_FIND_${library}_RELEASE_LIBRARY_FOUND NAMES ${library} PATHS ${${module_name}_TO_FIND_LIBRARY_DIRS_RELEASE} NO_CACHE NO_DEFAULT_PATH)

        # Get the debug library
        unset(${module_name}_TO_FIND_${library}_DEBUG_LIBRARY_FOUND)
        find_library(${module_name}_TO_FIND_${library}_DEBUG_LIBRARY_FOUND NAMES ${library} PATHS ${${module_name}_TO_FIND_LIBRARY_DIRS_DEBUG} NO_CACHE NO_DEFAULT_PATH)
        
        if (${${module_name}_TO_FIND_${library}_RELEASE_LIBRARY_FOUND} STREQUAL ${module_name}_TO_FIND_${library}_RELEASE_LIBRARY_FOUND-NOTFOUND)
            #message(STATUS "Giving up and just linking ${library}")
            set_property(TARGET PkgConfigWrapper::${target_name} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${library})
        else()
            add_library(PkgConfigWrapperSubLib::${library} UNKNOWN IMPORTED)
            set_target_properties(PkgConfigWrapperSubLib::${library} PROPERTIES
                IMPORTED_LOCATION ${${module_name}_TO_FIND_${library}_RELEASE_LIBRARY_FOUND}
                IMPORTED_LOCATION_DEBUG ${${module_name}_TO_FIND_${library}_DEBUG_LIBRARY_FOUND}
            )
        
            set_property(TARGET PkgConfigWrapper::${target_name} APPEND PROPERTY INTERFACE_LINK_LIBRARIES PkgConfigWrapperSubLib::${library})
        endif()
    endforeach()

    foreach(include_dir ${${module_name}_TO_FIND_INCLUDE_DIRS})
        if(IS_ABSOLUTE ${include_dir})
            set(processed_include_dir ${include_dir})
        else()
            get_filename_component(processed_include_dir "${include_dir}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
            #message(STATUS "Absolute Include Dir: ${processed_include_dir}")
        endif()
        
        if (EXISTS ${processed_include_dir})
            #message(STATUS "Added Include Dir: ${processed_include_dir}")
            set_property(TARGET PkgConfigWrapper::${target_name} APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${processed_include_dir})
        endif()
    endforeach()
    
    foreach(cflag ${${module_name}_TO_FIND_CFLAGS_OTHER})
        set_property(TARGET PkgConfigWrapper::${target_name} APPEND PROPERTY INTERFACE_COMPILE_OPTIONS ${cflag})
    endforeach()
endfunction()



# For libraries that have config scripts (smpeg-config, sdl-config, and so on, this will run them for us.)
function(run_library_config library_name include_directories)
    set(${library_name}_lib_cmd ${library_name}-config)
    set(lib_args --libs --cflags)

    find_program(${library_name}_lib_command ${${library_name}_lib_cmd})

    if (${${library_name}_lib_command} MATCHES "-NOTFOUND")
        return()
    endif()

    execute_process(
        COMMAND bash ${${library_name}_lib_command} ${lib_args}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE config_result
        OUTPUT_VARIABLE config_stdout_results
        ERROR_VARIABLE config_stderr_results
    )

    #message(STATUS "execute_process command: ${${library_name}_lib_command}")
    #message(STATUS "\targs: ${lib_args}")
    #message(STATUS "\tresults: ${config_stdout_results}")
    #message(STATUS "\terr: ${config_stderr_results}")
    #message(STATUS "\tcode: ${config_result}")

    string(REPLACE " " ";" flags ${config_stdout_results})
    foreach(flag ${flags})
        string(FIND "${flag}" "-I" out)
        if(NOT "${out}" EQUAL 0)
            continue()
        endif()

        string(SUBSTRING ${flag} 2 -1 include_directory)

        if (EXISTS ${include_directory})
            set(processed_include_directory ${include_directory})
        else()
            cmake_path(CONVERT ${${library_name}_lib_command} TO_CMAKE_PATH_LIST path_list)
            string(REPLACE "/" ";" path_list ${path_list})
            #message(STATUS "\tPathList: ${path_list}")
            
            foreach(component ${path_list})
                #message(STATUS "\tcomparing: ${include_directory}, ${component}")
                string(FIND "${include_directory}" "/${component}" out)
                if("${out}" EQUAL 0)
                    set(processed_include_directory ${processed_include_directory}${include_directory})
                    break()
                endif()

                if (component MATCHES ":$")
                    set(component ${component}/)
                endif()
                
                cmake_path(APPEND processed_include_directory ${component})
                #message(STATUS "\tAdded: ${processed_include_directory}")
            endforeach()
        endif()

        list(APPEND include_directories_list ${processed_include_directory})
    endforeach()

    #message(STATUS "\tinclude_directories [${include_directories}]: ${include_directories_list}")
    set(${include_directories} ${include_directories_list} PARENT_SCOPE)
endfunction()