cmake_minimum_required(VERSION 3.12)

project(WCCommonTest
        VERSION 1.0
        DESCRIPTION "WCCommon test cases"
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

find_package(Catch2 REQUIRED)

# list and compile executable
list(APPEND target_tests "NumericTimeTest")
list(APPEND target_tests "LogConfigTest")
list(APPEND target_tests "ProgressBarTest")
list(APPEND target_tests "YAMLGetFieldTest")

if ("NumericTimeTest" IN_LIST target_tests)
    set(test_name "NumericTimeTest")
    add_executable(${test_name})
    target_sources(${test_name} PUBLIC ${test_name}.cpp)
    target_link_libraries(${test_name} PUBLIC Catch2::Catch2 WCCommon::WCCommon)
    add_test("${test_name}" ${test_name})
endif()

if ("LogConfigTest" IN_LIST target_tests)
    set(test_name "LogConfigTest")
    add_executable(${test_name})
    target_sources(${test_name} PUBLIC ${test_name}.cpp)
    target_link_libraries(${test_name} PUBLIC Catch2::Catch2 LogConfig::LogConfig WCCommon::WCCommon)
    add_test("${test_name}" ${test_name})
endif()

if ("ProgressBarTest" IN_LIST target_tests)
    set(test_name "ProgressBarTest")
    add_executable(${test_name})
    target_sources(${test_name} PUBLIC ${test_name}.cpp)
    target_link_libraries(${test_name} PUBLIC Catch2::Catch2 WCCommon::WCCommon)
    add_test("${test_name}" ${test_name})
endif()

if ("WCCommonUtilsTest" IN_LIST target_tests)
    set(test_name "WCCommonUtilsTest")
    add_executable(${test_name})
    target_sources(${test_name} PUBLIC ${test_name}.cpp)
    target_link_libraries(${test_name} PUBLIC Catch2::Catch2 WCCommon::WCCommon)
    add_test("${test_name}" ${test_name})
endif()

if ("YAMLGetFieldTest" IN_LIST target_tests)
    set(test_name "YAMLGetFieldTest")
    add_executable(${test_name})
    target_sources(${test_name} PUBLIC ${test_name}.cpp)
    target_link_libraries(${test_name} PUBLIC Catch2::Catch2 WCCommon::WCCommon)
    add_test("${test_name}" ${test_name})
endif()
