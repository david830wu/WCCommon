#pragma once

#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/seq/variadic_seq_to_seq.hpp>

#define ELEMENT_TYPE(r, data, n, elem) \
    BOOST_PP_COMMA_IF(n)               \
    BOOST_PP_TUPLE_ELEM(0, elem)
#define ELEMENT_NAME(r, data, elem) \
    BOOST_PP_TUPLE_ELEM(1, elem),
#define ELEMENT_NAME_STR(r, data, elem) \
    BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(1, elem)),

#define DEF_TUPLE(name, values, names_func)                                                        \
    using name = std::tuple<                                                                       \
        BOOST_PP_SEQ_FOR_EACH_I_R(1, ELEMENT_TYPE, , BOOST_PP_VARIADIC_SEQ_TO_SEQ(values))>;       \
                                                                                                   \
    struct name##Field                                                                             \
    {                                                                                              \
        enum                                                                                       \
        {                                                                                          \
            BOOST_PP_SEQ_FOR_EACH_R(1, ELEMENT_NAME, , BOOST_PP_VARIADIC_SEQ_TO_SEQ(values))       \
        };                                                                                         \
    };                                                                                             \
    inline const std::vector<std::string> &names_func()                                            \
    {                                                                                              \
        static std::vector<std::string> s_field_names = {                                          \
            BOOST_PP_SEQ_FOR_EACH_R(1, ELEMENT_NAME_STR, , BOOST_PP_VARIADIC_SEQ_TO_SEQ(values))}; \
        return s_field_names;                                                                      \
    }

/*
 * Usage example:

namespace wcq {

DEF_TUPLE(
    Transaction,
    (seq_id_t     , SeqNo)         // unique in one channel
    (channel_id_t , ChannelNo)
    (timestamp_t  , TransactionTime)
    (decimal_t    , Price)
    (volume_t     , Volume)
    (price_t      , Amount)        // SH only
    (flag_t       , BSFlag)        // SH only 'B' for active buy and 'S' for active sell, 'U' for unknown
    (flag_t       , CancelFlag)    // SZ only 'C' for cancel, 'F' for trade
    (seq_id_t     , BidOrderSeqNo)
    (seq_id_t     , AskOrderSeqNo)
    (biz_index_t  , BizIndex) // used for SH trans+order sort
    (timestamp_t  , HostTime),
    trans_field_names
)

}  // namespace wcq
*/