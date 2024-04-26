#include "DefTuple.h"
#include <algorithm>
#include <sstream>
#include <iterator>

#include <catch2/catch_test_macros.hpp>

namespace test {

using seq_id_t = int;
using channel_id_t = int;
using timestamp_t = int;
using decimal_t = int;
using volume_t = int;
using flag_t = int;
using biz_index_t = int;
using price_t = double;

DEF_TUPLE(
    Transaction,
    (seq_id_t     , SeqNo)         // unique in one channel
    (channel_id_t , ChannelNo)
    (timestamp_t  , TransactionTime)
    (decimal_t    , Price, 6)      // Price1, Price2, ..., Price5
    (volume_t     , Volume, 6)     // Volume1, Volume2, ..., Volume5
    (price_t      , Amount)        // SH only
    (flag_t       , BSFlag)        // SH only 'B' for active buy and 'S' for active sell, 'U' for unknown
    (flag_t       , CancelFlag)    // SZ only 'C' for cancel, 'F' for trade
    (seq_id_t     , BidOrderSeqNo)
    (seq_id_t     , AskOrderSeqNo)
    (biz_index_t  , BizIndex) // used for SH trans+order sort
    (timestamp_t  , HostTime),
    trans_field_names
)

}  // namespace test

TEST_CASE("Usages", "[DEF_TUPLE]") {
	test::Transaction t{};  // all zeros

    // Since test::Transaction is actually a std::tuple (aliased by using), stringize(test::Transaction const&)
    // must be called with namespace prefixed (i.e., call as test::stringize(test::Transaction)).
    // ADL not applied here! (compiler will search stringize() and std::stringize() instead of test::stringize
    // if you don't provide namespace for it.)
	REQUIRE(test::stringize(t) ==  // everything's zero
        "SeqNo:0, ChannelNo:0, TransactionTime:0, Price1:0, Price2:0, Price3:0, "
        "Price4:0, Price5:0, Volume1:0, Volume2:0, Volume3:0, Volume4:0, Volume5:0, "
        "Amount:0, BSFlag:0, CancelFlag:0, BidOrderSeqNo:0, AskOrderSeqNo:0, BizIndex:0, HostTime:0");

    using TF = test::TransactionField;

    std::get<TF::SeqNo>(t) = 1;
    std::get<TF::Price1>(t) = 10000;
    std::get<TF::Price2>(t) = 20000;
    std::get<TF::Price3>(t) = 30000;
    std::get<TF::Price4>(t) = 40000;
    std::get<TF::Price5>(t) = 50000;
    std::get<TF::Volume1>(t) = 101;
    std::get<TF::Volume2>(t) = 201;
    std::get<TF::Volume3>(t) = 301;
    std::get<TF::Volume4>(t) = 401;
    std::get<TF::Volume5>(t) = 501;

    REQUIRE(std::get<TF::SeqNo>(t) == 1);
    REQUIRE(std::get<TF::Price1>(t) == 10000);
    REQUIRE(std::get<TF::Price2>(t) == 20000);
    REQUIRE(std::get<TF::Price3>(t) == 30000);
    REQUIRE(std::get<TF::Price4>(t) == 40000);
    REQUIRE(std::get<TF::Price5>(t) == 50000);
    REQUIRE(std::get<TF::Volume1>(t) == 101);
    REQUIRE(std::get<TF::Volume2>(t) == 201);
    REQUIRE(std::get<TF::Volume3>(t) == 301);
    REQUIRE(std::get<TF::Volume4>(t) == 401);
    REQUIRE(std::get<TF::Volume5>(t) == 501);

    REQUIRE(test::stringize(t) ==
        "SeqNo:1, ChannelNo:0, TransactionTime:0, "
        "Price1:10000, Price2:20000, Price3:30000, Price4:40000, Price5:50000, "
        "Volume1:101, Volume2:201, Volume3:301, Volume4:401, Volume5:501, "
        "Amount:0, BSFlag:0, CancelFlag:0, BidOrderSeqNo:0, AskOrderSeqNo:0, BizIndex:0, HostTime:0");

    // trans_field_names() is a func returns vector of field names.
    // The func name is specified as last parameter to DEF_TUPLE!
    auto const& names = test::trans_field_names(); // names is a ref to a invisible static global vector
    std::ostringstream ostr;
    std::copy(names.begin(), names.end(), std::ostream_iterator<std::string>{ostr, ","});
    REQUIRE(ostr.str() ==
        "SeqNo,ChannelNo,TransactionTime,Price1,Price2,Price3,Price4,Price5,Volume1,"
        "Volume2,Volume3,Volume4,Volume5,Amount,BSFlag,CancelFlag,BidOrderSeqNo,"
        "AskOrderSeqNo,BizIndex,HostTime,");
}
