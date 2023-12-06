/* NumericTimeTest.cpp
* 
* Author: Wentao Wu
* Created: 20200401
* Version: 0.1
*/

#include "NumericTime.h"

//#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch_all.hpp>

#include <iostream>
#include <sstream>

using wcc::NumericTime;

TEST_CASE("NumericTimeTest", "[NumericTime]") {

    SECTION("InitInt") {
        REQUIRE(NumericTime(103159010) == NumericTime(10, 31, 59, 10));
    }
    SECTION("InitStr") {
        REQUIRE(NumericTime("10:31:59.010") == NumericTime(10, 31, 59, 10));
    }
    SECTION("Invalid convert") {
        REQUIRE(NumericTime(23, 59, 59, 1000)== NumericTime(0, 0, 0, 0));
    }
    SECTION("GetField") {
        auto ntime = NumericTime(10, 31, 59, 10);
        REQUIRE(ntime.hour() == 10);
        REQUIRE(ntime.min()  == 31);
        REQUIRE(ntime.sec()  == 59);
        REQUIRE(ntime.ms()   == 10);
    }
    SECTION("ToStr") {
        REQUIRE(NumericTime(10, 31, 59, 10).str() == "10:31:59.010");
    }
    SECTION("std ToStr") {
        REQUIRE(std::to_string(NumericTime(9, 31, 59, 10)) == "093159010"); // padding zero
    }
    SECTION("Negative time") {
        REQUIRE(NumericTime(10, 31, 59, -1) == NumericTime(10, 31, 58, 999));
        REQUIRE(NumericTime(10, 31, -1, 10) == NumericTime(10, 30, 59,  10));
        REQUIRE(NumericTime(10, -1, 59, 10) == NumericTime( 9, 59, 59,  10));
        REQUIRE(NumericTime(-1, 31, 59, 10) == NumericTime(23, 31, 59,  10));
    }

    SECTION("Now") {
        NumericTime now_time = NumericTime::now();
        std::string now_str = now_time.str();
        std::cout << "Now = " << now_str << std::endl;
    }
    SECTION("Benchmark") {
        BENCHMARK("From int") {
            auto ntime = NumericTime(103159010);
        };
        BENCHMARK("From fields") {
            auto ntime = NumericTime(10, 31, 59, 10);
        };
        BENCHMARK("From string") {
            auto ntime = NumericTime("10:31:59.010");
        };
    }
}