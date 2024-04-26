#include "FifoFile.h"
#include <filesystem>
#include <fstream>
#include <catch2/catch_test_macros.hpp>

using FifoFile = wcc::FifoFile;

TEST_CASE("Logger", "[LogConfig]") {
    const char* fn = "fifo_test_file";

    SECTION("Fifo Ctor and Dtor") {
        REQUIRE_NOTHROW(FifoFile(fn));  // ctor then dtor
        REQUIRE(!std::filesystem::exists(fn));
    }

    SECTION("Check if fifo generated") {
        FifoFile fifo(fn);
        REQUIRE(std::filesystem::is_fifo(fn));
    }

    SECTION("Open an existing fifo") {
        FifoFile fifo(fn);
        close(int(fifo));  // close but not remove
        REQUIRE_NOTHROW(FifoFile(fn));  // ctor then dtor
        REQUIRE(!std::filesystem::exists(fn));  // removed!
    }

    SECTION("Open an non-fifo file") {
        struct create_file {
            const char* fn;
            create_file(const char* fn) : fn(fn) { std::ofstream(this->fn); }
            ~create_file() { std::filesystem::remove(fn); }
        } dummy(fn);
        REQUIRE_THROWS(FifoFile(fn));  // open non-fifo file
    }
}