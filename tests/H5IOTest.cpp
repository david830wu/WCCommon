/* H5IOTest.cpp
* 
* Author: Wentao Wu
*/

#include "H5IO.h"
#include <vector>
#include <list>
#include <fmt/format.h>
#include <unistd.h>

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

using namespace wcc;

TEST_CASE("H5IOTest", "[WCCommon]") {
    std::string filename = "H5IOTestData.h5";
    SECTION("write - array") {
        std::vector<double> darray{1.1,1.1,2.1,3.1,5.1,8.1,11.1,13.1};
        std::vector<int   > iarray{1,1,2,3,5,8,11,13};
        herr_t status;
        hid_t file_id;
        file_id = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        if(file_id < 0) throw std::runtime_error("StreamFacade,dump_h5,H5FCreate");
        hid_t group_id = H5Gcreate (file_id, "Data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        write_array_to_h5(group_id, "d_dataset", darray.data(), darray.size());
        write_array_to_h5(group_id, "i_dataset", iarray.data(), iarray.size());
        H5Gclose(group_id);
        H5Fclose(file_id);
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
        write_array_to_h5(group_id, "d_dataset", darray.data(), darray.size());
        write_array_to_h5(group_id, "i_dataset", iarray.data(), iarray.size());
        auto objs_from_group = scan_group_object(group_id);
        REQUIRE(objs_from_group.size() == 2);
        REQUIRE(objs_from_group[0] == "d_dataset");
        REQUIRE(objs_from_group[1] == "i_dataset");
        auto objs_from_root = scan_path_object(file_id, "/");
        REQUIRE(objs_from_root.size() == 1);
        REQUIRE(objs_from_root[0] == "Data");
        auto objs_from_path = scan_path_object(file_id, "/Data");
        REQUIRE(objs_from_path.size() == 2);
        REQUIRE(objs_from_path[0] == "d_dataset");
        REQUIRE(objs_from_path[1] == "i_dataset");
        H5Gclose(group_id);
        H5Fclose(file_id);
        unlink(filename.c_str());
    }
    SECTION("write - vector") {
        std::vector<double> darray{1.1,1.1,2.1,3.1,5.1,8.1,11.1,13.1};
        std::vector<int   > iarray{1,1,2,3,5,8,11,13};
        herr_t status;
        hid_t file_id;
        file_id = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        if(file_id < 0) throw std::runtime_error("StreamFacade,dump_h5,H5FCreate");
        write_vector_to_h5(file_id, "/d_dataset", darray, 'z');
        write_vector_to_h5(file_id, "/i_dataset", iarray, 'z');
        auto objs = scan_path_object(file_id, "/");
        REQUIRE(objs.size() == 2);
        REQUIRE(objs[0] == "d_dataset");
        REQUIRE(objs[1] == "i_dataset");
        H5Fclose(file_id);
        unlink(filename.c_str());
    }
    SECTION("write - list") {
        std::list<double> dlist{1.1,1.1,2.1,3.1,5.1,8.1,11.1,13.1};
        std::list<int   > ilist{1,1,2,3,5,8,11,13};
        herr_t status;
        hid_t file_id;
        file_id = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        if(file_id < 0) throw std::runtime_error("StreamFacade,dump_h5,H5FCreate");
        write_vector_to_h5(file_id, "/d_dataset", dlist, 'z');
        write_vector_to_h5(file_id, "/i_dataset", ilist, 'z');
        auto objs = scan_path_object(file_id, "/");
        REQUIRE(objs.size() == 2);
        REQUIRE(objs[0] == "d_dataset");
        REQUIRE(objs[1] == "i_dataset");
        H5Fclose(file_id);
        unlink(filename.c_str());
    }
}
