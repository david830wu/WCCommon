
#include "DefEnum.h"
#include <cstdint>

#include <catch2/catch_test_macros.hpp>

namespace test {
// A test example here
DEF_ENUM(logout_reason_t, uint8_t,
    (user_initiated_logout,   1)
    (system_initiated_logout)   // this will be 2
    (heartbeat_failure,       101)
    (heartbeat_failure_unknown,             102)
    (heartbeat_failure_invalid_out_seq_num)
    (heartbeat_response_invalid,            105)
    (socket_disconnect,                     112)
    (sequence_less_than_expected,           113)
)
}  // namespace test

TEST_CASE("enum values", "[DEF_ENUM]") {
    REQUIRE(uint8_t(test::logout_reason_t::user_initiated_logout) == 1);
    REQUIRE(uint8_t(test::logout_reason_t::system_initiated_logout) == 2);
    REQUIRE(uint8_t(test::logout_reason_t::heartbeat_failure) == 101);
    REQUIRE(uint8_t(test::logout_reason_t::heartbeat_failure_unknown) == 102);
    REQUIRE(uint8_t(test::logout_reason_t::heartbeat_failure_invalid_out_seq_num) == 103);
    REQUIRE(uint8_t(test::logout_reason_t::heartbeat_response_invalid) == 105);
    REQUIRE(uint8_t(test::logout_reason_t::socket_disconnect) == 112);
    REQUIRE(uint8_t(test::logout_reason_t::sequence_less_than_expected) == 113);
}

TEST_CASE("enum strings", "[DEF_ENUM]") {
    // actually call const char* test::stringize(test::logout_reason_t).
    // No test:: before stringize call b/c of ADL (unless there a global
    // stringize(test::logout_reason_t) already exist somehow)
    REQUIRE(stringize(test::logout_reason_t::user_initiated_logout) == "user_initiated_logout");
    REQUIRE(stringize(test::logout_reason_t::system_initiated_logout) == "system_initiated_logout");
    REQUIRE(stringize(test::logout_reason_t::heartbeat_failure) == "heartbeat_failure");
    REQUIRE(stringize(test::logout_reason_t::heartbeat_failure_unknown) == "heartbeat_failure_unknown");
    REQUIRE(stringize(test::logout_reason_t::heartbeat_failure_invalid_out_seq_num) == "heartbeat_failure_invalid_out_seq_num");
    REQUIRE(stringize(test::logout_reason_t::heartbeat_response_invalid) == "heartbeat_response_invalid");
    REQUIRE(stringize(test::logout_reason_t::socket_disconnect) == "socket_disconnect");
    REQUIRE(stringize(test::logout_reason_t::sequence_less_than_expected) == "sequence_less_than_expected");
}

