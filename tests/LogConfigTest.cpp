/* LogConfigTest.cpp
*
* Author: Wentao Wu
*/

#include "LogConfig.h"
#include <catch2/catch_test_macros.hpp>

struct Logged : wcc::AttachLogger<"Test"> {
    Logged() {
        // Normal usage:
        // since we are running inside Catch2 internal functions which can not
        // be detected by out macro as HAS_ID, so id outputed as [-] event we
        // have defined the metod.
        HJ_TLOG(1, info, "init", ("Logged constructor called!"));
    }

    void run() {
        // For a variable. All these output without id field (since using HJ_LOG)
        int x = 1;
        HJ_LOG(1, info,  "event", VAR(x));           // default format: "id:{}"
        HJ_LOG(1, error, "event", ("x:{:06}", x));  // specify a format fully

        // Multiple format/value pairs allowed
        // This one also output id field as [1] since we has id() method for this class
        HJ_TLOG(1, info, "withid", VAR(x), ("msg:{}", "hello"), ("pi:{}", 3.14));
    }

    auto id() const { return 8; }
};

TEST_CASE("Logger", "[LogConfig]") {
    // Note no timestamp here so that we can compare log content
    YAML::Node cfg = YAML::Load(R"(
        default_format    : "[%-8l] [%-12n] %v"
        default_level     : "error"
        default_log_dir   : "log"
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

    // Macros needs more familarity
    SECTION("macros") {
        double x = 3.14;
        auto f = [](auto x) { return x; };
        int price = 1024;
        int vol = 200;

        // First arg to HJ_LOG and HJ_TLOG must be 0 is used outside a class/struct/union

        HJ_LOG(0, debug, "event");
        HJ_LOG(0, info,  "event", ("{:3d}", 5), (f(x)), ("hello"), ("{:.10s}","pliu"));  // ("hello") <=> ("{}","hello")
        HJ_LOG(0, debug, "event", VAR(price), VAR(vol));  // VAR(x) expand to ("x:{}", x)

        // HJ_TLOG supposed to output [id]. Since no id availale, here output as [-]
        HJ_TLOG(0, info,  "withid", ("{:3d}", 5), (f(x)), ("hello"), ("{:.10s}","pliu"));
        HJ_TLOG(0, debug, "event");
        HJ_TLOG(0, debug, "event", VAR(price), VAR(vol));  // VAR(x) expand to ("x:{}", x)
    }

    SECTION("Class with attached Logger") {
        Logged logged{};
        logged.run();
    }

    SECTION("Check log content") {
        // Note debugged info not here
        REQUIRE(wcc::get_logger_str() ==
            "[info    ] [main        ] pi = 3.1416\n"
            "[warning ] [main        ] The answer is     42\n"
            "[error   ] [main        ] 000005 is SZ stock\n"
            "[critical] [main        ] cat is not a fruit\n"
            "[info    ] [main        ] [CATCH2_INTERNAL_] [event       ] {  5,3.14,hello,pliu}\n"
            "[info    ] [main        ] [CATCH2_INTERNAL_] [withid      ] [-] {  5,3.14,hello,pliu}\n"
            "[info    ] [Test        ] [Logged          ] [init        ] [-] {Logged constructor called!}\n"
            "[info    ] [Test        ] [run             ] [event       ] {x:1}\n"
            "[error   ] [Test        ] [run             ] [event       ] {x:000001}\n"
            "[info    ] [Test        ] [run             ] [withid      ] [-] {x:1,msg:hello,pi:3.14}\n"
        );
    }
}
