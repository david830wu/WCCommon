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

add_library(WCCommon)
target_sources(
    WCCommon
PRIVATE
    WCCommon.cpp
)
target_include_directories(
    WCCommon
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(
    WCCommon
PUBLIC
    Threads::Threads
)
add_library(WCCommon::WCCommon ALIAS WCCommon)

install(TARGETS WCCommon DESTINATION lib)
install(FILES ${CMAKE_SOURCE_DIR}/include/WCCommon.h DESTINATION include)
