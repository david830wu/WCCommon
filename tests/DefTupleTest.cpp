#include "DefTuple.h"

#include <catch2/catch_test_macros.hpp>

using namespace wcc;

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
    (decimal_t    , Price, 6)
    (volume_t     , Volume, 6)
    (price_t      , Amount)        // SH only
    (flag_t       , BSFlag)        // SH only 'B' for active buy and 'S' for active sell, 'U' for unknown
    (flag_t       , CancelFlag)    // SZ only 'C' for cancel, 'F' for trade
    (seq_id_t     , BidOrderSeqNo)
    (seq_id_t     , AskOrderSeqNo)
    (biz_index_t  , BizIndex) // used for SH trans+order sort
    (timestamp_t  , HostTime),
    trans_field_names
)

int main() {
	wcq::Transaction t{};
	std::cout << stringize(t);
}
