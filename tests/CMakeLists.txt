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
list(APPEND target_tests "CsvIOTest")
list(APPEND target_tests "H5IOTest")
list(APPEND target_tests "YAMLGetFieldTest")

if ("CsvIOTest" IN_LIST target_tests)
    set(test_name "CsvIOTest.generic")
    add_executable(${test_name})
    target_sources(${test_name} PUBLIC CsvIOTest.cpp)
    target_link_libraries(${test_name} PUBLIC Catch2::Catch2 WCCommon::WCCommon)
    add_test("${test_name}" ${test_name})

    set(test_name "CsvIOTest.specialized")
    add_executable(${test_name})
    target_sources(${test_name} PUBLIC CsvIOTest.cpp)
    target_compile_definitions(${test_name} PUBLIC WCC_USE_CUSTOM_CONVERTER)
    target_link_libraries(${test_name} PUBLIC Catch2::Catch2 WCCommon::WCCommon)
    add_test("${test_name}" ${test_name})
endif()
if ("H5IOTest" IN_LIST target_tests)
    set(test_name "H5IOTest")
    add_executable(${test_name})
    target_sources(${test_name} PUBLIC H5IOTest.cpp)
    target_link_libraries(${test_name} PUBLIC Catch2::Catch2 WCCommon::WCCommon Hdf5::Hdf5)
    add_test("${test_name}" ${test_name})
endif()
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
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/LogConfig.yaml DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
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
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/YAMLGetFieldTest.yaml DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()
