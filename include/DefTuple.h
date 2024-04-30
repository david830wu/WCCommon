#pragma once

#include <vector>
#include <iostream>
#include <fmt/format.h>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/variadic_seq_to_seq.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/size.hpp>
#include <boost/preprocessor/tuple/pop_back.hpp>
#include <boost/preprocessor/tuple/replace.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/comparison/not_equal.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>

#define ELEM_i(s, data, elem) BOOST_PP_TUPLE_ELEM(data, elem)

#define ELEM_1_STR(s, data, elem) BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(1, elem))

// The twin below used by stringize(o)
#define ELEM_1_FMT_STR(s, data, elem) BOOST_PP_STRINGIZE(BOOST_PP_COMMA_IF(BOOST_PP_NOT_EQUAL(s,1)) BOOST_PP_TUPLE_ELEM(1, elem))":{}"
#define ELEM_1_FMT_VAL(s, name, elem) std::get<name##Field::BOOST_PP_TUPLE_ELEM(1, elem)>(o)

#define ELEM_POP_BACK(z, n, elem) \
    BOOST_PP_TUPLE_REPLACE(BOOST_PP_TUPLE_POP_BACK(elem), 1, BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(1, elem),n))

#define ELEM_FOR(r, data, elem) \
    BOOST_PP_IIF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(elem), 2), elem, \
        BOOST_PP_REPEAT_FROM_TO(1, BOOST_PP_TUPLE_ELEM(2, elem), ELEM_POP_BACK, elem))

#define EXPAND_SEQ(seq)  BOOST_PP_SEQ_FOR_EACH_R(1, ELEM_FOR, 0, seq)

#define DEF_TUPLE(name, values, names_func)                                                  \
    using name = std::tuple<BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(ELEM_i, 0,              \
        BOOST_PP_VARIADIC_SEQ_TO_SEQ(EXPAND_SEQ(BOOST_PP_VARIADIC_SEQ_TO_SEQ(values)))       \
    ))>;                                                                                     \
                                                                                             \
    struct name##Field {                                                                     \
        enum {                                                                               \
            BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(ELEM_i, 1,                              \
              BOOST_PP_VARIADIC_SEQ_TO_SEQ(EXPAND_SEQ(BOOST_PP_VARIADIC_SEQ_TO_SEQ(values))) \
            ))                                                                               \
        };                                                                                   \
    };                                                                                       \
                                                                                             \
    inline const std::vector<std::string>& names_func() {                                    \
        static std::vector<std::string> s_field_names = {                                    \
            BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(ELEM_1_STR, 1,                          \
              BOOST_PP_VARIADIC_SEQ_TO_SEQ(EXPAND_SEQ(BOOST_PP_VARIADIC_SEQ_TO_SEQ(values))) \
            ))                                                                               \
        };                                                                                   \
        return s_field_names;                                                                \
    }                                                                                        \
                                                                                             \
    inline std::string stringize(name const& o) {                                            \
        return fmt::format(                                                                  \
            BOOST_PP_SEQ_FOR_EACH(ELEM_1_FMT_STR, 1,                                         \
              BOOST_PP_VARIADIC_SEQ_TO_SEQ(EXPAND_SEQ(BOOST_PP_VARIADIC_SEQ_TO_SEQ(values))) \
            ),                                                                               \
            BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(ELEM_1_FMT_VAL, name,                   \
              BOOST_PP_VARIADIC_SEQ_TO_SEQ(EXPAND_SEQ(BOOST_PP_VARIADIC_SEQ_TO_SEQ(values))) \
            ))                                                                               \
        );                                                                                   \
    }

