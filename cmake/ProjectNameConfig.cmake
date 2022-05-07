# add dependencies
include(CMakeFindDependencyMacro)
# add all libs imported by find_package() in project
find_dependency(HDF5)
find_dependency(Torch)

# config {{ProjectName}}
include("${CMAKE_CURRENT_LIST_DIR}/{{ProjectName}}Targets.cmake")
