cmake_minimum_required(VERSION 3.12)

project({{ProjectName}}
        VERSION {{Version}}
        DESCRIPTION "{{Description}}"
        LANGUAGES C CXX
)

if(POLICY CMP0076)
  cmake_policy(SET CMP0076 NEW)
endif()

enable_testing()

# enable c++17 globally
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# find and configure required package
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost 1.70.0 REQUIRED COMPONENTS)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(apps)
