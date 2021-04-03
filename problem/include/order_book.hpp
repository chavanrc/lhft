#pragma once

#include <list>
#include <map>
#include <vector>

#include "order_tracker.hpp"
#include "types.hpp"

namespace lhft::book {
    template<typename OrderPtr>
    class OrderBook {
    public:
        using Tracker = OrderTracker<OrderPtr>;
        using TrackerMap = std::multimap<ComparablePrice, Tracker>;
        using DeferredMatches = std::list<typename TrackerMap::iterator>;
        using TrackerVec = std::vector<Tracker>;
        using Bids = TrackerMap;
        using Asks = TrackerMap;

        explicit OrderBook(Symbol symbol = 0);

        auto SetSymbol(Symbol symbol) -> void;

        [[nodiscard]] auto GetSymbol() const -> Symbol;

        [[nodiscard]] auto Add(const OrderPtr &order) -> bool;

        auto Cancel(const OrderPtr &order) -> void;

        auto Replace(const OrderPtr &order, int64_t size_delta = SIZE_UNCHANGED, Price new_price = PRICE_UNCHANGED)
        -> void;

        auto MarketPrice(Price price) -> void;

        [[nodiscard]] auto MarketPrice() const -> Price;

        auto GetBids() const -> const TrackerMap &;

        auto GetAsks() const -> const TrackerMap &;

        auto MatchOrder(Tracker &inbound, Price inbound_price, TrackerMap &current_orders) -> bool;

        auto MatchRegularOrder(Tracker &inbound, Price inbound_price, TrackerMap &current_orders) -> bool;

        auto CreateTrade(Tracker &inbound_tracker, Tracker &current_tracker, Quantity max_quantity = UINT64_MAX)
        -> Quantity;

        auto FindOnMarket(const OrderPtr &order, typename TrackerMap::iterator &result) -> bool;

        auto AllOrderCancel() -> std::vector<OrderId>;

    private:
        auto SubmitOrder(Tracker &inbound) -> bool;

        auto AddOrder(Tracker &inbound, Price order_price) -> bool;

        Symbol symbol_;
        TrackerMap bids_;
        TrackerMap asks_;
        Price market_price_{MARKET_ORDER_PRICE};
    };
}    // namespace lhft::book

#include "../src/order_book.inl"