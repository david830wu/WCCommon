find_package(HDF5)

if(HDF5_FOUND)
    message(STATUS "Found Hdf5: create Hdf5::Hdf5 interface target")
    message(STATUS "Hdf5 include: ${HDF5_INCLUDE_DIRS}")
    message(STATUS "Hdf5 libs   : ${HDF5_LIBRARIES}"   )
    add_library(ihdf5 INTERFACE)
    target_include_directories(
        ihdf5
    INTERFACE
        ${HDF5_INCLUDE_DIRS}
    )
    target_link_libraries(
        ihdf5
    INTERFACE
        ${HDF5_LIBRARIES}
    )
    add_library(Hdf5::Hdf5 ALIAS ihdf5)
endif()
