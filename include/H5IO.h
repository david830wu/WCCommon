/* H5IO.h
 * simple and user-friendly utils to read and write Hdf5 format file
 * 
 * Author: Wentao Wu
*/

#pragma once

#include "NaNDefs.h"
#include "NumericTime.h"

#include <hdf5.h>
#include <iostream>
#include <string>
#include <vector>

namespace wcc {

//===============================================================================
// Type conversions
//===============================================================================
template <typename T>
struct H5TypeMatchFalse { enum { value = false }; };
template<typename T> inline hid_t to_h5_type_id() { 
    static_assert(H5TypeMatchFalse<T>::value, "Undefined target HDF5 type"); 
    return H5T_NATIVE_CHAR; 
}
template<> inline hid_t to_h5_type_id<bool              >() { return H5T_NATIVE_HBOOL ; } 
template<> inline hid_t to_h5_type_id<char              >() { return H5T_NATIVE_CHAR  ; } 
template<> inline hid_t to_h5_type_id<short             >() { return H5T_NATIVE_SHORT ; } 
template<> inline hid_t to_h5_type_id<int               >() { return H5T_NATIVE_INT   ; } 
template<> inline hid_t to_h5_type_id<long              >() { return H5T_NATIVE_LONG  ; } 
template<> inline hid_t to_h5_type_id<long long         >() { return H5T_NATIVE_LLONG ; } 
template<> inline hid_t to_h5_type_id<unsigned short    >() { return H5T_NATIVE_USHORT; } 
template<> inline hid_t to_h5_type_id<unsigned int      >() { return H5T_NATIVE_UINT  ; } 
template<> inline hid_t to_h5_type_id<unsigned long     >() { return H5T_NATIVE_ULONG ; } 
template<> inline hid_t to_h5_type_id<unsigned long long>() { return H5T_NATIVE_ULLONG; } 
template<> inline hid_t to_h5_type_id<float             >() { return H5T_NATIVE_FLOAT ; } 
template<> inline hid_t to_h5_type_id<double            >() { return H5T_NATIVE_DOUBLE; } 
template<> inline hid_t to_h5_type_id<wcc::NumericTime  >() { return H5T_NATIVE_UINT  ; } 

//===============================================================================
// scan for objects in group
//===============================================================================
inline herr_t _add_group_name_to_keys(hid_t group, const char *name, const H5L_info_t *info, void *op_data) {
    std::vector<std::string>* p_keys = (std::vector<std::string>*)op_data;
    p_keys->emplace_back(name);
    return 0;
}

// scan for objects in given group_id without recursion
// e.g. for an hdf5 file contains
//   /Stocks/AAPL/Transactions
//   /Stocks/AAPL/Orders
//   /Stocks/GOOGL/Transactions
//   /Stocks/GOOGL/Orders
// given stock_grp as group id at /Stock, `scan_group_objects(stock_grp)` returns 
//   std::vector<std::string>{"AAPL", "GOOGL"}
inline std::vector<std::string> scan_group_object(hid_t group_id) {
    std::vector<std::string> objs;
    hsize_t idx = 0;
    herr_t status = H5Literate(group_id, H5_INDEX_NAME, H5_ITER_NATIVE, &idx, &_add_group_name_to_keys, (void*)(&objs));
    if(status < 0) {
        throw std::runtime_error("scan_group_objects,H5Literate");
    }
    return objs;
}

// scan for objects in given path group without recursion
// same as scan_group_object, but accept path string as an argument
// e.g. for an hdf5 file contains
//   /Stocks/AAPL/Transactions
//   /Stocks/AAPL/Orders
//   /Stocks/GOOGL/Transactions
//   /Stocks/GOOGL/Orders
// given opened file id as fid, `scan_path_objects(fid, "/Stocks")` returns 
//   std::vector<std::string>{"AAPL", "GOOGL"}
// given /Stock group id as gid, `scan_path_objects(gid, "AAPL")` returns 
//   std::vector<std::string>{"Transaction", "Orders"}
inline std::vector<std::string> scan_path_object(hid_t file_id, std::string const& path) {
    herr_t status;
    hid_t group_id = H5Gopen(file_id, path.c_str(), H5P_DEFAULT);
    if(group_id < 0) {
        throw std::runtime_error("scan_path_object,OpenGroupFailed,group=\""+path+"\"");
    }
    return scan_group_object(group_id);
}

//===============================================================================
// Basic Write Operations
//===============================================================================
template<typename T>
void write_array_to_h5(hid_t file_id, const std::string& dataset_name, const T* data, std::size_t len) {
    constexpr std::size_t k_label_rank = 1;
    hsize_t dims[k_label_rank];
    dims[0] = len;

    hid_t dataspace_id = H5Screate_simple(k_label_rank, dims, nullptr);
    if(dataspace_id < 0) { throw std::runtime_error("WriteArray::H5Screate_simple"); }

    hid_t data_type_id = to_h5_type_id<T>();

    hid_t dataset_id = H5Dcreate(file_id, dataset_name.c_str(), data_type_id, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if(dataset_id < 0) { throw std::runtime_error("WriteArray::H5Dcreate"); }

    herr_t status = H5Dwrite(dataset_id, data_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
    if(status < 0) { throw std::runtime_error("WriteArray::H5Dwrite"); }

    if( H5Sclose(dataspace_id) < 0 ) { throw std::runtime_error("WriteArray::H5Sclose"); }
    if( H5Dclose(dataset_id) < 0 ) { throw std::runtime_error("WriteArray::H5Dclose"); }
}
template<typename Container>
void write_vector_to_h5(hid_t file_id, const std::string& dataset_name, Container const& data, char zip_method) {
    using value_type = typename Container::value_type;
    std::cout << "call write_vector_to_h5(general)" << std::endl;
    write_array_to_h5<value_type>(file_id, dataset_name, buffer.data(), buffer.size());
}
template<typename T>
void write_vector_to_h5(hid_t file_id, const std::string& dataset_name, std::vector<T> const& data, char zip_method) {
    using value_type = T;
    // avoid copy to continuous memory
    write_array_to_h5<value_type>(file_id, dataset_name, data.data(), data.size());
}

template<typename T>
void read_vector_from_h5(hid_t file_id, const std::string& dataset_name,
                     std::vector<T>& data) {
    constexpr std::size_t k_label_rank = 1;
    hsize_t dims[k_label_rank];
    hid_t data_type_id = to_h5_type_id<T>();

    hid_t dataset_id = H5Dopen(file_id, dataset_name.c_str(), H5P_DEFAULT);
    if(dataset_id < 0) { throw std::runtime_error("ReadArray::H5Dopen"); }
    hid_t dataspace_id = H5Dget_space(dataset_id);
    if(dataspace_id < 0) { throw std::runtime_error("ReadArray::H5Dget_space"); }
    H5Sget_simple_extent_dims(dataspace_id, dims, nullptr);

    data.clear();
    std::fill_n(std::back_inserter(data), dims[0], static_cast<T>(0));
    herr_t status = H5Dread(dataset_id, data_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, data.data());
    if(status < 0) { throw std::runtime_error("ReadArray::H5Dread"); }

    if( H5Dclose(dataset_id) < 0 ) { throw std::runtime_error("ReadArray::H5Dclose"); }
}

} // namespace wcc
