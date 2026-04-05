cmake_minimum_required(VERSION 3.20)
project(OBSPropio VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX- /MP)
    add_compile_definitions(NOMINMAX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find packages
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

# FFmpeg Configuration via vcpkg
find_package(avcodec REQUIRED)
find_package(avformat REQUIRED)
find_package(avutil REQUIRED)
find_package(swscale REQUIRED)
find_package(swresample REQUIRED)

# nlohmann/json (header-only)
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, will download")
    include(FetchContent)
    FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
    FetchContent_MakeAvailable(json)
endif()

# spdlog (logging)
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
    message(STATUS "spdlog not found, will download")
    include(FetchContent)
    FetchContent_Declare(spdlog URL https://github.com/gabime/spdlog/archive/refs/tags/v1.12.0.tar.gz)
    FetchContent_MakeAvailable(spdlog)
endif()

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
)

target_link_libraries(obs_core PUBLIC
    Threads::Threads
    OpenGL::GL
    avcodec::avcodec
    avformat::avformat
    avutil::avutil
    swscale::swscale
    swresample::swresample
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

# Testing (disabled until test files are implemented)
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_input_manager.cpp")
        add_executable(test_input_manager tests/test_input_manager.cpp)
        target_link_libraries(test_input_manager obs_core)
        add_test(NAME InputManagerTest COMMAND test_input_manager)
    endif()
    
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_scene_engine.cpp")
        add_executable(test_scene_engine tests/test_scene_engine.cpp)
        target_link_libraries(test_scene_engine obs_core)
        add_test(NAME SceneEngineTest COMMAND test_scene_engine)
    endif()
    
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_replay.cpp")
        add_executable(test_replay tests/test_replay.cpp)
        target_link_libraries(test_replay obs_core)
        add_test(NAME ReplayTest COMMAND test_replay)
    endif()
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
message(STATUS "FFmpeg found via vcpkg")