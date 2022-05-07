cmake_minimum_required(VERSION 3.12)

project({{ProjectName}}Lib
        VERSION {{Version}}
        DESCRIPTION "{{ProjectName}}"
        LANGUAGES C CXX
)

if(POLICY CMP0076)
  cmake_policy(SET CMP0076 NEW)
endif()

# enable c++17 globally
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library({{ProjectName}})
target_sources(
    {{ProjectName}}
PRIVATE
    {{ProjectName}}.cpp
)
target_include_directories(
    {{ProjectName}}
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(
    {{ProjectName}}
PUBLIC
    Threads::Threads
    Boost::boost
)
add_library({{ProjectName}}::{{ProjectName}} ALIAS {{ProjectName}})

install(TARGETS {{ProjectName}} DESTINATION lib)
install(FILES ${CMAKE_SOURCE_DIR}/include/{{ProjectName}}.h DESTINATION include)
