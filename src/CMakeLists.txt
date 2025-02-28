cmake_minimum_required(VERSION 3.12)

project(WCCommonLib
        VERSION 1.7.0
        DESCRIPTION "WCCommon"
        LANGUAGES C CXX
)

if(POLICY CMP0076)
  cmake_policy(SET CMP0076 NEW)
endif()

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

find_package(spdlog REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(fmt REQUIRED)
find_package(date REQUIRED)

if(NOT HDF5_FOUND)
    message(STATUS "H5IO is not available since Hdf5 libs is missing")
endif()

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
    yaml-cpp::yaml-cpp
    fmt::fmt
    Threads::Threads
)
add_library(WCCommon::WCCommon ALIAS WCCommon)

if(Boost_FOUND)
    message(STATUS "Boost is found, LogConfig will be configured")
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
        SPDLOG_FMT_EXTERNAL
    )
    target_link_libraries(
        LogConfig
    INTERFACE
        spdlog::spdlog
        Boost::headers
    )
    add_library(LogConfig::LogConfig ALIAS LogConfig)
else()
    message(WARNING "Boost is missing, LogConfig will not be ignored")
endif()


#================================================
# Install
#================================================
if(Boost_FOUND)
    message(STATUS "Boost is found, LogConfig will be installed")
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
else()
    message(WARNING "Boost is missing, LogConfig will not be installed")
    install(TARGETS WCCommon
        EXPORT WCCommonTargets
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include
    )
    install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/include/
    DESTINATION
        include
    PATTERN "Def*.h" EXCLUDE
    PATTERN "*Log*.h" EXCLUDE
    )
endif()

set(ConfigPackageLocation lib/cmake/WCCommon)
install(EXPORT WCCommonTargets
    FILE WCCommonTargets.cmake
    NAMESPACE WCCommon::
    DESTINATION ${ConfigPackageLocation}
)
install(FILES ${CMAKE_SOURCE_DIR}/cmake/WCCommonConfig.cmake
    DESTINATION ${ConfigPackageLocation}
)
