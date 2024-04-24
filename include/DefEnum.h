#pragma once

#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/tuple/size.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/seq/variadic_seq_to_seq.hpp>
#include <ostream>

#define DEFINE_ENUM_VALUE(r, data, elem)                       \
    BOOST_PP_TUPLE_ELEM(0, elem)                               \
    BOOST_PP_IIF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(elem), 2), \
                 = BOOST_PP_TUPLE_ELEM(1, elem), )             \
    BOOST_PP_COMMA()

#define DEFINE_ENUM_FORMAT(r, name, elem)    \
    case name::BOOST_PP_TUPLE_ELEM(0, elem): \
        return BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, elem));

#define DEF_ENUM(name, base_type, values)                                                              \
    enum class name : base_type                                                                        \
    {                                                                                                  \
        BOOST_PP_SEQ_FOR_EACH_R(1, DEFINE_ENUM_VALUE, , BOOST_PP_VARIADIC_SEQ_TO_SEQ(values))          \
    };                                                                                                 \
    inline const char *stringize(name val)                                                             \
    {                                                                                                  \
        switch (val)                                                                                   \
        {                                                                                              \
            BOOST_PP_SEQ_FOR_EACH_R(1, DEFINE_ENUM_FORMAT, name, BOOST_PP_VARIADIC_SEQ_TO_SEQ(values)) \
        default:                                                                                       \
            return "unknown";                                                                          \
        }                                                                                              \
    }                                                                                                  \
    inline std::ostream &operator<<(std::ostream &os, name val)                                        \
    {                                                                                                  \
        return os << stringize(val);                                                                   \
    }

/* Usage example: 
#include <iostream>
#include <cstdint>

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
*/
