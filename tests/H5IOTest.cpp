/* H5IOTest.cpp
* 
* Author: Wentao Wu
*/

#include "H5IO.h"
#include <vector>
#include <list>
#include <fmt/format.h>
#include <unistd.h>

//#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

using namespace wcc;

TEST_CASE("H5IOTest", "[WCCommon]") {
    std::string filename = "H5IOTestData.h5";
    SECTION("file create") {
        herr_t status;
        H5File h5_file(filename, 'w');
        REQUIRE(access(filename.c_str(), F_OK) == 0);
        // h5_file auto calls H5close(file_id_) in destruction
        unlink(filename.c_str());
    }
    SECTION("group create") {
        herr_t status;
        H5File h5_file(filename, 'w');
        hid_t group_id = h5_make_group_if_not_exist(h5_file.id(), "Data");
        REQUIRE(h5_has_object(h5_file.id(), "Data") == true);
        H5Gclose(group_id);
        unlink(filename.c_str());
    }
    SECTION("query objects - relative path") {
        herr_t status;
        H5File h5_file(filename, 'w');
        hid_t stock_group_id = h5_make_group_if_not_exist(h5_file.id(), "Stock");
        hid_t aapl_group_id = h5_make_group_if_not_exist(stock_group_id, "AAPL");
        hid_t goog_group_id = h5_make_group_if_not_exist(stock_group_id, "GOOGL");
        auto objs_from_root = h5_scan_path_object(h5_file.id(), ".");
        REQUIRE(objs_from_root.size() == 1);
        REQUIRE(objs_from_root[0] == "Stock");
        auto objs_from_stock_group = h5_scan_group_object(stock_group_id);
        REQUIRE(objs_from_stock_group.size() == 2);
        REQUIRE(objs_from_stock_group[0] == "AAPL");
        REQUIRE(objs_from_stock_group[1] == "GOOGL");
        auto objs_from_stock_path = h5_scan_path_object(h5_file.id(), "Stock");
        REQUIRE(objs_from_stock_path.size() == 2);
        REQUIRE(objs_from_stock_path[0] == "AAPL");
        REQUIRE(objs_from_stock_path[1] == "GOOGL");
        unlink(filename.c_str());
    }
    SECTION("query objects - absolute path") {
        herr_t status;
        H5File h5_file(filename, 'w');
        hid_t stock_group_id = h5_make_group_if_not_exist(h5_file.id(), "/Stock");
        hid_t aapl_group_id = h5_make_group_if_not_exist(h5_file.id(), "/Stock/AAPL");
        hid_t goog_group_id = h5_make_group_if_not_exist(h5_file.id(), "/Stock/GOOGL");
        auto objs_from_root = h5_scan_path_object(h5_file.id(), "/");
        REQUIRE(objs_from_root.size() == 1);
        REQUIRE(objs_from_root[0] == "Stock");
        auto objs_from_stock_group = h5_scan_group_object(stock_group_id);
        REQUIRE(objs_from_stock_group.size() == 2);
        REQUIRE(objs_from_stock_group[0] == "AAPL");
        REQUIRE(objs_from_stock_group[1] == "GOOGL");
        auto objs_from_stock_path = h5_scan_path_object(h5_file.id(), "/Stock");
        REQUIRE(objs_from_stock_path.size() == 2);
        REQUIRE(objs_from_stock_path[0] == "AAPL");
        REQUIRE(objs_from_stock_path[1] == "GOOGL");
        unlink(filename.c_str());
    }
    SECTION("scan objects") {
        std::vector<double> darray{1.1,1.1,2.1,3.1,5.1,8.1,11.1,13.1};
        std::vector<int   > iarray{1,1,2,3,5,8,11,13};
        herr_t status;
        hid_t file_id;
        file_id = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        if(file_id < 0) throw std::runtime_error("StreamFacade,dump_h5,H5FCreate");
        hid_t group_id = H5Gcreate (file_id, "Data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        h5_write_array(group_id, "d_dataset", darray.data(), darray.size());
        h5_write_array(group_id, "i_dataset", iarray.data(), iarray.size());
        auto objs_from_group = h5_scan_group_object(group_id);
        REQUIRE(objs_from_group.size() == 2);
        REQUIRE(objs_from_group[0] == "d_dataset");
        REQUIRE(objs_from_group[1] == "i_dataset");
        auto objs_from_root = h5_scan_path_object(file_id, "/");
        REQUIRE(objs_from_root.size() == 1);
        REQUIRE(objs_from_root[0] == "Data");
        auto objs_from_path = h5_scan_path_object(file_id, "/Data");
        REQUIRE(objs_from_path.size() == 2);
        REQUIRE(objs_from_path[0] == "d_dataset");
        REQUIRE(objs_from_path[1] == "i_dataset");
        unlink(filename.c_str());
    }
    SECTION("write - vector") {
        std::vector<double> darray{1.1,1.1,2.1,3.1,5.1,8.1,11.1,13.1};
        std::vector<int   > iarray{1,1,2,3,5,8,11,13};
        H5File h5_file(filename, 'w');
        h5_write_vector(h5_file.id(), "/d_dataset", darray);
        h5_write_vector(h5_file.id(), "/i_dataset", iarray);
        REQUIRE(h5_has_object(h5_file.id(), "/d_dataset") == true);
        REQUIRE(h5_has_object(h5_file.id(), "/i_dataset") == true);
        auto dims = h5_query_dataset_dim(h5_file.id(), "/d_dataset");
        REQUIRE(dims.size() == 1);
        REQUIRE(dims[0] == darray.size());
        dims = h5_query_dataset_dim(h5_file.id(), "/i_dataset");
        REQUIRE(dims.size() == 1);
        REQUIRE(dims[0] == iarray.size());
        unlink(filename.c_str());
    }
    SECTION("write - vector compressed") {
        constexpr std::size_t k_len = 16UL<<10; // 16k
        std::vector<double> darray(k_len, 0), darray_load;
        for(std::size_t i = 0; i < k_len; ++i) {
            darray[i] = i % 1024;
        }
        H5File h5_file(filename, 'w');
        h5_write_vector(h5_file.id(), "/compressed", darray, true);
        h5_read_vector(h5_file.id(), "/compressed", darray_load);
        REQUIRE(darray_load == darray);
        unlink(filename.c_str());
    }
    SECTION("write - list") {
        std::list<double> dlist{1.1,1.1,2.1,3.1,5.1,8.1,11.1,13.1};
        std::list<int   > ilist{1,1,2,3,5,8,11,13};
        H5File h5_file(filename, 'w');
        h5_write_vector(h5_file.id(), "/d_dataset", dlist);
        h5_write_vector(h5_file.id(), "/i_dataset", ilist);
        REQUIRE(h5_has_object(h5_file.id(), "/d_dataset") == true);
        REQUIRE(h5_has_object(h5_file.id(), "/i_dataset") == true);
        auto dims = h5_query_dataset_dim(h5_file.id(), "/d_dataset");
        REQUIRE(dims.size() == 1);
        REQUIRE(dims[0] == dlist.size());
        dims = h5_query_dataset_dim(h5_file.id(), "/i_dataset");
        REQUIRE(dims.size() == 1);
        REQUIRE(dims[0] == ilist.size());
        unlink(filename.c_str());
    }
    SECTION("read - vector") {
        std::vector<double> darray{1.1,1.1,2.1,3.1,5.1,8.1,11.1,13.1};
        std::vector<int   > iarray{1,1,2,3,5,8,11,13};
        {
            H5File h5_file(filename, 'w');
            hid_t group_id = h5_make_group_if_not_exist(h5_file.id(), "Data");
            h5_write_vector(group_id, "d_dataset", darray);
            h5_write_vector(group_id, "i_dataset", iarray);
        } {
            H5File h5_file(filename, 'r');
            std::vector<double> darray_load;
            std::vector<int   > iarray_load;
            h5_read_vector(h5_file.id(), "/Data/d_dataset", darray_load);
            h5_read_vector(h5_file.id(), "/Data/i_dataset", iarray_load);
            REQUIRE(darray_load == darray);
            REQUIRE(iarray_load == iarray);
        }
        unlink(filename.c_str());
    }
    SECTION("read - list") {
        std::list<double> dlist{1.1,1.1,2.1,3.1,5.1,8.1,11.1,13.1};
        std::list<int   > ilist{1,1,2,3,5,8,11,13};
        {
            H5File h5_file(filename, 'w');
            hid_t group_id = h5_make_group_if_not_exist(h5_file.id(), "Data");
            h5_write_vector(group_id, "d_dataset", dlist);
            h5_write_vector(group_id, "i_dataset", ilist);
        } {
            H5File h5_file(filename, 'r');
            std::list<double> dlist_load;
            std::list<int   > ilist_load;
            h5_read_vector(h5_file.id(), "/Data/d_dataset", dlist_load);
            h5_read_vector(h5_file.id(), "/Data/i_dataset", ilist_load);
            REQUIRE(dlist_load == dlist);
            REQUIRE(ilist_load == ilist);
        }
        unlink(filename.c_str());
    }
}
