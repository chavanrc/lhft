#pragma once

#include <cstdint>

namespace lhft::book {
    using Price    = std::size_t;
    using Quantity = std::size_t;
    using Cost     = std::size_t;
    using FillId   = std::size_t;
    using ChangeId = std::size_t;
    using OrderId  = std::size_t;
    using Symbol   = std::size_t;

    namespace {
        const Price   MARKET_ORDER_PRICE(0);
        const Price   PRICE_UNCHANGED(0);
        const int64_t SIZE_UNCHANGED(0);
        const Price   INVALID_LEVEL_PRICE(0);
        const Price   MARKET_ORDER_BID_SORT_PRICE(UINT32_MAX);
        const Price   MARKET_ORDER_ASK_SORT_PRICE(0);
    }    // namespace

    static const int32_t BOOK_DEPTH = 10;
}    // namespace lhft::book