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
// File management
//===============================================================================
class H5File {
public:
    H5File(std::string const& file_name, char mode = 'r')
        : file_name_(file_name)
    {
        switch (mode) {
        case 'r':
            file_id_ = H5Fopen(file_name_.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
            break;
        case 'w':
            file_id_ = H5Fcreate(file_name_.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
            break;
        default:
            throw std::invalid_argument("H5File,InvalidOpenMode");
            break;
        }
        if(file_id_ == H5I_INVALID_HID) {
            throw std::runtime_error("H5File,OpenFileFailed");
        }
    }
    ~H5File() noexcept {
        herr_t status = H5Fclose(file_id_);
        if(status < 0) {
            std::cerr << "H5File,ErrorCloseFile,name=" << file_name_ << std::endl; 
        }
    }

    hid_t id() const noexcept {
        return file_id_;
    }
    std::string const& name() const noexcept {
        return file_name_;
    }
protected:
    std::string file_name_;
    hid_t file_id_;
};


//===============================================================================
// Path management
//===============================================================================
inline bool h5_has_object(hid_t file_id, std::string const& group) {
    htri_t tri = H5Lexists(file_id, group.c_str(), H5P_DEFAULT);
    if (tri > 0) {
        return true;
    } else if (tri == 0) { 
        return false;
    } else {
        throw std::runtime_error("has_object_on_path,H5Lexists"); 
    }
}
inline hid_t h5_make_group_if_not_exist(hid_t file_id, std::string const& group) {
    hid_t group_id;
    if(h5_has_object(file_id, group)) {
        group_id = H5Gopen(file_id, group.c_str(), H5P_DEFAULT);
        if(group_id < 0) {
            throw std::runtime_error("make_group_if_not_exist,H5GOpen"); 
        }
        return group_id;
    } else {
        group_id = H5Gcreate(file_id, group.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if(group_id == H5I_INVALID_HID) {
            throw std::runtime_error("make_group_if_not_exist,H5Gcreate"); 
        }
        return group_id;
    }
}

//===============================================================================
// scan for objects in group
//===============================================================================
inline herr_t _h5_add_group_name_to_keys(hid_t group, const char *name, const H5L_info_t *info, void *op_data) {
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
inline std::vector<std::string> h5_scan_group_object(hid_t group_id) {
    std::vector<std::string> objs;
    hsize_t idx = 0;
    herr_t status = H5Literate(group_id, H5_INDEX_NAME, H5_ITER_NATIVE, &idx, &_h5_add_group_name_to_keys, (void*)(&objs));
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
inline std::vector<std::string> h5_scan_path_object(hid_t file_id, std::string const& path) {
    herr_t status;
    hid_t group_id = H5Gopen(file_id, path.c_str(), H5P_DEFAULT);
    if(group_id == H5I_INVALID_HID) {
        throw std::runtime_error("scan_path_object,OpenGroupFailed,group=\""+path+"\"");
    }
    return h5_scan_group_object(group_id);
}

//===============================================================================
// Basic Read Operations
//===============================================================================
inline std::vector<std::size_t> h5_query_dataset_dim(hid_t file_id, std::string const& dataset_name) {
    hid_t dataset_id = H5Dopen(file_id, dataset_name.c_str(), H5P_DEFAULT);
    if(dataset_id == H5I_INVALID_HID) { throw std::runtime_error("query_dataset_dim::H5Dopen"); }

    hid_t dataspace_id = H5Dget_space(dataset_id);
    if(dataspace_id == H5I_INVALID_HID) { throw std::runtime_error("query_dataset_dim::H5Dget_space"); }

    int n_dims = H5Sget_simple_extent_ndims(dataspace_id);
    std::vector<hsize_t> ext_dims(n_dims, 0);
    H5Sget_simple_extent_dims(dataspace_id, ext_dims.data(), nullptr);

    if( H5Dclose(dataset_id) < 0 ) { throw std::runtime_error("query_dataset_dim::H5Dclose"); }
    std::vector<std::size_t> ext_dims_casted(n_dims, 0);
    for(int i = 0; i < n_dims; ++i){
        ext_dims_casted[i] = static_cast<std::size_t>(ext_dims[i]);
    }
    return ext_dims_casted;
}

template<typename T>
inline void h5_read_array(hid_t file_id, const std::string& dataset_name, T* data) {
    hid_t dataset_id = H5Dopen(file_id, dataset_name.c_str(), H5P_DEFAULT);
    if(dataset_id == H5I_INVALID_HID) { throw std::runtime_error("read_array_from_h5::H5Dopen"); }

    hid_t data_type_id = to_h5_type_id<T>();
    herr_t status = H5Dread(dataset_id, data_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, static_cast<void*>(data));
    if(status < 0) { throw std::runtime_error("read_array_from_h5::H5Dread"); }

    if( H5Dclose(dataset_id) < 0 ) { throw std::runtime_error("read_array_from_h5::H5Dclose"); }
}

template<typename Container>
inline void h5_read_vector(hid_t file_id, const std::string& dataset_name, Container& data) {
    using value_type = typename Container::value_type;
    std::vector<value_type> buffer;

    std::vector<std::size_t> dims;
    dims = h5_query_dataset_dim(file_id, dataset_name);

    if(dims.empty()) { throw std::runtime_error("read_vector_from_h5,EmptyDataset"); }

    std::size_t total_elements = 1;
    for(int d : dims) { total_elements *= d; }

    if(total_elements == 0) { throw std::runtime_error("read_vector_from_h5,ExistZeroDim"); }

    buffer.clear();
    std::fill_n(std::back_inserter(buffer), total_elements, static_cast<value_type>(0));
    h5_read_array<value_type>(file_id, dataset_name, buffer.data());

    // copy buffer to data
    data.clear();
    for(value_type const& elem: buffer) {
        data.push_back(elem);
    }
}
template<typename T>
inline void h5_read_vector(hid_t file_id, const std::string& dataset_name, std::vector<T>& data) {
    std::vector<std::size_t> dims;
    dims = h5_query_dataset_dim(file_id, dataset_name);

    if(dims.empty()) { throw std::runtime_error("read_vector_from_h5,EmptyDataset"); }

    std::size_t total_elements = 1;
    for(int d : dims) { total_elements *= d; }

    if(total_elements == 0) { throw std::runtime_error("read_vector_from_h5,ExistZeroDim"); }

    data.clear();
    std::fill_n(std::back_inserter(data), total_elements, static_cast<T>(0));
    h5_read_array<T>(file_id, dataset_name, data.data());
}

//===============================================================================
// Basic Write Operations
//===============================================================================
template<typename T>
inline void h5_write_array(hid_t file_id, const std::string& dataset_name, const T* data, std::size_t len) {
    constexpr std::size_t k_label_rank = 1;
    hsize_t dims[k_label_rank];
    dims[0] = len;

    hid_t dataspace_id = H5Screate_simple(k_label_rank, dims, nullptr);
    if(dataspace_id == H5I_INVALID_HID) { throw std::runtime_error("WriteArray::H5Screate_simple"); }

    hid_t data_type_id = to_h5_type_id<T>();

    hid_t dataset_id = H5Dcreate(file_id, dataset_name.c_str(), data_type_id, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if(dataset_id == H5I_INVALID_HID) { throw std::runtime_error("WriteArray::H5Dcreate"); }

    herr_t status = H5Dwrite(dataset_id, data_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
    if(status < 0) { throw std::runtime_error("WriteArray::H5Dwrite"); }

    if( H5Sclose(dataspace_id) < 0 ) { throw std::runtime_error("WriteArray::H5Sclose"); }
    if( H5Dclose(dataset_id) < 0 ) { throw std::runtime_error("WriteArray::H5Dclose"); }
}
template<typename Container>
inline void h5_write_vector(hid_t file_id, const std::string& dataset_name, Container const& data, char zip_method) {
    using value_type = typename Container::value_type;
    std::vector<value_type> buffer(data.begin(), data.end());
    h5_write_array<value_type>(file_id, dataset_name, buffer.data(), buffer.size());
}
template<typename T>
inline void h5_write_vector(hid_t file_id, const std::string& dataset_name, std::vector<T> const& data, char zip_method) {
    using value_type = T;
    // avoid copy to continuous memory
    h5_write_array<value_type>(file_id, dataset_name, data.data(), data.size());
}

} // namespace wcc
