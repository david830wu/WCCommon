#define TEST
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "AppendOnlyVec.h"
#include <thread>
#include <chrono>

TEST_CASE("AppendOnlyVec", "[AppendOnlyVec]") {

    struct Item {
        int i;
    };

    constexpr size_t CHUNK_SIZE = 4;
    constexpr size_t NUM_CHUNKS = 16;
    using Vec = wcc::AppendOnlyVec<Item, CHUNK_SIZE>; //  NUM_CHUNKS>;

    SECTION("Create Vec before Vec::config throws") {
        Vec* p_vec;
        REQUIRE_THROWS_AS((p_vec = new Vec()), std::logic_error);
    }

    SECTION("Init/Appending/Accessing/Iterating") {
        {
            Vec::config(NUM_CHUNKS);
            Vec vec;
            REQUIRE(vec.size() == 0);

            for (auto i = 0; i < 5; ++i) {
                vec.push_back(Item{i});
            }
            for (auto i = 5; i < CHUNK_SIZE * NUM_CHUNKS; ++i) {
                vec.emplace_back(Item{i});
            }

            // Outdated: now will alloc new chunks
            // Further emplace_back or push_back will cause throwing exception
            // REQUIRE_THROWS(vec.push_back(Item{888}));
            // REQUIRE_THROWS(vec.emplace_back(Item{888}));
            vec.push_back(Item{888});
            REQUIRE(vec.size() == CHUNK_SIZE * NUM_CHUNKS + 1);
            vec.emplace_back(Item{888});
            REQUIRE(vec.size() == CHUNK_SIZE * NUM_CHUNKS + 2);
        }

        // Previouse vec out of scope and chunks returned back to storage

        {
            Vec vec;
            vec.reserve(NUM_CHUNKS);
            REQUIRE_NOTHROW(vec.push_back(Item{888}));
            REQUIRE_NOTHROW(vec.emplace_back(Item{999}));
            REQUIRE(vec.size() == 2);
            REQUIRE(vec.capacity() == CHUNK_SIZE * NUM_CHUNKS);
            REQUIRE(vec[0].i == 888);
            REQUIRE(vec[1].i == 999);

            vec[0].i = 0;  // overwrite
            vec[1].i = 1;  // overwrite

            for (auto i = 2; i < 15; ++i) {
                vec.push_back(Item{i});
            }

            REQUIRE(vec.size() == 15);
            REQUIRE(vec.capacity() == CHUNK_SIZE * NUM_CHUNKS);
            for (auto i = 0; i < 15; ++i) {
                REQUIRE(i == vec[i].i);
            }

            // Accessing element at index above vec.size() is undefined, just as std::vector.

            // Iterator
            int i = 0;
            for (auto it = vec.begin(); it != vec.end(); ++it) {
                REQUIRE(it->i == i++);
            }
            REQUIRE(i == vec.size());

            // Explicitly return memory back
            vec = Vec();  // call move assignment of Vec to swap a new-borned temp vec to current vec, then the temp out of scope
            REQUIRE(vec.size() == 0);
            for (auto i = 1; i < 20; ++i) {
                vec.emplace_back(Item{i});
            }
            i = 0;
            for (auto it = vec.begin(); it != vec.end(); ++it) {
                REQUIRE(it->i == ++i);
            }
            REQUIRE(i == vec.size());
        }
    }  // SECTION

    // Already configured before
    SECTION("Call Vec::config multiple times throws") {
        REQUIRE_THROWS_AS((Vec::config(20)), std::logic_error);
        REQUIRE_THROWS_AS((Vec::config(NUM_CHUNKS)), std::logic_error);
    }

}  // TEST_CASE


// Adjust N_GB and CHUNK_SIZE. NUK_CHUNKS calc'ed based on those two.
constexpr size_t N_GB = 8;  // 1GB per vector of doubles
constexpr size_t CHUNK_SIZE = 8192;
constexpr size_t NUM_CHUNKS = N_GB * (size_t(1) << 30) / CHUNK_SIZE / sizeof(double); // 16384 for 1GB
using Vector = wcc::AppendOnlyVec<double, CHUNK_SIZE>;
using VectorTS = wcc::AppendOnlyVec<uint32_t, CHUNK_SIZE>;

TEST_CASE("TransBookDefinition", "TransBook") {
    // The sleep for us to monitor memory usage via
    //   top -p $(ps -a | grep AppendOnlyVec | awk -e'{print $1}')
    std::this_thread::sleep_for(std::chrono::seconds(5));

    struct TransactionBook {
        VectorTS   timestamp;     // 0.5GB
        Vector     trade_index;   // 1GB and thereafter
        Vector     trade_buyno;
        Vector     trade_sellno;
        Vector     trade_type;    // 1 for cancel and 0 for trade
        Vector     trade_bsflag;  // 1 for buy and 2 for sell
        Vector     trade_price;
        Vector     trade_quantity;
        Vector     trade_money;
        double     pre_close;
        double     free_float_shares;
    };

    Vector::config(NUM_CHUNKS);
    VectorTS::config(NUM_CHUNKS/8);
    TransactionBook tb;

    std::this_thread::sleep_for(std::chrono::seconds(5));

    for (auto i = 0u; i < 100000000u; ++i) {
        tb.timestamp.push_back(i);
        tb.trade_index.push_back(i*3.14);
        tb.trade_buyno.push_back(i*3.14);
        tb.trade_sellno.push_back(i*3.14);
        tb.trade_type.push_back(i*3.14);
        tb.trade_bsflag.push_back(i*3.14);
        tb.trade_price.push_back(i*3.14);
        tb.trade_quantity.push_back(i*3.14);
        tb.trade_money.push_back(i*3.14);
    }

    // This is the supposed accessing method in multithreading
    for (auto i = 0u; i < 100000000u; ++i) {
        REQUIRE(tb.timestamp[i] == i);
        REQUIRE_THAT(tb.trade_price[i], Catch::Matchers::WithinAbs(i*3.14, 1e-10));
    }
}
