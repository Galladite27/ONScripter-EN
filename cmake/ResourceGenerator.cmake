message(STATUS "Arg Count: ${CMAKE_ARGC}")

set(i 0)
while(NOT ${i} STREQUAL ${CMAKE_ARGC})
    message(STATUS "Arg${i} ${CMAKE_ARGV${i}}")
    math(EXPR i "${i}+1")
endwhile()

message(STATUS "File name to generate ${CMAKE_ARGV4}")

set(generatedFile "// Generated file - do not edit\n\n#include <string.h>\n#include \"resources.h\"\n\n")

set(arg_counter 5)
set(file_counter 0)
while(NOT ${arg_counter} STREQUAL ${CMAKE_ARGC})
    message(STATUS "File name to blit ${CMAKE_ARGV${arg_counter}}")

    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_ARGV${arg_counter}} currentFileData HEX)
    
    string(REPEAT "[0-9a-f]" 32 column_pattern)
    string(REGEX REPLACE "(${column_pattern})" "\\1\n\t" content "${currentFileData}")
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1, " hexData ${content})

    set(generatedFile "${generatedFile}\nstatic const unsigned char resource_${file_counter}_buffer[] = {\n\t${hexData}\n}\;\n")
    message(STATUS "${CMAKE_ARGV${arg_counter}}")
    math(EXPR arg_counter "${arg_counter}+1")
    math(EXPR file_counter "${file_counter}+1")
endwhile()

set(generatedFile "${generatedFile}\nstatic const InternalResource resource_list[] = {")
set(arg_counter 5)
set(file_counter 0)
while(NOT ${arg_counter} STREQUAL ${CMAKE_ARGC})
    file(SIZE ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_ARGV${arg_counter}} currentFileSize)
    set(generatedFile "${generatedFile}\n\t{ \"${CMAKE_ARGV${arg_counter}}\", resource_${file_counter}_buffer, ${currentFileSize} },")
    
    message(STATUS "${CMAKE_ARGV${arg_counter}}")
    math(EXPR arg_counter "${arg_counter}+1")
    math(EXPR file_counter "${file_counter}+1")
endwhile()

set(generatedFile "${generatedFile}\n\t{ 0, 0, 0 }\n}\;\n
const InternalResource* getResource(const char* filename)
{
	for (const InternalResource* rv = resource_list\; rv->buffer\; ++rv) {
		if (strcmp(rv->filename, filename) == 0) return rv\;
	}
	return NULL\;
}\n\n")

file(WRITE ${CMAKE_ARGV4} ${generatedFile})
