
#include "DefEnum.h"
#include <iostream>
#include <cstdint>

#include <catch2/catch_test_macros.hpp>


using namespace wcc;

// A test example here
DEF_ENUM(logout_reason_t, uint8_t,
    (user_initiated_logout,   1)
    (system_initiated_logout)   // this will be 2
    (heartbeat_failure,       101)
    (heartbeat_failure_unknown,             102)
    (heartbeat_failure_invalid_out_seq_num, 103)
    (heartbeat_response_invalid,            105)
    (socket_disconnect,                     112)
    (sequence_less_than_expected,           113)
)

int main()
{
    std::cout << stringize(logout_reason_t::heartbeat_failure) << '\n';
    std::cout << stringize(logout_reason_t::system_initiated_logout) << '\n';
    std::cout << uint16_t(logout_reason_t::system_initiated_logout) << '\n';
}
