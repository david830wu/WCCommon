cmake_minimum_required(VERSION 3.12)

project(WCCommon
        VERSION 1.0
        DESCRIPTION "Frequently used small tools and functions"
        LANGUAGES C CXX
)

if(POLICY CMP0076)
  cmake_policy(SET CMP0076 NEW)
endif()

enable_testing()

# set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
# set(BUILD_SHARED_LIBS OFF)
# set(CMAKE_EXE_LINKER_FLAGS "-static")

# enable c++20 globally
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# find and configure required package
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(${CMAKE_SOURCE_DIR}/cmake/FindHdf5.cmake)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(apps)
