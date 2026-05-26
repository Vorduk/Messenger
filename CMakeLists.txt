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

if(MSVC)
    add_compile_options(/utf-8)
    add_compile_options(/W0 /wd4996 /wd4244 /wd4267)
endif()

if(MINGW OR CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall -Wextra)
    add_compile_options(-Wno-cast-function-type)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-stringop-overread")
endif()

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

# -------------------------------------------------------------------
# Поиск OpenSSL с автоматическим обнаружением
# -------------------------------------------------------------------
if(WIN32)
    set(OPENSSL_ROOT_DIR "" CACHE PATH "Root directory of OpenSSL installation")
endif()

find_package(OpenSSL REQUIRED)
add_compile_definitions(OPENSSL_FOUND)
message(STATUS "OpenSSL found: ${OPENSSL_INCLUDE_DIR}")

# -------------------------------------------------------------------
# Precompiled headers configuration
# -------------------------------------------------------------------
set(PCH_HEADER "${CMAKE_SOURCE_DIR}/Common/pch.h")
set(PCH_SOURCE "${CMAKE_SOURCE_DIR}/Common/pch.cpp")  # Optional: create an empty .cpp file

# Create empty pch.cpp if it doesn't exist (for MSVC compatibility)
if(NOT EXISTS "${PCH_SOURCE}")
    file(WRITE "${PCH_SOURCE}" "#include \"pch.h\"\n")
endif()

# Function to enable precompiled headers for a target
function(enable_pch TARGET_NAME)
    if(MSVC)
        # Visual Studio: /Yc for pch.cpp, /Yu for all other files
        target_precompile_headers(${TARGET_NAME} PRIVATE ${PCH_HEADER})
    elseif(CMAKE_COMPILER_IS_GNUCXX OR MINGW)
        # GCC/MinGW: Use -include flag to force include
        target_precompile_headers(${TARGET_NAME} PRIVATE ${PCH_HEADER})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # Clang: Native PCH support
        target_precompile_headers(${TARGET_NAME} PRIVATE ${PCH_HEADER})
    endif()
    
    # Add PCH directory to includes
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/Common)
endfunction()

# Function to add precompiled headers to a target with more control
function(target_add_pch TARGET_NAME PCH_FILE)
    get_filename_component(PCH_DIR ${PCH_FILE} DIRECTORY)
    
    if(MSVC)
        # MSVC approach: use target_precompile_headers
        target_precompile_headers(${TARGET_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${PCH_FILE}>")
        target_include_directories(${TARGET_NAME} PRIVATE ${PCH_DIR})
    elseif(MINGW OR CMAKE_COMPILER_IS_GNUCXX)
        # GCC/MinGW approach: compile PCH separately and use it
        get_filename_component(PCH_NAME ${PCH_FILE} NAME_WE)
        set(PCH_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PCH_NAME}.h.gch")
        
        # Create directory for PCH output
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
        
        # Custom command to build PCH
        add_custom_command(
            OUTPUT ${PCH_OUTPUT}
            COMMAND ${CMAKE_CXX_COMPILER} 
                ${CMAKE_CXX_FLAGS}
                -x c++-header
                -I${PCH_DIR}
                -I${JSON_INCLUDE_DIR}
                -o ${PCH_OUTPUT}
                ${PCH_FILE}
            DEPENDS ${PCH_FILE}
            COMMENT "Building precompiled header ${PCH_NAME}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        
        # Create custom target for PCH
        add_custom_target(${TARGET_NAME}_pch DEPENDS ${PCH_OUTPUT})
        add_dependencies(${TARGET_NAME} ${TARGET_NAME}_pch)
        
        # Add include path for PCH
        target_include_directories(${TARGET_NAME} PRIVATE ${PCH_DIR} ${CMAKE_CURRENT_BINARY_DIR})
        
        # Force all source files to depend on PCH
        target_compile_options(${TARGET_NAME} PRIVATE 
            -include ${PCH_DIR}/${PCH_NAME}.h
            -Winvalid-pch
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # Clang: Native PCH support similar to MSVC
        target_precompile_headers(${TARGET_NAME} PRIVATE ${PCH_FILE})
        target_include_directories(${TARGET_NAME} PRIVATE ${PCH_DIR})
    endif()
endfunction()

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
    target_link_libraries(${TARGET_NAME} PRIVATE 
        Common glew glfw imgui spdlog
        OpenSSL::SSL OpenSSL::Crypto
    )
    target_compile_definitions(Client PRIVATE GLEW_STATIC)
    
    # Enable precompiled headers for this target
    enable_pch(${TARGET_NAME})
    
    if(WIN32)
        target_link_libraries(${TARGET_NAME} PRIVATE ws2_32)
                if(MSVC)
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_RUNTIME_DLLS:${TARGET_NAME}>"
                    "$<TARGET_FILE_DIR:${TARGET_NAME}>"
                COMMAND_EXPAND_LISTS
            )
        endif()
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
    target_link_libraries(${TARGET_NAME} PUBLIC 
        glew glfw imgui spdlog
        OpenSSL::SSL OpenSSL::Crypto
    )
    
    # Enable precompiled headers for this target
    enable_pch(${TARGET_NAME})
    
    if(WIN32)
        target_link_libraries(${TARGET_NAME} PUBLIC ws2_32)
    endif()
    set_target_properties(${TARGET_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/lib"
    )
endfunction()

find_package(OpenGL REQUIRED)

add_subdirectory(extern/glew)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(extern/glfw)
add_subdirectory(extern/imgui)
add_subdirectory(extern/sqlite) 
add_subdirectory(extern/spdlog)

add_subdirectory(Common)
add_subdirectory(Client)
add_subdirectory(Server)

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
message(STATUS "OpenGL: ${OPENGL_INCLUDE_DIR}")
message(STATUS "OpenSSL include: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "GLEW: ${CMAKE_CURRENT_SOURCE_DIR}/extern/glew")
message(STATUS "Precompiled header: ${PCH_HEADER}")
message(STATUS "========================================")
message(STATUS "")