cmake_minimum_required(VERSION 3.12)

project(WCCommonLib
        VERSION 1.0
        DESCRIPTION "WCCommon"
        LANGUAGES C CXX
)

if(POLICY CMP0076)
  cmake_policy(SET CMP0076 NEW)
endif()

# enable c++17 globally
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(fmt REQUIRED)

add_library(WCCommon INTERFACE)
target_include_directories(
    WCCommon
INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(
    WCCommon
INTERFACE
    yaml-cpp
    fmt::fmt
    Threads::Threads
)
add_library(WCCommon::WCCommon ALIAS WCCommon)

add_library(LogConfig)
target_sources(
    LogConfig
PRIVATE
    LogConfig.cpp
)
target_include_directories(
    LogConfig
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(
    LogConfig
PUBLIC
    spdlog::spdlog
PRIVATE
    WCCommon::WCCommon
)
add_library(LogConfig::LogConfig ALIAS LogConfig)
