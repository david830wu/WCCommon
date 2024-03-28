/* ProgressBarTest.cpp
*
* Author: Wentao Wu
*/

#include "ProgressBar.h"

#include <chrono>
#include <thread>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("ProgressBarTest", "[WCCommon]") {
    SECTION("show") {
        std::size_t data_len = 128;
        wcc::ProgressBar prog_bar(70, std::cout);
        for(std::size_t i = 0; i < data_len ; ++i) {
            // proccessing data
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            prog_bar.update( (i + 1) * 100 / data_len );
        }
        prog_bar.finish();
    }
}
