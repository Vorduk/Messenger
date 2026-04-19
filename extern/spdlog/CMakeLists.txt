cmake_minimum_required(VERSION 3.20)

add_library(spdlog INTERFACE)

target_include_directories(spdlog
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(spdlog INTERFACE cxx_std_11)

# target_compile_definitions(spdlog INTERFACE SPDLOG_FMT_EXTERNAL=OFF)

# target_compile_definitions(spdlog INTERFACE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)