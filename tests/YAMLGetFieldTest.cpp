/* YAMLGetFieldTest.cpp
* 
* Author: Wentao Wu
*/

#include "WCCommonUtils.h"
#include "YAMLGetField.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

TEST_CASE("YAMLGetFieldTest", "[WCCommon]") {
    SECTION("YAMLGetField") {
        std::string config_file("YAMLGetFieldTest.yaml");
        wcc::check_file_exist(config_file);
        std::string today_str { wcc::get_today_str() };
        YAML::Node config = YAML::LoadFile(config_file);
        std::string data_folder; YAML_GET_FIELD(data_folder, config, data_folder);
        uint64_t    trader_id  ; YAML_GET_FIELD(trader_id  , config, trader_id  );
        std::string username   ; YAML_GET_FIELD(username   , config, username   );
        std::string server_ip  ; YAML_GET_FIELD(server_ip  , config, server_ip  );
        uint32_t    server_port; YAML_GET_FIELD(server_port, config, server_port);
        REQUIRE(data_folder == fmt::format("Data/{}", today_str));
        REQUIRE(trader_id   == 42);
        REQUIRE(username    == "username");
        REQUIRE(server_ip   == "127.0.0.1");
        REQUIRE(server_port == 8888);
    }
    SECTION("YAMLGetField-NoField") {
        std::string config_file("YAMLGetFieldTest.yaml");
        wcc::check_file_exist(config_file);
        std::string today_str { wcc::get_today_str() };
        YAML::Node config = YAML::LoadFile(config_file);
        bool is_thrown = false;
        std::string err_message;
        try {
            std::string password; YAML_GET_FIELD(password, config, password);
        } catch (std::runtime_error& e) {
            is_thrown = true;
            err_message = e.what();
        }
        REQUIRE(is_thrown == true);
        REQUIRE(err_message == "Cannot find \"password\" in Node \"config\"");
    }
    SECTION("YAMLGetField-BadConvertion") {
        std::string config_file("YAMLGetFieldTest.yaml");
        wcc::check_file_exist(config_file);
        std::string today_str { wcc::get_today_str() };
        YAML::Node config = YAML::LoadFile(config_file);
        bool is_thrown = false;
        std::string err_message;
        try {
            int username   ; YAML_GET_FIELD(username   , config, username   );
        } catch (std::runtime_error& e) {
            is_thrown = true;
            err_message = e.what();
        }
        REQUIRE(is_thrown == true);
        REQUIRE(err_message == "Bad conversion of \"username\" in Node \"config\"");
    }
    SECTION("YAMLGetField-NoRender") {
        std::string config_file("YAMLGetFieldTest.yaml");
        wcc::check_file_exist(config_file);
        // std::string today_str is not required in no render version
        YAML::Node config = YAML::LoadFile(config_file);
        std::string data_folder; YAML_GET_FIELD_NO_RENDER(data_folder, config, data_folder);
        REQUIRE(data_folder == "Data/${today}");
    }
}
