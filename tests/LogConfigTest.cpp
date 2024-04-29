/*
* LogConfigTest.cpp
*
* Author: Wentao Wu
*
* This file contains tests for the LogConfig functionality.
* It demonstrates various logging scenarios using the wcc::AttachLogger utility,
* spdlog logging library, and custom log macros (HJ_LOG and HJ_TLOG).
*/

#include "LogConfig.h"

#include <catch2/catch_test_macros.hpp>

#define MLOG(...)   HJ_LOG(1, __VA_ARGS__)
#define LOG(...)    HJ_LOG(0, __VA_ARGS__)
#define ID_MLOG(...)  HJ_TLOG(1, __VA_ARGS__)
#define ID_LOG(...)   HJ_TLOG(0, __VA_ARGS__)

// Struct with an attached logger named "Test"
struct Logged : wcc::AttachLogger<"Test"> {
   Logged() {
       // Normal usage:
       // Since we are running inside Catch2 internal functions, which cannot
       // be detected by our macro as HAS_ID, the id is output as [-].
       // However, we have defined the id() method for this class.
       ID_MLOG(info, "init", ("Logged constructor called!"));
   }

   void run() {
       // For a variable. All these output without an id field (since using HJ_LOG)
       int x = 1;
       MLOG(info,  "event", VAR(x));           // default format: "x:{}"
       MLOG(error, "event", ("x:{:06}", x));  // specify a format fully

       // Multiple format/value pairs allowed
       // This one also outputs an id field as [1] since we have the id() method for this class
       ID_MLOG(info, "with_id", VAR(x), ("msg:{}", "hello"), ("pi:{}", 3.14));
   }

   auto id() const { return 8; }
};

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

   // Macros need more familiarity
   SECTION("macros") {
       double x = 3.14;
       auto f = [](auto x) { return x; };
       int price = 1024;
       int vol = 200;

       // First arg to HJ_LOG and HJ_TLOG must be 0 when used outside a class/struct/union
       LOG(debug, "event");
       LOG(info,  "event", ("{:3d}", 5), (f(x)), ("hello"), ("{:.10s}","pliu")); // ("hello") <=> ("{}","hello")
       LOG(debug, "event", VAR(price), VAR(vol)); // VAR(x) expands to ("x:{}", x)

       // HJ_TLOG is supposed to output [id]. Since no id is available, it outputs as [-]
       ID_LOG(info,  "with_id", ("{:3d}", 5), (f(x)), ("hello"), ("{:.10s}","pliu"));
       ID_LOG(debug, "event");
       ID_LOG(debug, "event", VAR(price), VAR(vol)); // VAR(x) expands to ("x:{}", x)
   }

   SECTION("Class with attached Logger") {
       Logged logged{};
       logged.run();
   }

   SECTION("Check log content") {
       // Note debugged info is not present here
       REQUIRE(wcc::get_logger_str() ==
            "[info    ] [main        ] pi = 3.1416\n"
            "[warning ] [main        ] The answer is     42\n"
            "[error   ] [main        ] 000005 is SZ stock\n"
            "[critical] [main        ] cat is not a fruit\n"
            "[info    ] [main        ] [CATCH2_INTERNAL_] [event       ] {  5,3.14,hello,pliu}\n"
            "[info    ] [main        ] [CATCH2_INTERNAL_] [with_id     ] [-] {  5,3.14,hello,pliu}\n"
            "[info    ] [Test        ] [Logged          ] [init        ] [-] {Logged constructor called!}\n"
            "[info    ] [Test        ] [run             ] [event       ] {x:1}\n"
            "[error   ] [Test        ] [run             ] [event       ] {x:000001}\n"
            "[info    ] [Test        ] [run             ] [with_id     ] [-] {x:1,msg:hello,pi:3.14}\n"
       );
   }
}