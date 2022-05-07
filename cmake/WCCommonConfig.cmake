# add dependencies
include(CMakeFindDependencyMacro)
# add all libs imported by find_package() in project
find_dependency(HDF5)
find_dependency(Torch)

# config WCCommon
include("${CMAKE_CURRENT_LIST_DIR}/WCCommonTargets.cmake")
