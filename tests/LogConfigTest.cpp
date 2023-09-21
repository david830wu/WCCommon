/* LogConfigTest.cpp
* 
* Author: Wentao Wu
*/

#include "LogConfig.h"
#include "WCCommonUtils.h" // for mkdir_if_not_exist
        

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

TEST_CASE("WCCommonTest", "[WCCommon]") {
    std::string log_config_file= "tests/LogConfig.yaml";
    SECTION("spdlog-get") {
        wcc::check_file_exist(log_config_file);
        wcc::mkdir_if_not_exist("log");
        wcc::config_log(log_config_file);
        auto p_logger = spdlog::get("main");
    }
    SECTION("spdlog-levels") {
        wcc::config_log(log_config_file);
        wcc::mkdir_if_not_exist("log");
        auto p_logger = wcc::get_logger("main");
        p_logger->debug("This is a {} message", "debug");
        p_logger->info("pi = {:.4f}", 3.1415926);
        p_logger->warn("The answer is {:6d}", 42);
        p_logger->error("{:06d} is SZ stock", 5);
        p_logger->critical("{} is not a {}", "cat", "fruit");
    }
}
