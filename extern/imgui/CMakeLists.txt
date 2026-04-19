cmake_minimum_required(VERSION 3.20)
set(IMGUI_SOURCES
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
)

if(TARGET glfw)
    list(APPEND IMGUI_SOURCES
        backends/imgui_impl_glfw.cpp
        backends/imgui_impl_opengl3.cpp
    )
endif()

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/backends>
)

if(TARGET glfw)
    target_link_libraries(imgui PUBLIC glfw)
endif()
if(TARGET OpenGL::GL)
    target_link_libraries(imgui PUBLIC OpenGL::GL)
endif()

if(WIN32)
    target_compile_definitions(imgui PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

set_target_properties(imgui PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)