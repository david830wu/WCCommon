# add dependencies
include(CMakeFindDependencyMacro)
# add all libs imported by find_package() in project
find_dependency(date)
find_dependency(fmt)
find_dependency(yaml-cpp)
find_dependency(spdlog)

# config WCCommon
include("${CMAKE_CURRENT_LIST_DIR}/WCCommonTargets.cmake")
