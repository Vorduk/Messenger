cmake_minimum_required(VERSION 3.20)

add_library(glew STATIC
    src/glew.c
)

set_target_properties(glew PROPERTIES
    COMPILE_DEFINITIONS "GLEW_STATIC"
    PUBLIC_HEADER "include/GL/glew.h;include/GL/wglew.h;include/GL/glxew.h"
)

target_include_directories(glew
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(OpenGL REQUIRED)
target_link_libraries(glew PUBLIC OpenGL::GL)

if(WIN32)
    target_compile_definitions(glew PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(glew PUBLIC opengl32)
endif()

set_target_properties(glew PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)