cmake_minimum_required(VERSION 3.12)

project(WCCommonLib
        VERSION 1.0
        DESCRIPTION "WCCommon"
        LANGUAGES C CXX
)

if(POLICY CMP0076)
  cmake_policy(SET CMP0076 NEW)
endif()

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(fmt 10 REQUIRED)
find_package(date REQUIRED)

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
    date::date
    fmt::fmt
    yaml-cpp
    Threads::Threads
)
add_library(WCCommon::WCCommon ALIAS WCCommon)

add_library(LogConfig INTERFACE)
target_include_directories(
    LogConfig
INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_definitions(
    LogConfig
INTERFACE
    SPDLOG_FMT_EXTERNAL_HO
)
target_link_libraries(
    LogConfig
INTERFACE
    spdlog::spdlog_header_only
)
add_library(LogConfig::LogConfig ALIAS LogConfig)


#================================================
# Install
#================================================
install(TARGETS WCCommon LogConfig
    EXPORT WCCommonTargets
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)
install(DIRECTORY
    ${CMAKE_SOURCE_DIR}/include/
DESTINATION
    include
)

set(ConfigPackageLocation lib/cmake/WCCommon)
install(EXPORT WCCommonTargets
    FILE WCCommonTargets.cmake
    NAMESPACE WCCommon::
    DESTINATION ${ConfigPackageLocation}
)
install(FILES ${CMAKE_SOURCE_DIR}/cmake/WCCommonConfig.cmake
    DESTINATION ${ConfigPackageLocation}
)
