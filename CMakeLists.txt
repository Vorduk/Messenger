cmake_minimum_required(VERSION 3.20)

function(get_current_date)
    if(WIN32)
        execute_process(
            COMMAND powershell -Command "Get-Date -Format 'yyyy.MM.dd'"
            OUTPUT_VARIABLE CURRENT_DATE
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    else()
        execute_process(
            COMMAND date "+%Y.%m.%d"
            OUTPUT_VARIABLE CURRENT_DATE
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()
    set(CURRENT_DATE ${CURRENT_DATE} PARENT_SCOPE)
endfunction()

get_current_date()
project(Messenger VERSION ${CURRENT_DATE})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

set(OUTPUT_BASE "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")

if(WIN32)
    set(OUTPUT_BASE "${OUTPUT_BASE}/x64")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE}")

foreach(CFG DEBUG RELEASE MINSIZEREL RELWITHDEBINFO)
    string(TOUPPER ${CFG} CFG_UPPER)
    if(WIN32)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_SOURCE_DIR}/bin/${CFG}/x64")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_SOURCE_DIR}/bin/${CFG}/x64")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_SOURCE_DIR}/bin/${CFG}/x64")
    else()
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_SOURCE_DIR}/bin/${CFG}")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_SOURCE_DIR}/bin/${CFG}")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG_UPPER} "${CMAKE_SOURCE_DIR}/bin/${CFG}")
    endif()
endforeach()

if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0601)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    add_compile_definitions(NOMINMAX)
endif()

set(JSON_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/extern/nlohmann")
if(NOT EXISTS "${JSON_INCLUDE_DIR}/json.hpp")
    message(FATAL_ERROR "json.hpp not found in ${JSON_INCLUDE_DIR}. Please copy single_include/nlohmann/json.hpp there.")
endif()

function(auto_add_executable TARGET_NAME)
    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    )
    add_executable(${TARGET_NAME} ${SOURCES})
    target_include_directories(${TARGET_NAME} PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${JSON_INCLUDE_DIR}
    )
    target_link_libraries(${TARGET_NAME} PRIVATE Common)
    if(WIN32)
        target_link_libraries(${TARGET_NAME} PRIVATE ws2_32)
    endif()
    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}"
    )
endfunction()

function(auto_add_library TARGET_NAME)
    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    )
    add_library(${TARGET_NAME} STATIC ${SOURCES})
    target_include_directories(${TARGET_NAME} PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${JSON_INCLUDE_DIR}
    )
    if(WIN32)
        target_link_libraries(${TARGET_NAME} PUBLIC ws2_32)
    endif()
    set_target_properties(${TARGET_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/lib"
    )
endfunction()

add_subdirectory(Common)
add_subdirectory(Client)
add_subdirectory(Server)
add_subdirectory(extern/sqlite)

message(STATUS "")
message(STATUS "========================================")
message(STATUS "Messenger v${PROJECT_VERSION}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(WIN32)
    message(STATUS "Output: ${OUTPUT_BASE}/x64")
else()
    message(STATUS "Output: ${OUTPUT_BASE}")
endif()
message(STATUS "JSON include: ${JSON_INCLUDE_DIR}")
message(STATUS "========================================")
message(STATUS "")