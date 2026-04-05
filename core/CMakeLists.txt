cmake_minimum_required(VERSION 3.20)
project(OBSPropio VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags - be less strict for CI builds
if(MSVC)
    add_compile_options(/W3 /MP)
    add_compile_definitions(NOMINMAX;_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)
else()
    add_compile_options(-Wall -Wno-unused-parameter)
endif()

# Find packages
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

# FFmpeg Configuration for Windows
if(WIN32)
    if(DEFINED ENV{FFMPEG_ROOT})
        set(FFMPEG_ROOT "$ENV{FFMPEG_ROOT}")
        set(FFMPEG_INCLUDE_DIRS "$ENV{FFMPEG_INCLUDE_DIR}")
        set(FFMPEG_LIB_DIR "$ENV{FFMPEG_LIB_DIR}")
        
        set(FFMPEG_LIBRARIES 
            "${FFMPEG_LIB_DIR}/avcodec.lib"
            "${FFMPEG_LIB_DIR}/avformat.lib"
            "${FFMPEG_LIB_DIR}/avutil.lib"
            "${FFMPEG_LIB_DIR}/swscale.lib"
            "${FFMPEG_LIB_DIR}/swresample.lib"
        )
        
        message(STATUS "Using explicit FFmpeg paths for Windows:")
        message(STATUS "  Include: ${FFMPEG_INCLUDE_DIRS}")
        message(STATUS "  Libs: ${FFMPEG_LIBRARIES}")
    else()
        message(FATAL_ERROR "FFMPEG_ROOT environment variable not set. Please install FFmpeg.")
    endif()
else()
    # Linux/Mac - use pkg-config
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale libswresample)
    else()
        message(FATAL_ERROR "FFmpeg not found and pkg-config not available.")
    endif()
endif()

# nlohmann/json (header-only) - fetch from github
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# spdlog (logging) - fetch from github
FetchContent_Declare(
    spdlog
    URL https://github.com/gabime/spdlog/releases/download/v1.12.0/spdlog-1.12.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(spdlog)

# Core library
add_library(obs_core STATIC
    src/input_manager.cpp
    src/video_source.cpp
    src/decoder.cpp
    src/scene_engine.cpp
    src/switching_engine.cpp
    src/overlay_engine.cpp
    src/output_engine.cpp
    src/replay_engine.cpp
    src/hardware_detector.cpp
    src/frame.cpp
    src/logger.cpp
)

target_include_directories(obs_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${FFMPEG_INCLUDE_DIRS}
)

target_link_libraries(obs_core PUBLIC
    Threads::Threads
    OpenGL::GL
    ${FFMPEG_LIBRARIES}
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)

# Platform-specific libs
if(WIN32)
    target_link_libraries(obs_core PUBLIC mfplat mfuuid strmiids)
elseif(APPLE)
    find_library(FOUNDATION_FRAMEWORK Foundation)
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
    target_link_libraries(obs_core PUBLIC ${FOUNDATION_FRAMEWORK} ${COREFOUNDATION_FRAMEWORK})
else()
    find_package(X11 REQUIRED)
    target_link_libraries(obs_core PUBLIC X11::X11)
endif()

# Installation
install(TARGETS obs_core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
)

# Version info
message(STATUS "OBS Propio v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "FFmpeg include: ${FFMPEG_INCLUDE_DIRS}")
message(STATUS "FFmpeg libs: ${FFMPEG_LIBRARIES}")