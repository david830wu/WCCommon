/*
* LogConfigTest.cpp
*
* Author: Wentao Wu
*
* This file contains tests for the LogConfig functionality.
*/

#include "LogConfig.h"

#include <catch2/catch_test_macros.hpp>

// Struct with an attached logger named "Test"
TEST_CASE("Logger", "[LogConfig]") {
   // Note no timestamp here so that we can compare log content
   YAML::Node cfg = YAML::Load(R"(
       default_format : "[%-8l] [%-12n] %v"
       default_level : "error"
       default_log_dir : "log"
       default_log_prefix: "test"
       sinks:
       - stdout
       - string
       - basic_file
       loggers:
       - main
       - Test
       set_error_loggers:
       set_debug_loggers:
   )");

   wcc::config_log(cfg);

   SECTION("spdlog-get") {
       wcc::config_log(cfg);
       auto p_logger = spdlog::get("main");
   }

   // Usual usage as old times
   SECTION("spdlog-levels") {
       auto p_logger = wcc::get_logger("main");
       p_logger->debug("This is a {} message", "debug");
       p_logger->info("pi = {:.4f}", 3.1415926);
       p_logger->warn("The answer is {:6d}", 42);
       p_logger->error("{:06d} is SZ stock", 5);
       p_logger->critical("{} is not a {}", "cat", "fruit");
   }

   SECTION("Check log content") {
       // Note debugged info is not present here
       REQUIRE(wcc::get_logger_str() ==
            "[info    ] [main        ] pi = 3.1416\n"
            "[warning ] [main        ] The answer is     42\n"
            "[error   ] [main        ] 000005 is SZ stock\n"
            "[critical] [main        ] cat is not a fruit\n"
       );
   }
}